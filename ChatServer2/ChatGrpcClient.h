#pragma once
#include <grpcpp/grpcpp.h>
#include "const.h"
#include "Singleton.h"
#include "message.grpc.pb.h"
#include "message.pb.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::AddFriendReq;
using message::AddFriendRsp;
using message::AuthFriendRsp;
using message::AuthFriendReq;
using message::TextChatMsgRsp;
using message::TextChatMsgReq;
using message::TextChatData;
using message::ChatService;


class ChatConPool{
public:
    ChatConPool(size_t poolSize, std::string host, std::string port)
        :poolSize_(poolSize),host_(host), port_(port),b_stop_(false)
    {
        for(size_t i=0; i<poolSize_; i++){
            std::shared_ptr<Channel> channel = grpc::CreateChannel(host+":"+port, grpc::InsecureChannelCredentials());
            connections_.push(ChatService::NewStub(channel));
        }
    }

    ~ChatConPool(){
        std::lock_guard<std::mutex> lock(_mutex);
        Close();
        while(!connections_.empty()){
            connections_.pop();
        }
    }

    void Close(){
        b_stop_ = true;
        _cond.notify_all();
    }

    std::unique_ptr<ChatService::Stub> getConnection(){
        std::unique_lock<std::mutex> lock(_mutex);
        _cond.wait(lock,[this]{
            if(b_stop_)
                return true;
            return !connections_.empty();
        });
        
        if(b_stop_){
            return nullptr;
        }
        auto con = std::move(connections_.front());
        connections_.pop();
        return con;
    }

    void returnConnection(std::unique_ptr<ChatService::Stub> con){
        std::lock_guard<std::mutex> lock(_mutex);
        if(b_stop_){
            return;
        }
        connections_.push(std::move(con));
        _cond.notify_one();
        return;
    }

private:
    std::atomic<bool> b_stop_;
    size_t poolSize_;
    std::string host_;
    std::string port_;
    std::queue<std::unique_ptr<ChatService::Stub>> connections_;
    std::mutex _mutex;
    std::condition_variable _cond;
};

class ChatGrpcClient : public Singleton<ChatGrpcClient>{
    friend Singleton<ChatGrpcClient>;
public:
    ~ChatGrpcClient(){
	    std::cout << "Destruct ChatGrpcClient" << std::endl;
    }
	// rpc NotifyAddFriend(AddFriendReq) returns (AddFriendRsp) {}
    AddFriendRsp NotifyAddFriend(std::string server_ip, const AddFriendReq& req);
    AuthFriendRsp NotifyAuthFriend(std::string server_ip, const AuthFriendReq& req);
    bool GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo);
    TextChatMsgRsp NotifyTextChatMsg(std::string server_ip, const TextChatMsgReq& req, const Json::Value& rtvalue);
private:
    ChatGrpcClient();
    // 对端可能有多个聊天服务器，需要区分
    std::unordered_map<std::string, std::unique_ptr<ChatConPool>> _pools;
};