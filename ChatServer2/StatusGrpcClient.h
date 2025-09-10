#include <grpcpp/grpcpp.h>

#include "const.h"
#include "Singleton.h"
#include "message.grpc.pb.h"
#include "message.pb.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::LoginReq;
using message::LoginRsp;
using message::StatusService;

class StatusConPool{
public:
    StatusConPool(size_t poolSize, std::string host, std::string port)
        : poolSize_(poolSize), host_(host), port_(port), b_stop_(false)
    {
        for(size_t i=0; i<poolSize_; i++){
            std::shared_ptr<Channel> channel = grpc::CreateChannel(host+":"+port, grpc::InsecureChannelCredentials());
            connections_.push(StatusService::NewStub(channel));
        }
    }

    ~StatusConPool(){
        std::lock_guard<std::mutex> lock(mutex_);
        Close();
        while(!connections_.empty()){
            connections_.pop();
        }
    }

    void Close(){
        b_stop_ = true;
        cond_.notify_all();
    }

    std::unique_ptr<StatusService::Stub> getConnection(){
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this]{
            if(b_stop_){
                return true;    //解锁
            }
            return !connections_.empty();
        });
        if(b_stop_){
            return nullptr;
        }
        auto con = std::move(connections_.front());
        connections_.pop();
        return con;
    }

    void returnConnection(std::unique_ptr<StatusService::Stub> con){
        std::lock_guard<std::mutex> lock(mutex_);
        if(b_stop_){
            return ;
        }
        connections_.push(std::move(con));
        cond_.notify_one();
        return;
    }

private:

    std::atomic<bool> b_stop_;
    size_t poolSize_;
    std::string host_;
    std::string port_;
    std::queue<std::unique_ptr<StatusService::Stub>> connections_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

class StatusGrpcClient : public Singleton<StatusGrpcClient>
{
    friend class Singleton<StatusGrpcClient>;
public:
    ~StatusGrpcClient();
    //     rpc Login(LoginReq) returns(LoginRsp);
    LoginRsp Login(int uid, std::string token);
	GetChatServerRsp GetChatServer(int uid);

private:
    StatusGrpcClient();

    std::unique_ptr<StatusConPool> pool_;
};
