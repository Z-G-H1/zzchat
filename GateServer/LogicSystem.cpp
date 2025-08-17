#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VarifyGrpcClient.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"
#include "StatusGrpcClient.h"

LogicSystem::LogicSystem(){
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
            return ;
        }
        auto email = src_root["email"].asString();
        std::cout << "email is " << email << std::endl;
        GetVarifyRsp resp = VarifyGrpcClient::GetInstance()->GetVarifyCode(email);
        root["email"] = src_root["email"];
        root["error"] = resp.error();
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->_response.body()) << jsonstr;
        return ;
    });

    RegPost("/user_register",[](std::shared_ptr<HttpConnection> connection){
        // 获取请求体
        auto body_str = beast::buffers_to_string(connection->_request.body().data());
        std::cout << "receive body is " << body_str << std::endl;
        connection->_response.set(http::field::content_type, "text/json");
        Json::Reader reader;
        Json::Value root;
        Json::Value src_root;

        // 转换数据
        bool parse_success = reader.parse(body_str, src_root);
        if(!parse_success){
            std::cout << "Failed to parse Json data! " << std::endl;
            root["error"] = ErrorCodes::Error_Json;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return ;
        }
        // 提取数据
        auto email = src_root["email"].asString();
        auto name = src_root["user"].asString();
        auto pwd = src_root["passwd"].asString();
        auto confirm = src_root["confirm"].asString();
        
        // 转换成功 先查询redis，看输入的验证码是否合法
        std::string varify_code;
        bool b_get_varify = RedisMgr::GetInstance()->Get(CODEPREFIX+email, varify_code);
        if(!b_get_varify){
            std::cout << "varify code error" << std::endl;
            root["error"] = ErrorCodes::VarifyCodeErr;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return ;
        }
        // 验证码比对
        if (varify_code != src_root["varifycode"].asString()) {
            std::cout << " varify code error" << std::endl;
            root["error"] = ErrorCodes::VarifyCodeErr;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return ;
        }

        // 访问redis，查看用户是否在线（也是排查是否存在用户）
        bool b_user_exist = RedisMgr::GetInstance()->ExistsKey(name);
        if(b_user_exist){
            std::cout << "user already exist" << std::endl;
            root["error"] = ErrorCodes::UserExist;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return ;
        }

        // 查完redis，再检查数据库
        int uid = MysqlMgr::GetInstance()->RegUser(name, email, pwd);
        if(uid == 0 || uid == -1){
            std::cout << "user or email already exist" << std::endl;
            root["error"] = ErrorCodes::UserExist;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return ;
        }

        root["error"] = 0;
        root["email"] = email;
        root["user"]= name;
        root["passwd"] = pwd;
        root["confirm"] = confirm;
        root["varifycode"] = src_root["varifycode"].asString();
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->_response.body()) << jsonstr;
        return ;

    });

    RegPost("/reset_pwd",[](std::shared_ptr<HttpConnection> connection){
        // 获取请求体
        auto body_str = beast::buffers_to_string(connection->_request.body().data());
        std::cout << "receive body is " << body_str << std::endl;
        connection->_response.set(http::field::content_type, "text/json");
        Json::Reader reader;
        Json::Value root;
        Json::Value src_root;

        // 转换数据
        bool parse_success = reader.parse(body_str, src_root);
        if(!parse_success){
            std::cout << "Failed to parse Json data! " << std::endl;
            root["error"] = ErrorCodes::Error_Json;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return ;
        }
        // 提取数据
        auto email = src_root["email"].asString();
        auto name = src_root["user"].asString();
        auto pwd = src_root["passwd"].asString();
        
        // 转换成功 先查询redis，看输入的验证码是否合法
        std::string varify_code;
        bool b_get_varify = RedisMgr::GetInstance()->Get(CODEPREFIX+email, varify_code);
        if(!b_get_varify){
            std::cout << "varify code error" << std::endl;
            root["error"] = ErrorCodes::VarifyCodeErr;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return ;
        }
        // 验证码比对
        if (varify_code != src_root["varifycode"].asString()) {
            std::cout << " varify code error" << std::endl;
            root["error"] = ErrorCodes::VarifyCodeErr;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return ;
        }

        // 检查数据库判断用户名和邮箱是否匹配
        bool check_user_email = MysqlMgr::GetInstance()->CheckEmail(name, email);
        if(!check_user_email){
            std::cout << "user email not match " << std::endl;
            root["error"] = ErrorCodes::EmailNotMatch;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return ;
        }

        root["error"] = 0;
        root["email"] = email;
        root["user"]= name;
        root["passwd"] = pwd;
        root["varifycode"] = src_root["varifycode"].asString();
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->_response.body()) << jsonstr;
        return ;
    });

    RegPost("/user_login",[](std::shared_ptr<HttpConnection> connection){
        // 获取请求体
        auto body_str = beast::buffers_to_string(connection->_request.body().data());
        std::cout << "receive body is " << body_str << std::endl;
        connection->_response.set(http::field::content_type, "text/json");
        Json::Reader reader;
        Json::Value root;
        Json::Value src_root;

        // 转换数据
        bool parse_success = reader.parse(body_str, src_root);
        if(!parse_success){
            std::cout << "Failed to parse Json data! " << std::endl;
            root["error"] = ErrorCodes::Error_Json;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return ;
        }
        // 提取数据 获取用户名和密码
        auto name = src_root["user"].asString();
        auto pwd = src_root["passwd"].asString();
        UserInfo userinfo;
        
        // 检查数据库判断用户名和邮箱是否匹配
        bool check_pwd = MysqlMgr::GetInstance()->CheckPwd(name, pwd,userinfo);
        if(!check_pwd){
            std::cout << "user pwd not match " << std::endl;
            root["error"] = ErrorCodes::EmailNotMatch;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return ;
        }

        // 从状态服务器获取token
        auto reply = StatusGrpcClient::GetInstance()->GetChatServer(userinfo.uid);
        if(reply.error() != 0){
            // 有错误
            std::cout << " grpc get chat server failed, error is " << reply.error()<< std::endl;
            root["error"] = ErrorCodes::RPCFailed;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return ;
        }
        std::cout << "succeed to load userinfo uid is " << userinfo.uid << std::endl;
        root["error"] = 0;
        root["user"] = name;
        root["uid"] = userinfo.uid;
        root["token"] = reply.token();
        root["host"] = reply.host();
        root["port"] = reply.port();
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->_response.body()) << jsonstr;
        return ;
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
