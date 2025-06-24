#pragma once
#include "const.h"
#include "data.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using message::AddFriendReq;
using message::AddFriendRsp;

using message::AuthFriendReq;
using message::AuthFriendRsp;

using message::GetChatServerRsp;
using message::ChatService;

using message::TextChatMsgReq;
using message::TextChatMsgRsp;
using message::TextChatData;

class ChatServiceImpl final : public ChatService::Service
{
public:
    ChatServiceImpl();
    Status NotifyAddFriend(::grpc::ServerContext* context, const ::message::AddFriendReq* request, ::message::AddFriendRsp* response) override;
    Status NotifyAuthFriend(::grpc::ServerContext* context, const ::message::AuthFriendReq* request, ::message::AuthFriendRsp* response) override;
    Status NotifyTextChatMsg(::grpc::ServerContext* context, const ::message::TextChatMsgReq* request, ::message::TextChatMsgRsp* response) override;

    bool GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo);
private:

};

