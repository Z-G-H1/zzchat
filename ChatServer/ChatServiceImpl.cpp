#include "ChatServiceImpl.h"
#include "UserMgr.h"
#include "CSession.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"

ChatServiceImpl::ChatServiceImpl(){

}

Status ChatServiceImpl::NotifyAddFriend(ServerContext *context, const AddFriendReq *request, AddFriendRsp *reply){
    auto touid = request->touid();
    auto session = UserMgr::GetInstance()->GetSession(touid);

    reply->set_error(ErrorCodes::Success);
    reply->set_applyuid(request->applyuid());
    reply->set_touid(request->touid());

    //用户不在内存中则直接返回
    if (session == nullptr) {
        return Status::OK;
    }

    Json::Value  rtvalue;
    rtvalue["error"] = ErrorCodes::Success;
    rtvalue["applyuid"] = request->applyuid();
    rtvalue["name"] = request->name();
    rtvalue["desc"] = request->desc();
    rtvalue["icon"] = request->icon();
    rtvalue["sex"] = request->sex();
    rtvalue["nick"] = request->nick();


    std::string return_str = rtvalue.toStyledString();
    session->Send(return_str, ID_NOTIFY_ADD_FRIEND_REQ);
    return Status::OK;
}

Status ChatServiceImpl::NotifyAuthFriend(ServerContext* context,const AuthFriendReq* request, AuthFriendRsp* reply){
    auto touid = request->touid();
    auto fromuid = request->fromuid();
    auto session = UserMgr::GetInstance()->GetSession(touid);

    reply->set_error(ErrorCodes::Success);
    reply->set_fromuid(fromuid);
    reply->set_touid(touid);

    //用户不在内存中则直接返回
    if (session == nullptr) {
        return Status::OK;
    }

    Json::Value rtvalue;
    rtvalue["error"] = ErrorCodes::Success;
    rtvalue["fromuid"] = fromuid;
    rtvalue["touid"] = touid;

    std::string base_key = USER_BASE_INFO + std::to_string(fromuid);
    auto user_info = std::make_shared<UserInfo>();
    bool b_info = GetBaseUserInfo(base_key, fromuid, user_info);
        if (b_info) {
        rtvalue["name"] = user_info->name;
        rtvalue["nick"] = user_info->nick;
        rtvalue["icon"] = user_info->icon;
        rtvalue["sex"] = user_info->sex;
    }
    else {
        rtvalue["error"] = ErrorCodes::UidInvalid;
    }

    std::string return_str = rtvalue.toStyledString();
    session->Send(return_str, ID_NOTIFY_AUTH_FRIEND_REQ);
    return Status::OK;
}

Status ChatServiceImpl::NotifyTextChatMsg(::grpc::ServerContext* context,
    const TextChatMsgReq* request, TextChatMsgRsp* reply) {
    //查找用户是否在本服务器
    auto touid = request->touid();
    auto session = UserMgr::GetInstance()->GetSession(touid);
    reply->set_error(ErrorCodes::Success);

    //用户不在内存中则直接返回
    if (session == nullptr) {
        return Status::OK;
    }

    //在内存中则直接发送通知对方
    Json::Value  rtvalue;
    rtvalue["error"] = ErrorCodes::Success;
    rtvalue["fromuid"] = request->fromuid();
    rtvalue["touid"] = request->touid();

    //将聊天数据组织为数组
    Json::Value text_array;
    for (auto& msg : request->textmsgs()) {
        Json::Value element;
        element["content"] = msg.msgcontent();
        element["msgid"] = msg.msgid();
        text_array.append(element);
    }
    rtvalue["text_array"] = text_array;

    std::string return_str = rtvalue.toStyledString();

    session->Send(return_str, ID_NOTIFY_TEXT_CHAT_MSG_REQ);
    return Status::OK;
}

bool ChatServiceImpl::GetBaseUserInfo(std::string base_key, int uid, std::shared_ptr<UserInfo> userinfo){
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