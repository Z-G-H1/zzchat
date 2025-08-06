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

class VarifyGrpcClient : public Singleton<VarifyGrpcClient>{
    friend Singleton<VarifyGrpcClient>;
public:
    GetVarifyRsp GetVarifyCode(std::string email);
private:
    VarifyGrpcClient();
    // 使用智能指针 管理 gRPC 客户端存根 它的所有方法（如 GetVarifyCode）直接对应服务端实现的同名方法
    std::unique_ptr<VarifyService::Stub> stub_;
};