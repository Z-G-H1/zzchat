#include "LogicSystem.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"
#include "message.grpc.pb.h"
#include "StatusGrpcClient.h"
#include "ConfigMgr.h"
#include "UserMgr.h"
#include "ChatGrpcClient.h"

LogicSystem::LogicSystem(): _b_stop(false){
    RegisterCallBackS();
    _worker_thread = std::thread(&LogicSystem::DealMsg, this);
}

void LogicSystem::RegisterCallBackS(){
    _fun_callbacks[MSG_CHAT_LOGIN] = [this](std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data){
        LoginHandler(session, msg_id, msg_data);
    };

    _fun_callbacks[ID_SEARCH_USER_REQ] = [this](std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data){
        SearchInfo(session, msg_id, msg_data);
    };

    _fun_callbacks[ID_ADD_FRIEND_REQ] = [this](std::shared_ptr<CSession>session, const short& msg_id, const std::string& msg_data){
        AddFriendApply(session, msg_id, msg_data);
    };

    _fun_callbacks[ID_AUTH_FRIEND_REQ] = [this](std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data){
        AuthFriendApply(session, msg_id, msg_data);
    };

    _fun_callbacks[ID_TEXT_CHAT_MSG_REQ] = [this](std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data){
        DealChatMsg(session, msg_id, msg_data);
    };
}

LogicSystem::~LogicSystem(){
    _b_stop = true;
    _consume.notify_one();
    _worker_thread.join();
	std::cout << "Destruct Logicsystem" << std::endl;

}

// 添加待处理的内容 --生产者
void LogicSystem::PostMsgToQue(std::shared_ptr<LogicNode> msg){
    std::lock_guard<std::mutex> lock(_mutex);
    if(_b_stop){
        return ;
    }
    _msg_que.push(msg);
    // 由0变为1时 notify， 因为队列非空会自动处理
    if(_msg_que.size() == 1)
        _consume.notify_one();
}

// 处理逻辑，调用请求对应的回调函数--消费者
void LogicSystem::DealMsg(){
    while(1){
        std::unique_lock<std::mutex> lock(_mutex);
        while(_msg_que.empty() && !_b_stop){
            _consume.wait(lock);
        }
        // 停服， 将消息队列中数据处理完，退出
        if(_b_stop){
            while(!_msg_que.empty()){
                auto msg_node = _msg_que.front();
                _msg_que.pop();
		        std::cout << "msg id is " << msg_node->_recv_node->msg_id << std::endl;
                auto iter = _fun_callbacks.find(msg_node->_recv_node->msg_id);
                if(iter == _fun_callbacks.end()){
                    // 没有对应的回调函数
                    std::cout << "msg id [" << msg_node->_recv_node->msg_id << "] handler not found" << std::endl;
                    continue;
                }
                // 调用回调函数
                iter->second(msg_node->_session, msg_node->_recv_node->msg_id, 
                    std::string(msg_node->_recv_node->_data, msg_node->_recv_node->_total_len));
            }
            break; 
        }
        // 没有停服
        auto msg_node = _msg_que.front();
        _msg_que.pop();
		std::cout << "msg is is " << msg_node->_recv_node->msg_id << std::endl;
        auto iter = _fun_callbacks.find(msg_node->_recv_node->msg_id);
        if(iter == _fun_callbacks.end()){
            // 没有对应的回调函数
            std::cout << "msg id [" << msg_node->_recv_node->msg_id << "] handler not found" << std::endl;
            continue;
        }
        iter->second(msg_node->_session, msg_node->_recv_node->msg_id, 
            std::string(msg_node->_recv_node->_data, msg_node->_recv_node->_total_len));
    }
}

// 用户登录该服务器处理函数
void LogicSystem::LoginHandler(std::shared_ptr<CSession> session, const short &msg_id, const std::string &msg_data){
    Json::Reader reader;
    Json::Value root;
    reader.parse(msg_data, root);
    int uid = root["uid"].asInt();
    auto token = root["token"].asString();
    std::cout << "user login uid is  " << uid << " user token  is " << token << std::endl;

    // 从状态服务器获取token 看是否匹配  在statusServer中进行比对
    auto resp = StatusGrpcClient::GetInstance()->Login(uid, root["token"].asString());
    Json::Value rtvalue;
    rtvalue["error"] = resp.error();
    if (resp.error() != ErrorCodes::Success) {
        std::string jsonstr = rtvalue.toStyledString();
        std::cout << "Login Error " << jsonstr << std::endl;
        session->Send(jsonstr, MSG_CHAT_LOGIN_RSP);
        return;
    }

    // 获取用户信息
    std::string uid_str = std::to_string(uid);
    std::string base_key = USER_BASE_INFO + uid_str;
    auto user_info = std::make_shared<UserInfo>();
    // 获取用户信息
    bool b_base = GetBaseInfo(base_key, uid, user_info);
    if (!b_base) {
        rtvalue["error"] = ErrorCodes::UidInvalid;
        std::string jsonstr = rtvalue.toStyledString();
        session->Send(jsonstr, MSG_CHAT_LOGIN_RSP);
        return;
    }

    rtvalue["uid"] = uid;
    rtvalue["pwd"] = user_info->pwd;
    rtvalue["name"] = user_info->name;
    rtvalue["email"] = user_info->email;
    rtvalue["nick"] = user_info->nick;
    rtvalue["desc"] = user_info->desc;
    rtvalue["sex"] = user_info->sex;
    rtvalue["icon"] = user_info->icon;

    // 从数据库获取 申请未处理的添加好友请求列表
    std::vector<std::shared_ptr<ApplyInfo>> apply_list;
    auto b_apply = MysqlMgr::GetInstance()->GetApplyList(uid, apply_list, 0, 10);
    if(b_apply){
        for(auto &apply : apply_list){
            Json::Value obj;
            obj["name"] = apply->_name;
            obj["uid"] = apply->_uid;
            obj["icon"] = apply->_icon;
            obj["nick"] = apply->_nick;
            obj["sex"] = apply->_sex;
            obj["desc"] = apply->_desc;
            obj["status"] = apply->_status;
            rtvalue["apply_list"].append(obj);
        }
    }

    // 获取用户的好友列表
    std::vector<std::shared_ptr<UserInfo>> friend_list;
    bool b_friend_list = MysqlMgr::GetInstance()->GetFriendList(uid, friend_list);
    if(b_friend_list){
        for(auto& friend_ : friend_list){
            Json::Value obj;
            obj["name"] = friend_->name;
            obj["uid"] = friend_->uid;
            obj["icon"] = friend_->icon;
            obj["nick"] = friend_->nick;
            obj["sex"] = friend_->sex;
            obj["desc"] = friend_->desc;
            obj["back"] = friend_->back;
            rtvalue["friend_list"].append(obj);
        }
    }

    //登录到当前服务器，登录数量加1
    auto &cfg = ConfigMgr::Inst();
    auto server_name = cfg["SelfServer"]["Name"];
    auto cur_num = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server_name);
    int count = 0;
    if(!cur_num.empty()){
        count = std::stoi(cur_num);
    }
    count++;
    // key field value
    RedisMgr::GetInstance()->HSet(LOGIN_COUNT, server_name, std::to_string(count));

    //为用户设置登录ip server的名字
    std::string  ipkey = USERIPPREFIX + uid_str;
    RedisMgr::GetInstance()->Set(ipkey, server_name);

    // 设置用户会话
    UserMgr::GetInstance()->SetUserSession(uid, session);

    rtvalue["token"] = resp.token();
    std::string jsonstr = rtvalue.toStyledString();
    session->Send(jsonstr, MSG_CHAT_LOGIN_RSP);
}

// 获取用户基本信息
bool LogicSystem::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo){
    // 根据basekey 和 uid 获取用户信息
    // 先在redis中查询
    std::string info_str = "";
    bool exist = RedisMgr::GetInstance()->Get(base_key, info_str);
    if(exist){
        Json::Reader reader;
        Json::Value root;
        reader.parse(info_str,root);
        userinfo->uid = root["uid"].asInt();
        userinfo->name = root["name"].asString();
        userinfo->pwd = root["pwd"].asString();
        userinfo->email = root["email"].asString();
        userinfo->nick = root["nick"].asString();
        userinfo->desc = root["desc"].asString();
        userinfo->sex = root["sex"].asInt();
        userinfo->icon = root["icon"].asString();
        std::cout << "user login uid is  " << userinfo->uid << " name  is "
			<< userinfo->name << " pwd is " << userinfo->pwd << " email is " << userinfo->email << std::endl;
    }else{
        // redis中没有， 查询mysql
        userinfo = MysqlMgr::GetInstance()->GetUser(uid);

        if(userinfo == nullptr)
            return false;
        
        // 把数据库中的内容读取到Redis
        //将数据库内容写入redis缓存
		Json::Value redis_root;
		redis_root["uid"] = uid;
		redis_root["pwd"] = userinfo->pwd;
		redis_root["name"] = userinfo->name;
		redis_root["email"] = userinfo->email;
		redis_root["nick"] = userinfo->nick;
		redis_root["desc"] = userinfo->desc;
		redis_root["sex"] = userinfo->sex;
		redis_root["icon"] = userinfo->icon;
        RedisMgr::GetInstance()->Set(base_key, redis_root.toStyledString());
    }
    return true;
}

// 根据uid获取用户信息，处理查询用户请求
void LogicSystem::SearchInfo(std::shared_ptr<CSession>session, const short& msg_id, const std::string& msg_data){
    Json::Reader reader;
    Json::Value root;
    reader.parse(msg_data, root);
    auto uid_str = root["uid"].asString();
    auto token = root["token"].asString();
    std::cout << "SerchInfo: user uid is  " << uid_str << std::endl;

    // 根据uid获取用户信息
    Json::Value rtvalue;

    if(isPureDigit(uid_str)){
        GetUserByUid(uid_str, rtvalue);
    }else{
        GetUserByName(uid_str, rtvalue);
    }
    std::string jsonstr = rtvalue.toStyledString();
    session->Send(jsonstr, ID_SEARCH_USER_RSP);
}

// 添加好友请求
void LogicSystem::AddFriendApply(std::shared_ptr<CSession>session, const short& msg_id, const std::string& msg_data){
    Json::Reader reader;
    Json::Value root;
    reader.parse(msg_data, root);
    auto uid = root["uid"].asInt();
    auto applyname = root["applyname"].asString();
    auto bakname = root["bakname"].asString();
    // 目标用户的id
    auto touid = root["touid"].asInt();

    std::cout << "user login uid is  " << uid << " applyname  is "
        << applyname << " bakname is " << bakname << " touid is " << touid << std::endl;

    Json::Value rtvalue;
    rtvalue["error"] = ErrorCodes::Success;
    std::string rt_str = rtvalue.toStyledString();
    // 将该请求添加到数据库中。
    MysqlMgr::GetInstance()->AddFriendApply(uid, touid);

    // 查询目标用户所在的服务器
    auto to_str = std::to_string(touid);
    auto to_ip_key = USERIPPREFIX + to_str;
    std::string to_ip_value = "";
    bool b_ip = RedisMgr::GetInstance()->Get(to_ip_key, to_ip_value);
    if(!b_ip){
        // 如果没找到，直接返回， 因为请求已经存入了数据库
        session->Send(rt_str, ID_ADD_FRIEND_RSP);
        return;
    }

    // 查询是否在同一个服务器
    auto &cfg = ConfigMgr::Inst();
    auto self_server = cfg["SelfServer"]["Name"];

    // 如果在同一个服务器，直接通知对方，有加好友的请求到来
    if(to_ip_value == self_server){
        // 获取对方的会话
        auto to_session = UserMgr::GetInstance()->GetSession(touid);
        if(to_session){
            Json::Value notify;
            notify["error"] = ErrorCodes::Success;
            notify["applyuid"] = uid;
            notify["name"] = applyname;
            notify["desc"] = "";
            std::string return_str = notify.toStyledString();
            to_session->Send(return_str, ID_NOTIFY_ADD_FRIEND_REQ);
        }
        session->Send(rt_str, ID_ADD_FRIEND_RSP);
        return;
    }
    // 如果不在同一个服务器，则需要通过rpc进行通知
    std::string base_key = USER_BASE_INFO + std::to_string(uid);
    auto apply_info = std::make_shared<UserInfo>();
    bool b_info = GetBaseInfo(base_key, uid, apply_info);

    AddFriendReq req;
    req.set_touid(touid);
    req.set_applyuid(uid);
    req.set_name(applyname);
    req.set_desc("");
    if(b_info){
        req.set_icon(apply_info->icon);
        req.set_sex(apply_info->sex);
        req.set_nick(apply_info->nick);
    }
    ChatGrpcClient::GetInstance()->NotifyAddFriend(to_ip_value, req);
    session->Send(rt_str, ID_ADD_FRIEND_RSP);
}

void LogicSystem::AuthFriendApply(std::shared_ptr<CSession>session, const short& msg_id, const std::string& msg_data){
    Json::Reader reader;
    Json::Value root;
    reader.parse(msg_data, root);

    auto uid = root["uid"].asInt();
    auto touid = root["touid"].asInt();
    auto bakname = root["back"].asString();

    std::cout << "from " << uid << " auth friend to " << touid << std::endl;

    Json::Value rtvalue;
    rtvalue["error"] = ErrorCodes::Success;  

    auto user_info = std::make_shared<UserInfo>();
    std::string base_key = USER_BASE_INFO + std::to_string(touid);
    // 获取目标用户的信息
    bool b_info = GetBaseInfo(base_key, touid, user_info);
    if(b_info){
        rtvalue["name"] = user_info->name;
        rtvalue["nick"] = user_info->nick;
        rtvalue["icon"] = user_info->icon;
        rtvalue["sex"] = user_info->sex;
        rtvalue["uid"] = touid;
    }else{
        rtvalue["error"] = ErrorCodes::UidInvalid;
    }

    std::string return_str = rtvalue.toStyledString();

    // 更新数据库 -- 添加认证， 将好友申请表的申请状态置为1，表示通过
    MysqlMgr::GetInstance()->AuthFriendApply(uid, touid);
    //更新数据库 添加好友
    MysqlMgr::GetInstance()->AddFriend(uid, touid, bakname);

    // redis查询目标用户所在的 server
    auto to_str = std::to_string(touid);
    auto to_ip_key = USERIPPREFIX + to_str;
    std::string to_ip_value = "";
    bool b_ip =  RedisMgr::GetInstance()->Get(to_ip_key, to_ip_value);
    // 没找到直接返回，目标用户下次登录的时候会从数据库获取到添加上了好友
    if(!b_ip){
        session->Send(return_str, ID_AUTH_FRIEND_RSP);
        return ;
    }
    // 如果找到了，看是否与本用户在同一个server
    auto &cfg = ConfigMgr::Inst();
    auto self_server = cfg["SelfServer"]["Name"];
    // 是的话 直接通知对方 好友认证成功了
    if(to_ip_value == self_server){
        // 在同一个服务器，获取对方的session
        auto to_session = UserMgr::GetInstance()->GetSession(touid);
        if(to_session){
            Json::Value notify;
            notify["error"] = ErrorCodes::Success;
            notify["fromuid"] = uid;
            notify["touid"] = touid;
            std::string base_key = USER_BASE_INFO + std::to_string(uid);
            auto user_info = std::make_shared<UserInfo>();
            bool b_info = GetBaseInfo(base_key, uid, user_info);
            if(b_info){
                notify["name"] = user_info->name;
                notify["nick"] = user_info->nick;
                notify["icon"] = user_info->icon;
                notify["sex"] = user_info->sex;
            }else{
                notify["error"] = ErrorCodes::UidInvalid;
            }
            std::string rt_str = notify.toStyledString();
            to_session->Send(rt_str, ID_NOTIFY_AUTH_FRIEND_REQ);
        }
        session->Send(return_str, ID_AUTH_FRIEND_RSP);
        return ;
    }

    // 如果不在同一个server 使用grpc 发送给所在的server 通知他
    AuthFriendReq auth_req;
    auth_req.set_fromuid(uid);
    auth_req.set_touid(touid);
    // 发送grpc请求， 告诉对方。
    ChatGrpcClient::GetInstance()->NotifyAuthFriend(to_ip_value, auth_req);
    session->Send(return_str, ID_AUTH_FRIEND_RSP);
}

void LogicSystem::DealChatMsg(std::shared_ptr<CSession>session, const short& msg_id, const std::string& msg_data){
    Json::Reader reader;
    Json::Value root;
    reader.parse(msg_data, root);

    auto uid = root["fromuid"].asInt();
    auto touid = root["touid"].asInt();
    // 发送消息的内容 使用消息数组的形式进行存储
    const Json::Value arrays = root["text_array"];

    std::cout << "from " << uid << " auth friend to " << touid << std::endl;

    Json::Value rtvalue;
    rtvalue["error"] = ErrorCodes::Success;
    rtvalue["fromuid"] = uid;
    rtvalue["touid"] = touid;
    rtvalue["text_array"] = arrays;

    std::string return_str = rtvalue.toStyledString();

    // redis查询目标用户所在的 server
    auto to_str = std::to_string(touid);
    auto to_ip_key = USERIPPREFIX + to_str;
    std::string to_ip_value = "";
    bool b_ip =  RedisMgr::GetInstance()->Get(to_ip_key, to_ip_value);
    // 没找到直接返回，目标用户下次登录的时候会从数据库获取到添加上了好友
    if(!b_ip){
        session->Send(return_str, ID_TEXT_CHAT_MSG_RSP);
        return ;
    }
    // 如果找到了，看是否与本用户在同一个server
    auto &cfg = ConfigMgr::Inst();
    auto self_server = cfg["SelfServer"]["Name"];
    // 如果在同一个服务器上，
    if(to_ip_value == self_server){
        // 在同一个服务器，获取对方的session
        auto to_session = UserMgr::GetInstance()->GetSession(touid);
        if(to_session){
            // 对方在内存中，直接发送给对方。
            std::string rt_str = rtvalue.toStyledString();
            // 对方的客户端 接收到请求之后，直接处理消息即可。
            to_session->Send(rt_str, ID_NOTIFY_TEXT_CHAT_MSG_REQ);
        }
        session->Send(return_str, ID_TEXT_CHAT_MSG_RSP);
        return ;
    }

    // 如果不在同一个server 使用grpc 发送给所在的server 通知他
    TextChatMsgReq  text_req;
    text_req.set_fromuid(uid);
    text_req.set_touid(touid);
    // 将消息添加进去
    for(const auto& text : arrays){
        auto content = text["content"].asString();
        auto msgid = text["msgid"].asString();
        std::cout << "content is " << content << std::endl;
        std::cout << "msgid is " << msgid << std::endl;
        // 将消息元素添加到text_req，并返回添加元素的指针
        auto *text_msg = text_req.add_textmsgs();
        text_msg->set_msgid(msgid);
        text_msg->set_msgcontent(content);
    }
    // 发送grpc请求， 告诉对方。
    ChatGrpcClient::GetInstance()->NotifyTextChatMsg(to_ip_value, text_req, rtvalue);
    session->Send(return_str, ID_TEXT_CHAT_MSG_RSP);
}


//-------------------- 辅助函数--------------------
bool LogicSystem::isPureDigit(std::string uid){
    for(char c : uid){
        if(!std::isdigit(c)){
            return false;
        }
    }
    return true;
}

void LogicSystem::GetUserByUid(std::string uid_str, Json::Value& rtvalue){
    rtvalue["error"] = ErrorCodes::Success;
    std::string base_key = USER_BASE_INFO + uid_str;
    // 先从redis中查询用户数据， 在redis中说明用户在线。
    std::string info_str = "";
    auto exist = RedisMgr::GetInstance()->Get(base_key, info_str);
    if(exist){
        Json::Reader reader;
        Json::Value root;
        reader.parse(info_str, root);
        auto uid = root["uid"].asInt();
		auto name = root["name"].asString();
		auto pwd = root["pwd"].asString();
		auto email = root["email"].asString();
		auto nick = root["nick"].asString();
		auto desc = root["desc"].asString();
		auto sex = root["sex"].asInt();
		auto icon = root["icon"].asString();
		std::cout << "user  uid is  " << uid << " name  is "
			<< name << " pwd is " << pwd << " email is " << email << " icon is " << icon << std::endl;

        rtvalue["uid"] = uid;
		rtvalue["pwd"] = pwd;
		rtvalue["name"] = name;
		rtvalue["email"] = email;
		rtvalue["nick"] = nick;
		rtvalue["desc"] = desc;
		rtvalue["sex"] = sex;
		rtvalue["icon"] = icon;
		return;
    }

    // redis中没有，再去查询数据库
    auto uid = std::stoi(uid_str);
    std::shared_ptr<UserInfo> user_info = nullptr;
    user_info = MysqlMgr::GetInstance()->GetUser(uid);
    if(user_info == nullptr){
        rtvalue["error"] = ErrorCodes::UidInvalid;
        return ;
    }
    // 将数据库的内容写到redis缓存
	Json::Value redis_root;
	redis_root["uid"] = user_info->uid;
	redis_root["pwd"] = user_info->pwd;
	redis_root["name"] = user_info->name;
	redis_root["email"] = user_info->email;
	redis_root["nick"] = user_info->nick;
	redis_root["desc"] = user_info->desc;
	redis_root["sex"] = user_info->sex;
	redis_root["icon"] = user_info->icon;

	RedisMgr::GetInstance()->Set(base_key, redis_root.toStyledString());

	// 返回数据
	rtvalue["uid"] = user_info->uid;
	rtvalue["pwd"] = user_info->pwd;
	rtvalue["name"] = user_info->name;
	rtvalue["email"] = user_info->email;
	rtvalue["nick"] = user_info->nick;
	rtvalue["desc"] = user_info->desc;
	rtvalue["sex"] = user_info->sex;
	rtvalue["icon"] = user_info->icon;
    return ;
}

void LogicSystem::GetUserByName(std::string name, Json::Value& rtvalue){
    rtvalue["error"] = ErrorCodes::Success;
    std::string base_key = USER_BASE_INFO + name;
    // 先从redis中查询用户数据， 在redis中说明用户在线。
    std::string info_str = "";
    auto exist = RedisMgr::GetInstance()->Get(base_key, info_str);
    if(exist){
        Json::Reader reader;
        Json::Value root;
        reader.parse(info_str, root);
        auto uid = root["uid"].asInt();
		auto name = root["name"].asString();
		auto pwd = root["pwd"].asString();
		auto email = root["email"].asString();
		auto nick = root["nick"].asString();
		auto desc = root["desc"].asString();
		auto sex = root["sex"].asInt();
		auto icon = root["icon"].asString();
		std::cout << "user  uid is  " << uid << " name  is "
			<< name << " pwd is " << pwd << " email is " << email << " icon is " << icon << std::endl;

        rtvalue["uid"] = uid;
		rtvalue["pwd"] = pwd;
		rtvalue["name"] = name;
		rtvalue["email"] = email;
		rtvalue["nick"] = nick;
		rtvalue["desc"] = desc;
		rtvalue["sex"] = sex;
		rtvalue["icon"] = icon;
		return;
    }

    // redis中没有，再去查询数据库
    std::shared_ptr<UserInfo> user_info = nullptr;
    user_info = MysqlMgr::GetInstance()->GetUser(name);
    if(user_info == nullptr){
        rtvalue["error"] = ErrorCodes::UidInvalid;
        return ;
    }
    // 将数据库的内容写到redis缓存
	Json::Value redis_root;
	redis_root["uid"] = user_info->uid;
	redis_root["pwd"] = user_info->pwd;
	redis_root["name"] = user_info->name;
	redis_root["email"] = user_info->email;
	redis_root["nick"] = user_info->nick;
	redis_root["desc"] = user_info->desc;
	redis_root["sex"] = user_info->sex;
	redis_root["icon"] = user_info->icon;

	RedisMgr::GetInstance()->Set(base_key, redis_root.toStyledString());

	// 返回数据
	rtvalue["uid"] = user_info->uid;
	rtvalue["pwd"] = user_info->pwd;
	rtvalue["name"] = user_info->name;
	rtvalue["email"] = user_info->email;
	rtvalue["nick"] = user_info->nick;
	rtvalue["desc"] = user_info->desc;
	rtvalue["sex"] = user_info->sex;
	rtvalue["icon"] = user_info->icon;
    return ;
}