#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VarifyGrpcClient.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"
#include "message.grpc.pb.h"
#include "StatusGrpcClient.h"

LogicSystem::LogicSystem(): _b_stop(false){
    RegisterCallBackS();
    _worker_thread = std::thread(&LogicSystem::DealMsg, this);
}

void LogicSystem::RegisterCallBackS(){
    _fun_callbacks[MSG_CHAT_LOGIN] = [this](std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data){
        LoginHandler(session, msg_id, msg_data);
    };
}

LogicSystem::~LogicSystem(){
    _b_stop = true;
    _users.clear();
    _consume.notify_one();
    _worker_thread.join();
}

void LogicSystem::PostMsgToQue(std::shared_ptr<LogicNode> msg){
    std::lock_guard<std::mutex> lock(_mutex);
    if(_b_stop){
        return ;
    }
    _msg_que.push(msg);
    // 由0变为1时 notify， 因为队列非空会自动处理
    if(_msg_que.size() == 1)
        _consume.notify_one();
}

void LogicSystem::DealMsg(){
    while(1){
        std::unique_lock<std::mutex> lock(_mutex);
        while(_msg_que.empty() && !_b_stop){
            _consume.wait(lock);
        }
        // 停服， 将消息队列中数据处理完，退出
        if(_b_stop){
            while(!_msg_que.empty()){
                auto msg_node = _msg_que.front();
                _msg_que.pop();
		        std::cout << "msg is is " << msg_node->_recv_node->msg_id << std::endl;
                auto iter = _fun_callbacks.find(msg_node->_recv_node->msg_id);
                if(iter == _fun_callbacks.end()){
                    // 没有对应的回调函数
                    std::cout << "msg id [" << msg_node->_recv_node->msg_id << "] handler not found" << std::endl;
                    return;
                }
                // 调用回调函数
                iter->second(msg_node->_session, msg_node->_recv_node->msg_id, 
                    std::string(msg_node->_recv_node->_data, msg_node->_recv_node->_total_len));
            }
            return; 
        }
        // 没有停服
        auto msg_node = _msg_que.front();
        _msg_que.pop();
		std::cout << "msg is is " << msg_node->_recv_node->msg_id << std::endl;
        auto iter = _fun_callbacks.find(msg_node->_recv_node->msg_id);
        if(iter == _fun_callbacks.end()){
            // 没有对应的回调函数
            std::cout << "msg id [" << msg_node->_recv_node->msg_id << "] handler not found" << std::endl;
            return;
        }
        iter->second(msg_node->_session, msg_node->_recv_node->msg_id, 
            std::string(msg_node->_recv_node->_data, msg_node->_recv_node->_total_len));
    }
}

void LogicSystem::LoginHandler(std::shared_ptr<CSession> session, const short &msg_id, const std::string &msg_data){
    Json::Reader reader;
    Json::Value root;
    reader.parse(msg_data, root);
    int uid = root["uid"].asInt();
    std::cout << "user login uid is  " << root["uid"].asInt() << " user token  is "
		<< root["token"].asString() << std::endl;

    // 从状态服务器获取token 看是否匹配  在statusServer中进行比对
    auto resp = StatusGrpcClient::GetInstance()->Login(uid, root["token"].asString());
    Json::Value rtvalue;
    rtvalue["error"] = resp.error();
    if (resp.error() != ErrorCodes::Success) {
        std::string jsonstr = rtvalue.toStyledString();
        session->Send(jsonstr, MSG_CHAT_LOGIN_RSP);
        return;
    }

    // 内存中查询用户信息
    auto find_iter = _users.find(uid);
    std::shared_ptr<UserInfo> user_info = nullptr;
    if(find_iter == _users.end()){
        // 查询数据库
        user_info = MysqlMgr::GetInstance()->GetUser(uid);
        if(user_info == nullptr){
            rtvalue["error"] = ErrorCodes::UidInvalid;
            return;
        }
        // 添加到内存
        _users[uid] = user_info;

    }else{
        user_info = find_iter->second;
    }

    rtvalue["uid"] = uid;
    rtvalue["name"] = user_info->name;
    rtvalue["token"] = resp.token();
    std::string jsonstr = rtvalue.toStyledString();
    session->Send(jsonstr, msg_id);
}
