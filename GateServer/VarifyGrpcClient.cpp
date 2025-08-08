#include "VarifyGrpcClient.h"
#include "ConfigMgr.h"

VarifyGrpcClient::VarifyGrpcClient(){
    auto &cfg_mgr = ConfigMgr::Inst();
    std::string host = cfg_mgr["VarifyServer"]["Host"];
    std::string port = cfg_mgr["VarifyServer"]["Port"];

    _pool.reset(new RpcConPool(5,host,port));
}

GetVarifyRsp VarifyGrpcClient::GetVarifyCode(std::string email){
    ClientContext context;
    GetVarifyRsp reply;
    GetVarifyReq request;

    request.set_email(email);
    // Status status = stub_->GetVarifyCode(&context, request, &reply);
    auto stub = _pool->getConnection();
    Status status = stub->GetVarifyCode(&context,request,&reply);

    if(status.ok()){
        _pool->returnConnection(std::move(stub));
        return reply;
    }else{
        _pool->returnConnection(std::move(stub));
        reply.set_error(ErrorCodes::RPCFailed);
        return reply;
    }
}
