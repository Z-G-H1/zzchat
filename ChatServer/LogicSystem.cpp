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
}

LogicSystem::~LogicSystem(){
    _b_stop = true;
    _consume.notify_one();
    _worker_thread.join();
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
		        std::cout << "msg is is " << msg_node->_recv_node->msg_id << std::endl;
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

    // 从数据库获取好友申请列表
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
    session->Send(jsonstr, msg_id);
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