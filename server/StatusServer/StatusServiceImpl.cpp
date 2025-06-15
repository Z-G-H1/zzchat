#include "StatusServiceImpl.h"
#include "ConfigMgr.h"
#include "const.h"



std::string generate_unique_string() {  
    // 创建uuid对象
    boost::uuids::uuid uuid = boost::uuids::random_generator()();  
	// 将uuid转换为字符串
	std::string unique_string = boost::uuids::to_string(uuid);
    return unique_string;  
}

Status StatusServiceImpl::GetChatServer(ServerContext* context, const GetChatServerReq* request, GetChatServerRsp* reply) {
    std::string prefix("zz status server has received : ");
    _server_index = (_server_index++) % (_servers.size());
	auto& server = _servers[_server_index];
    reply->set_host(server.host);
    reply->set_port(server.port);
    reply->set_error(ErrorCodes::Success);
    reply->set_token(generate_unique_string());
    return Status::OK;
}


StatusServiceImpl::StatusServiceImpl() : _server_index(0) {
    auto& config = ConfigMgr::Inst();
    ChatServer server;
    server.port = config["ChatServer1"]["Port"];
	server.host = config["ChatServer1"]["Host"];
    _servers.push_back(server);

	server.port = config["ChatServer2"]["Port"];
    server.host = config["ChatServer2"]["Host"];
	_servers.push_back(server);
}