#include "ChatServiceImpl.h"
#include "UserMgr.h"
#include "CServer.h"
ChatServiceImpl::ChatServiceImpl() {

}

Status ChatServiceImpl::NotifyAddFriend(::grpc::ServerContext* context, const ::message::AddFriendReq* request, 
	::message::AddFriendRsp* reply) {
    //查找用户是否在本服务器
    auto touid = request->touid();
    auto session = UserMgr::GetInstance()->GetSession(touid);

    Defer defer([request, reply]() {
        reply->set_error(ErrorCodes::Success);
        reply->set_applyuid(request->applyuid());
        reply->set_touid(request->touid());
        });

    //用户不在内存中则直接返回
    if (session == nullptr) {
        return Status::OK;
    }

    //在内存中则直接发送通知对方
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
Status ChatServiceImpl::NotifyAuthFriend(::grpc::ServerContext* context, const ::message::AuthFriendReq* request, 
	::message::AuthFriendRsp* response) {
    //查找用户是否在本服务器
    auto touid = request->touid();
    auto fromuid = request->fromuid();
    auto session = UserMgr::GetInstance()->GetSession(touid);

    Defer defer([request, reply]() {
        reply->set_error(ErrorCodes::Success);
        reply->set_fromuid(request->fromuid());
        reply->set_touid(request->touid());
        });

    //用户不在内存中则直接返回
    if (session == nullptr) {
        return Status::OK;
    }

    //在内存中则直接发送通知对方
    Json::Value  rtvalue;
    rtvalue["error"] = ErrorCodes::Success;
    rtvalue["fromuid"] = request->fromuid();
    rtvalue["touid"] = request->touid();

    std::string base_key = USER_BASE_INFO + std::to_string(fromuid);
    auto user_info = std::make_shared<UserInfo>();
    bool b_info = GetBaseInfo(base_key, fromuid, user_info);
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
Status ChatServiceImpl::NotifyTextChatMsg(::grpc::ServerContext* context, const ::message::TextChatMsgReq* request, 
	::message::TextChatMsgRsp* response) {
	return Status::OK;
}


bool ChatServiceImpl::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo)
{
	return true;
}