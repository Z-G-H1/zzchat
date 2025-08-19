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
    AddFriendReq resp;
    return resp;
}

AuthFriendRsp ChatGrpcClient::NotifyAuthFriend(std::string server_ip, const AuthFriendReq& req){
    AuthFriendRsp resp;
    return resp;
}
bool ChatGrpcClient::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo){
    return true;
}
TextChatMsgRsp ChatGrpcClient::NotifyTextChatMsg(std::string server_ip, const TextChatMsgReq& req, const Json::Value& rtvalue){
    TextChatMsgRsp resp;
    return resp;    
}