#include "StatusServiceImpl.h"
#include "ConfigMgr.h"

std::string generate_unique_string(){
    // 创建UUID对象
    boost::uuids::uuid uuid = boost::uuids::random_generator()();

    // 将UUID转换为字符串
    std::string unique_string = to_string(uuid);

    return unique_string;
}

StatusServiceImpl::StatusServiceImpl() : _server_index(0){
    auto &cfg_mgr = ConfigMgr::Inst();
    ChatServer server;
    server.host = cfg_mgr["ChatServer1"]["Host"];
    server.port = cfg_mgr["ChatServer1"]["Port"];
    _servers.push_back(server);

    server.host = cfg_mgr["ChatServer2"]["Host"];
    server.port = cfg_mgr["ChatServer2"]["Port"];
    _servers.push_back(server);
}
// 重写具体的 RPC 方法
Status StatusServiceImpl::GetChatServer(ServerContext *context, const GetChatServerReq *request, GetChatServerRsp *reply){
    std::string prefix("status server has received : ");
    _server_index = (_server_index++) % (_servers.size());
    auto &server = _servers[_server_index];
    reply->set_host(server.host);
    reply->set_port(server.port);
    reply->set_error(ErrorCodes::Success);
    reply->set_token(generate_unique_string());
    return Status::OK;
}