#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VarifyGrpcClient.h"

LogicSystem::LogicSystem(){
    RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection){
        beast::ostream(connection->_response.body()) << "receive get_test req";
        int i=0;
        for (auto& elem : connection->_get_params) {
            i++;
            beast::ostream(connection->_response.body()) << "param" << i << " key is " << elem.first;
            beast::ostream(connection->_response.body()) << ", " <<  " value is " << elem.second << std::endl;
        }
    });

    RegPost("/get_varifycode", [](std::shared_ptr<HttpConnection> connection){
        // 获取请求体
        auto body_str = beast::buffers_to_string(connection->_request.body().data());
        std::cout << "receive body is " << body_str << std::endl;
        connection->_response.set(http::field::content_type, "text/json");
        Json::Reader reader;
        Json::Value root;
        Json::Value src_root;
        bool parse_success = reader.parse(body_str, src_root);
        if(!parse_success){
            std::cout << "Failed to parse JSON data!" << std::endl;
            root["error"] = ErrorCodes::Error_Json;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return true;
        }
        auto email = src_root["email"].asString();
        std::cout << "email is " << email << std::endl;
        GetVarifyRsp resp = VarifyGrpcClient::GetInstance()->GetVarifyCode(email);
        root["email"] = src_root["email"];
        root["error"] = resp.error();
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->_response.body()) << jsonstr;
        return true;
    });
}

LogicSystem::~LogicSystem(){

}

bool LogicSystem::HandleGet(std::string url, std::shared_ptr<HttpConnection> connection){
    if(_get_handler.find(url) == _get_handler.end()){
        // 没有对应的处理函数
        return false;
    }
    _get_handler[url](connection);
    return true;
}

bool LogicSystem::HandlePost(std::string url, std::shared_ptr<HttpConnection> connection){
    if(_post_handler.find(url) == _post_handler.end()){
        // 没有对应的处理函数
        return false;
    }
    _post_handler[url](connection);
    return true;
}


void LogicSystem::RegGet(std::string url, HttpHandler handler){
    _get_handler.insert(std::make_pair(url, handler));
}

void LogicSystem::RegPost(std::string url, HttpHandler handler){
    _post_handler.insert(std::make_pair(url, handler));
}
