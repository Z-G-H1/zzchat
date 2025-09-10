#include "ChatGrpcClient.h"
#include "ConfigMgr.h"

ChatGrpcClient::ChatGrpcClient(){
    auto& cfg = ConfigMgr::Inst();
    // servers字符串用 逗号 进行了分隔
    auto server_list = cfg["PeerServer"]["Servers"];

    std::vector<std::string> words;
    std::stringstream ss(server_list);
    std::string word;

    while(std::getline(ss, word, ',')){
        words.push_back(word);
    }

    for(auto& word : words){
        // 检查是否存在该服务器
        if(cfg[word]["Name"].empty()){
            continue;
        }
        _pools[cfg[word]["Name"]] = std::make_unique<ChatConPool>(5, cfg[word]["Host"], cfg[word]["Port"]);
    }
}

AddFriendRsp ChatGrpcClient::NotifyAddFriend(std::string server_ip, const AddFriendReq &req){
    AddFriendRsp resp;
    resp.set_error(ErrorCodes::Success);
    resp.set_applyuid(req.applyuid());
    resp.set_touid(req.touid());

    auto find_iter = _pools.find(server_ip);
    if(find_iter == _pools.end()){
        return resp;
    }
    // 获取该serverip 对应的连接池
    auto &pool = find_iter->second;
    ClientContext context;
    auto stub = pool->getConnection();
    Status status = stub->NotifyAddFriend(&context, req, &resp);
    
    pool->returnConnection(std::move(stub));

    if(!status.ok()){
        resp.set_error(ErrorCodes::RPCFailed);
        return resp;
    }

    return resp;
}

AuthFriendRsp ChatGrpcClient::NotifyAuthFriend(std::string server_ip, const AuthFriendReq& req){
    AuthFriendRsp resp;
    resp.set_error(ErrorCodes::Success);
    resp.set_fromuid(req.fromuid());
    resp.set_touid(req.touid());

    auto find_iter = _pools.find(server_ip);
    if(find_iter == _pools.end()){
        return resp;
    }
    // 获取该serverip 对应的连接池
    auto &pool = find_iter->second;
    ClientContext context;
    auto stub = pool->getConnection();
    Status status = stub->NotifyAuthFriend(&context, req, &resp);
    
    pool->returnConnection(std::move(stub));

    if(!status.ok()){
        resp.set_error(ErrorCodes::RPCFailed);
        return resp;
    }

    return resp;
}

bool ChatGrpcClient::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo){
    return true;
}

TextChatMsgRsp ChatGrpcClient::NotifyTextChatMsg(std::string server_ip, const TextChatMsgReq& req, const Json::Value& rtvalue){
    TextChatMsgRsp resp;
    resp.set_error(ErrorCodes::Success);

    resp.set_fromuid(req.fromuid());
    resp.set_touid(req.touid());
    for(const auto& text : req.textmsgs()){
        TextChatData* new_msg = resp.add_textmsgs();
        new_msg->set_msgid(text.msgid());
        new_msg->set_msgcontent(text.msgcontent());
    }

    auto find_iter = _pools.find(server_ip);
    if(find_iter == _pools.end()){
        return resp;
    }

    auto &pool = find_iter->second;
    ClientContext context;
    auto stub = pool->getConnection();
    Status status = stub->NotifyTextChatMsg(&context, req, &resp);
    pool->returnConnection(std::move(stub));

    if(!status.ok()){
        resp.set_error(ErrorCodes::RPCFailed);
        return resp;
    }
    return resp;    
}