#include "ChatServiceImpl.h"

ChatServiceImpl::ChatServiceImpl() {

}

Status ChatServiceImpl::NotifyAddFriend(::grpc::ServerContext* context, const ::message::AddFriendReq* request, 
	::message::AddFriendRsp* response) {
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