#include "StatusServiceImpl.h"
#include "ConfigMgr.h"
#include "RedisMgr.h"

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
    const auto &server = getChatServer();
    reply->set_host(server.host);
    reply->set_port(server.port);
    reply->set_error(ErrorCodes::Success);
    reply->set_token(generate_unique_string());
    insertToken(request->uid(), reply->token());
    return Status::OK;
}

void StatusServiceImpl::insertToken(int uid, std::string token){
    // 插入到Redis中存储
    std::string uid_str = std::to_string(uid);
    std::string key = USERTOKENPREFIX + uid_str;
    RedisMgr::GetInstance()->Set(key, token);
}

ChatServer StatusServiceImpl::getChatServer(){
    std::lock_guard<std::mutex> lock(_server_mtx);
    auto minserver = _servers.begin()->second;
    auto count_str = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, minserver.name);
    // 不存在该服务器, 则不使用
    if(count_str.empty()){
        // 数目设为最大值
        minserver.con_count = INT_MAX;
    }else{
        minserver.con_count = std::stoi(count_str);
    }
    
    for(auto &server : _servers){
        if (server.second.name == minserver.name) {
            continue;
        }

        // 获取当前服务器的数量
        auto count = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server.second.name);
        if(count.empty()){
            continue;
        }
        server.second.con_count = std::stoi(count);
        if(server.second.con_count < minserver.con_count){
            minserver = server.second;
        }
    }
    return minserver;
}


Status StatusServiceImpl::Login(ServerContext* context, const LoginReq* request,LoginRsp* reply){
    auto uid = request->uid();
    auto token = request->token();
    
    std::string uid_str = std::to_string(uid);
    std::string key = USERTOKENPREFIX + uid_str;
    std::string token_redis = "";
    bool exist = RedisMgr::GetInstance()->Get(key, token_redis);

    if(!exist){
        reply->set_error(ErrorCodes::UidInvalid);
        return Status::OK;
    }
    
    if(token != token_redis){
        reply->set_error(ErrorCodes::TokenInvalid);
        return Status::OK;
    }

    reply->set_error(ErrorCodes::Success);
	reply->set_uid(uid);
	reply->set_token(token);
	return Status::OK;
}