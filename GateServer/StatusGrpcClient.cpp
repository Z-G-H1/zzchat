#include "StatusGrpcClient.h"
#include "ConfigMgr.h"

StatusGrpcClient::StatusGrpcClient(){
    auto &cfg_mgr = ConfigMgr::Inst();
    std::string host = cfg_mgr["StatusServer"]["Host"];
    std::string port = cfg_mgr["StatusServer"]["Port"];

    pool_.reset(new StatusConPool(5, host, port));
}

GetChatServerRsp StatusGrpcClient::GetChatServer(int uid){
    ClientContext context;
    GetChatServerReq req;
    GetChatServerRsp resp;

    req.set_uid(uid);
    auto stub = pool_->getConnection();
    Status status = stub->GetChatServer(&context, req, &resp);

    if(status.ok()){
        pool_->returnConnection(std::move(stub));
        return resp;
    }else{
        pool_->returnConnection(std::move(stub));
        resp.set_error(ErrorCodes::RPCFailed);
        return resp;
    }
}

StatusGrpcClient::~StatusGrpcClient(){
    
}