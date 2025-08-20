#include "ChatServiceImpl.h"
#include "UserMgr.h"
#include "CSession.h"

ChatServiceImpl::ChatServiceImpl(){

}
// 实现方法
Status ChatServiceImpl::NotifyAddFriend(ServerContext *context, const AddFriendReq *request, AddFriendRsp *reply){
    auto touid = request->touid();
    auto session = UserMgr::GetInstance()->GetSession(touid);

    //用户不在内存中则直接返回
    if (session == nullptr) {
        return Status::OK;
    }
    reply->set_error(ErrorCodes::Success);
    reply->set_applyuid(request->applyuid());
    reply->set_touid(request->touid());

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

bool ChatServiceImpl::GetBaseUserInfo(std::string base_key, int uid, std::shared_ptr<UserInfo> userinfo){
    return true;
}

Status ChatServiceImpl::NotifyAuthFriend(ServerContext* context,const AuthFriendReq* request, AuthFriendRsp* response){
    return Status::OK;
}

Status ChatServiceImpl::NotifyTextChatMsg(::grpc::ServerContext* context,const TextChatMsgReq* request, TextChatMsgRsp* response){
    return Status::OK;
}