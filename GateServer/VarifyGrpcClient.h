#pragma once
#include "grpcpp/grpcpp.h"
#include "message.grpc.pb.h"
#include "const.h"
#include "Singleton.h"


using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;

class RpcConPool{
public:
    RpcConPool(size_t poolSize, std::string host, std::string port)
        : poolSize_(poolSize), host_(host), port_(port),b_stop_(false)
    {
        for(size_t i=0; i<poolSize_; i++){
            std::shared_ptr<Channel> channel = grpc::CreateChannel(host+":"+port, grpc::InsecureChannelCredentials());
            connections_.push(VarifyService::NewStub(channel));
        }
    }

    ~RpcConPool(){
        std::lock_guard<std::mutex> lock(mutex_);
        Close();
        while (!connections_.empty()) {
            connections_.pop();
        }
    }

    std::unique_ptr<VarifyService::Stub> getConnection(){
        std::unique_lock<std::mutex> lock(mutex_);
        // ----------------------------------------------------------为什么这么写》？ 逻辑没懂
        cond_.wait(lock,[this]{
            if(b_stop_){
                return true;
            }
            return !connections_.empty();
        });

        if(b_stop_){
            return nullptr;
        }
        auto context = std::move(connections_.front());
        connections_.pop();
        return context;
    }

    void returnConnection(std::unique_ptr<VarifyService::Stub> context){
        std::lock_guard<std::mutex> lock(mutex_);
        if (b_stop_) {
            return;
        }
        // 使用move，因为uniqueptr 要独占所有权 不可复制，只能move
        connections_.push(std::move(context));
        // 队列中有了新的资源，唤醒一个
        cond_.notify_one();
        return;
    }

    void Close(){
        b_stop_ = true;
        cond_.notify_all();
    }

private:
    std::atomic<bool> b_stop_;
    size_t poolSize_;
    std::string host_;
    std::string port_;
    std::queue<std::unique_ptr<VarifyService::Stub>> connections_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

class VarifyGrpcClient : public Singleton<VarifyGrpcClient>{
    friend Singleton<VarifyGrpcClient>;
public:
    GetVarifyRsp GetVarifyCode(std::string email);
private:
    VarifyGrpcClient();
    // 使用智能指针 管理 gRPC 客户端存根 它的所有方法（如 GetVarifyCode）直接对应服务端实现的同名方法
    std::unique_ptr<VarifyService::Stub> stub_;
    // 使用uniqueptr 独占pool的生命周期
    std::unique_ptr<RpcConPool> _pool;
};