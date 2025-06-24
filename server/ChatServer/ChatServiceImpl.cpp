#include "ChatServiceImpl.h"
#include "UserMgr.h"
#include "CServer.h"
ChatServiceImpl::ChatServiceImpl() {

}

Status ChatServiceImpl::NotifyAddFriend(::grpc::ServerContext* context, const ::message::AddFriendReq* request, 
	::message::AddFriendRsp* reply) {
    //�����û��Ƿ��ڱ�������
    auto touid = request->touid();
    auto session = UserMgr::GetInstance()->GetSession(touid);

    Defer defer([request, reply]() {
        reply->set_error(ErrorCodes::Success);
        reply->set_applyuid(request->applyuid());
        reply->set_touid(request->touid());
        });

    //�û������ڴ�����ֱ�ӷ���
    if (session == nullptr) {
        return Status::OK;
    }

    //���ڴ�����ֱ�ӷ���֪ͨ�Է�
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