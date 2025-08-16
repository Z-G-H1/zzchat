#include "StatusServiceImpl.h"
#include "ConfigMgr.h"

std::string generate_unique_string(){
    // 创建UUID对象
    boost::uuids::uuid uuid = boost::uuids::random_generator()();

    // 将UUID转换为字符串
    std::string unique_string = to_string(uuid);

    return unique_string;
}

StatusServiceImpl::StatusServiceImpl(){
    auto &cfg_mgr = ConfigMgr::Inst();
    ChatServer server;
    server.host = cfg_mgr["ChatServer1"]["Host"];
    server.port = cfg_mgr["ChatServer1"]["Port"];
    server.con_count = 0;
    server.name = cfg_mgr["ChatServer1"]["Name"];
    _servers[server.name] = server;

    server.host = cfg_mgr["ChatServer2"]["Host"];
    server.port = cfg_mgr["ChatServer2"]["Port"];
    server.con_count = 0;
    server.name = cfg_mgr["ChatServer2"]["Name"];
    _servers[server.name] = server;
}
// 重写具体的 RPC 方法
Status StatusServiceImpl::GetChatServer(ServerContext *context, const GetChatServerReq *request, GetChatServerRsp *reply){
    std::string prefix("status server has received : ");

    const auto &server = getChatServer();
    reply->set_host(server.host);
    reply->set_port(server.port);
    reply->set_error(ErrorCodes::Success);
    reply->set_token(generate_unique_string());
    insertToken(request->uid(), reply->token());
    return Status::OK;
}

void StatusServiceImpl::insertToken(int uid, std::string token){
    std::lock_guard<std::mutex> lock(_token_mtx);
    _tokens[uid] = token;
}  

ChatServer StatusServiceImpl::getChatServer(){
    std::lock_guard<std::mutex> lock(_server_mtx);
    auto minserver = _servers.begin()->second;
    for(auto &server : _servers){
        if(server.second.con_count < minserver.con_count){
            minserver = server.second;
        }
    }
    return minserver;
}


Status StatusServiceImpl::Login(ServerContext* context, const LoginReq* request,LoginRsp* reply){
    std::lock_guard<std::mutex> lock(_token_mtx);
    auto uid = request->uid();
    auto token = request->token();
    auto iter = _tokens.find(uid);
    
    if(iter == _tokens.end()){
        // 没有找到token
        reply->set_error(ErrorCodes::UidInvalid);
        return Status::OK;
    }

    if(iter->second != token){
        reply->set_error(ErrorCodes::TokenInvalid);
		return Status::OK;
    }

    reply->set_error(ErrorCodes::Success);
	reply->set_uid(uid);
	reply->set_token(iter->second);
	return Status::OK;
}