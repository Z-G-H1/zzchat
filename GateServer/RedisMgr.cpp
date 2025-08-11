#include "RedisMgr.h"
#include "ConfigMgr.h"

RedisMgr::RedisMgr(){
    auto cfg_mgr = ConfigMgr::Inst();
    auto host = cfg_mgr["Redis"]["Host"];
    auto port = cfg_mgr["Redis"]["Port"];
    auto pwd = cfg_mgr["Redis"]["Passwd"];
    _pool.reset(new RedisConPool(5,host, atoi(port.c_str()), pwd.c_str()));
}

RedisMgr::~RedisMgr(){
    Close();
}



bool RedisMgr::Get(const std::string &key, std::string &value){
    auto connect = _pool->getConnection();
    if(connect == nullptr) {
		std::cout << "Failed to get Redis connection from pool." << std::endl;
		return false;
	}
    auto reply = (redisReply*)redisCommand(connect,"GET %s", key.c_str());
    // 获取失败
    if(reply == NULL){
        std::cout << "[ GET  " << key << " ] failed" << std::endl;
        // 释放reply
        _pool->returnConnection(connect);
        freeReplyObject(reply);
        return false;
    }
    if (reply->type != REDIS_REPLY_STRING) {
        std::cout << "[ GET  " << key << " ] failed" << std::endl;
        _pool->returnConnection(connect);
        freeReplyObject(reply);
        return false;
    }

    value = reply->str;
    _pool->returnConnection(connect);
    freeReplyObject(reply);

    std::cout << "Succeed to execute command [ GET " << key << "  ]" << std::endl;
    return true;
}

bool RedisMgr::Set(const std::string &key, const std::string &value){
    auto connect = _pool->getConnection();
    if(connect == nullptr) {
		std::cout << "Failed to get Redis connection from pool." << std::endl;
		return false;
	}
    auto reply = (redisReply*)redisCommand(connect, "SET %s %s", key.c_str(), value.c_str());
    std::cout << reply->str << std::endl;
    // 如果返回NULL说明执行失败
    if(NULL == reply){
        std::cout << "Execute command [ SET " << key << " " << value << " ] failure ! " << std::endl;
        _pool->returnConnection(connect);
        return false;
    }
    // 如果执行失败则释放连接
    if(!(reply->type == REDIS_REPLY_STATUS && (strcmp(reply->str, "OK") == 0 || strcmp(reply->str, "ok") == 0))){
        std::cout << "Execute command [ SET " << key << " " << value << " ] failure ! " << std::endl;
        _pool->returnConnection(connect);
        freeReplyObject(reply);

        return false;
    }

    // 执行成功 释放redisCommand返回的reply 占用的内存
    _pool->returnConnection(connect);
    freeReplyObject(reply);
    std::cout << "Execute command [ SET " << key << " " << value << " ] Success ! " << std::endl;
    return true;
}


bool RedisMgr::LPush(const std::string &key, const std::string &value)
{
    auto connect = _pool->getConnection();
    if(connect == nullptr) {
		std::cout << "Failed to get Redis connection from pool." << std::endl;
		return false;
	}
    auto reply = (redisReply*)redisCommand(connect, "LPUSH %s %s", key.c_str(), value.c_str());
    if (NULL == reply)
    {
        std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        _pool->returnConnection(connect);
        freeReplyObject(reply);
        return false;
    }

    if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
        std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        _pool->returnConnection(connect);
        freeReplyObject(reply);
        return false;
    }

    std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] success ! " << std::endl;
    _pool->returnConnection(connect);
    freeReplyObject(reply);
    return true;
}

bool RedisMgr::LPop(const std::string &key, std::string& value){
    auto connect = _pool->getConnection();
    if(connect == nullptr) {
		std::cout << "Failed to get Redis connection from pool." << std::endl;
		return false;
	}
    auto reply = (redisReply*)redisCommand(connect, "LPOP %s ", key.c_str());
    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
        std::cout << "Execut command [ LPOP " << key<<  " ] failure ! " << std::endl;
        _pool->returnConnection(connect);
        freeReplyObject(reply);
        return false;
    }
    value = reply->str;
    std::cout << "Execut command [ LPOP " << key <<  " ] success ! " << std::endl;
    _pool->returnConnection(connect);
    freeReplyObject(reply);
    return true;
}

bool RedisMgr::RPush(const std::string& key, const std::string& value) {
    auto connect = _pool->getConnection();
    if(connect == nullptr) {
		std::cout << "Failed to get Redis connection from pool." << std::endl;
		return false;
	}
    auto reply = (redisReply*)redisCommand(connect, "RPUSH %s %s", key.c_str(), value.c_str());
    if (NULL == reply)
    {
        std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        _pool->returnConnection(connect);
        freeReplyObject(reply);
        return false;
    }

    if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
        std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        _pool->returnConnection(connect);
        freeReplyObject(reply);
        return false;
    }

    std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] success ! " << std::endl;
    _pool->returnConnection(connect);
    freeReplyObject(reply);
    return true;
}


bool RedisMgr::RPop(const std::string& key, std::string& value) {
    auto connect = _pool->getConnection();
    if(connect == nullptr) {
		std::cout << "Failed to get Redis connection from pool." << std::endl;
		return false;
	}
    auto reply = (redisReply*)redisCommand(connect, "RPOP %s ", key.c_str());
    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
        std::cout << "Execut command [ RPOP " << key << " ] failure ! " << std::endl;
        _pool->returnConnection(connect);
        freeReplyObject(reply);
        return false;
    }
    value = reply->str;
    std::cout << "Execut command [ RPOP " << key << " ] success ! " << std::endl;
    _pool->returnConnection(connect);
    freeReplyObject(reply);
    return true;
}


bool RedisMgr::HSet(const std::string &key, const std::string &hkey, const std::string &value) {
    auto connect = _pool->getConnection();
    if(connect == nullptr) {
		std::cout << "Failed to get Redis connection from pool." << std::endl;
		return false;
	}
    auto reply = (redisReply*)redisCommand(connect, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER ) {
        std::cout << "Execut command [ HSet " << key << "  " << hkey <<"  " << value << " ] failure ! " << std::endl;
        _pool->returnConnection(connect);
        freeReplyObject(reply);
        return false;
    }
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << value << " ] success ! " << std::endl;
    _pool->returnConnection(connect);
    freeReplyObject(reply);
    return true;
}


bool RedisMgr::HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen)
{
    const char* argv[4];
    size_t argvlen[4];
    argv[0] = "HSET";
    argvlen[0] = 4;
    argv[1] = key;
    argvlen[1] = strlen(key);
    argv[2] = hkey;
    argvlen[2] = strlen(hkey);
    argv[3] = hvalue;
    argvlen[3] = hvaluelen;
    auto connect = _pool->getConnection();
    if(connect == nullptr) {
		std::cout << "Failed to get Redis connection from pool." << std::endl;
		return false;
	}
    auto reply = (redisReply*)redisCommandArgv(connect, 4, argv, argvlen);
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
        std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << hvalue << " ] failure ! " << std::endl;
        _pool->returnConnection(connect);
        freeReplyObject(reply);
        return false;
    }
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << hvalue << " ] success ! " << std::endl;
    _pool->returnConnection(connect);
    freeReplyObject(reply);
    return true;
}


std::string RedisMgr::HGet(const std::string &key, const std::string &hkey)
{
    const char* argv[3];
    size_t argvlen[3];
    argv[0] = "HGET";
    argvlen[0] = 4;
    argv[1] = key.c_str();
    argvlen[1] = key.length();
    argv[2] = hkey.c_str();
    argvlen[2] = hkey.length();
    auto connect = _pool->getConnection();
    if(connect == nullptr) {
		std::cout << "Failed to get Redis connection from pool." << std::endl;
		return "";
	}
    auto reply = (redisReply*)redisCommandArgv(connect, 3, argv, argvlen);
    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
        freeReplyObject(reply);
        _pool->returnConnection(connect);
        std::cout << "Execut command [ HGet " << key << " "<< hkey <<"  ] failure ! " << std::endl;
        return "";
    }

    std::string value = reply->str;
    freeReplyObject(reply);
    _pool->returnConnection(connect);
    std::cout << "Execut command [ HGet " << key << " " << hkey << " ] success ! " << std::endl;
    return value;
}

bool RedisMgr::Del(const std::string &key)
{
    auto connect = _pool->getConnection();
    if(connect == nullptr) {
		std::cout << "Failed to get Redis connection from pool." << std::endl;
		return false;
	}
    auto reply = (redisReply*)redisCommand(connect, "DEL %s", key.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
        std::cout << "Execut command [ Del " << key <<  " ] failure ! " << std::endl;
        freeReplyObject(reply);
        _pool->returnConnection(connect);
        return false;
    }
    std::cout << "Execut command [ Del " << key << " ] success ! " << std::endl;
     freeReplyObject(reply);
     _pool->returnConnection(connect);
     return true;
}

bool RedisMgr::ExistsKey(const std::string &key)
{
    auto connect = _pool->getConnection();
    if(connect == nullptr) {
		std::cout << "Failed to get Redis connection from pool." << std::endl;
		return false;
	}
    auto reply = (redisReply*)redisCommand(connect, "exists %s", key.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER || reply->integer == 0) {
        std::cout << "Not Found [ Key " << key << " ]  ! " << std::endl;
        freeReplyObject(reply);
        _pool->returnConnection(connect);
        return false;
    }
    std::cout << " Found [ Key " << key << " ] exists ! " << std::endl;
    freeReplyObject(reply);
    _pool->returnConnection(connect);
    return true;
}

void RedisMgr::Close()
{
    _pool->Close();
}

