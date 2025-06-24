#include "RedisMgr.h"
#include "ConfigMgr.h"

RedisMgr::RedisMgr(){
	auto& gCfgMgr = ConfigMgr::Inst();
	auto host = gCfgMgr["Redis"]["Host"];
	auto port = gCfgMgr["Redis"]["Port"];
	auto pwd = gCfgMgr["Redis"]["Passwd"];
	_con_pool.reset(new RedisConPool(5, host.c_str(), atoi(port.c_str()), pwd.c_str()));
}
RedisMgr::~RedisMgr()
{
	Close();
}

// 获取key对应的value
bool RedisMgr::Get(const std::string& key, std::string& value)
{
	auto connect = _con_pool->getConnection();
	if(connect == nullptr) {
		std::cout << "Failed to get Redis connection from pool." << std::endl;
		return false;
	}
	auto reply = (redisReply*)redisCommand(connect, "GET %s", key.c_str());
	if (reply == NULL) {
		std::cout << "[ GET " << key << "] failed" << std::endl;
		//freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}
	if (reply->type != REDIS_REPLY_STRING) {
		std::cout << "[ GET " << key << " ] failed" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}

	value = reply->str;
	freeReplyObject(reply);
	std::cout << "Succeed to execute command [ GET " << key << " ]" << std::endl;
	_con_pool->returnConnection(connect);
	return true;
}

// 设置键和值
bool RedisMgr::Set(const std::string& key, const std::string& value)
{
	auto connect = _con_pool->getConnection();
	if (connect == nullptr) {
		std::cout << "Failed to get Redis connection from pool." << std::endl;
		return false;
	}
	// 执行redis命令行
	auto reply = (redisReply*)redisCommand(connect, "SET %s %s", key.c_str(), value.c_str());

	// 如果返回NULL说明执行失败
	if (NULL == reply) {
		std::cout << "Execute command [ SET " << key << " " << value << "	 ] failed !" << std::endl;
		//freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}
	// 如果执行失败 释放连接
	if (!(reply->type == REDIS_REPLY_STATUS && (strcmp(reply->str, "OK") == 0 || strcmp(reply->str, "ok") == 0))) {
		std::cout << "Execute command [ SET " << key << " " << value << " ] failed !" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}
	// 执行成功 释放redisCommand返回的reply占用的内存
	freeReplyObject(reply);
	std::cout << "Execute command [ SET " << key << " " << value << " ] success !" << std::endl;
	_con_pool->returnConnection(connect);
	return true;
}

// 密码认证
//bool RedisMgr::Auth(const std::string& password)
//{
//	reply = (redisReply*)redisCommand(connect, "AUTH %s", password.c_str());
//	if (reply->type == REDIS_REPLY_ERROR) {
//		std::cout << "redis 认证失败" << std::endl;
//		// 执行失败 释放redisCommand 执行后返回的redisReply 占用的内存
//		freeReplyObject(reply);
//		return false;
//	}
//	else {
//		// 执行成功 释放redisCommand 执行后返回的redisReply 占用的内存
//		freeReplyObject(reply);
//		std::cout << "redis 认证成功" << std::endl;
//		return true;
//	}
//	return true;
//}

// 左侧PUSH
bool RedisMgr::LPush(const std::string& key, const std::string& value)
{
	auto connect = _con_pool->getConnection();
	if (connect == nullptr) {
		std::cout << "Failed to get Redis connection from pool." << std::endl;
		return false;
	}
	auto reply = (redisReply*)redisCommand(connect, "LPUSH %s %s", key.c_str(), value.c_str());
	if (NULL == reply) {
		std::cout << "Excute command [ LPUSH " << key << " " << value << " ] failed !" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}
	if(reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
		std::cout << "Excute command [ LPUSH " << key << " " << value << " ] failed !" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}
	std::cout << "Excute command [ LPUSH " << key << " " << value << " ] success !" << std::endl;
	freeReplyObject(reply);
	_con_pool->returnConnection(connect);
	return true;
}

// 左侧POP
bool RedisMgr::LPop(const std::string& key, std::string& value)
{
	auto connect = _con_pool->getConnection();
	if (connect == nullptr) {
		std::cout << "Failed to get Redis connection from pool." << std::endl;
		return false;
	}
	auto reply = (redisReply*)redisCommand(connect, "LPOP %s", key.c_str());
	if (reply == NULL || reply->type == REDIS_REPLY_NIL) {
		std::cout << "Excute command [ LPOP " << key << " ] failed !" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}

	value = reply->str;
	freeReplyObject(reply);
	_con_pool->returnConnection(connect);
	std::cout << "Excute command [ LPOP " << key << " ] success !" << std::endl;
	return true;
}

// 右侧PUSH
bool RedisMgr::RPush(const std::string& key, const std::string& value)
{
	auto connect = _con_pool->getConnection();
	if (connect == nullptr) {
		std::cout << "Failed to get Redis connection from pool." << std::endl;
		return false;
	}
	auto reply = (redisReply*)redisCommand(connect, "RPUSH %s %s", key.c_str(), value.c_str());
	if (NULL == reply) {
		std::cout << "Execute command [ RPUSH " << key << " " << value << " ] failed !" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}

	if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
		std::cout << "Excute command [ RPUSH " << key << " " << value << " ] failed !" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}

	std::cout << "Excute command [ RPUSH " << key << " " << value << " ] success !" << std::endl;
	freeReplyObject(reply);
	_con_pool->returnConnection(connect);
	return true;
}

// 右侧POP
bool RedisMgr::RPop(const std::string& key, std::string& value)
{
	auto connect = _con_pool->getConnection();
	if (connect == nullptr) {
		std::cout << "Failed to get Redis connection from pool." << std::endl;
		return false;
	}
	auto reply = (redisReply*)redisCommand(connect, "RPOP %s", key.c_str());
	if (reply == NULL || reply->type == REDIS_REPLY_NIL) {
		std::cout << "Excute command [ RPOP " << key << " ] failed !" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}
	value = reply->str;
	freeReplyObject(reply);
	_con_pool->returnConnection(connect);
	std::cout << "Excute command [ RPOP " << key << " ] success !" << std::endl;
	return true;
}

bool RedisMgr::HSet(const std::string& key, const std::string& hkey, const std::string& value)
{
	auto connect = _con_pool->getConnection();
	if (connect == nullptr) {
		std::cout << "Failed to get Redis connection from pool." << std::endl;
		return false;
	}
	auto reply = (redisReply*)redisCommand(connect, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
	if (reply == NULL || reply->type != REDIS_REPLY_INTEGER) {
		std::cout << "Execute command [ HSET " << key << " " << hkey << " " << value << " ] failed !" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}
	std::cout << "Execute command [ HSET " << key << " " << hkey << " " << value << " ] success !" << std::endl;
	freeReplyObject(reply);
	_con_pool->returnConnection(connect);
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

	auto connect = _con_pool->getConnection();
	if (connect == nullptr) {
		std::cout << "Failed to get Redis connection from pool." << std::endl;
		return false;
	}
	auto reply = (redisReply*)redisCommandArgv(connect, 4, argv, argvlen);
	if (reply == NULL || reply->type != REDIS_REPLY_INTEGER) {
		std::cout << "Execute command [ HSET " << key << " " << hkey << " " << hvalue << " ] failed !" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}
	std::cout << "Execute command [ HSET " << key << " " << hkey << " " << hvalue << " ] success !" << std::endl;
	freeReplyObject(reply);
	_con_pool->returnConnection(connect);
	return true;
}

std::string RedisMgr::HGet(const std::string& key, const std::string& hkey)
{
	const char* argv[3];
	size_t argvlen[3];
	argv[0] = "HGET";
	argvlen[0] = 4;
	argv[1] = key.c_str();
	argvlen[1] = key.length();
	argv[2] = hkey.c_str();
	argvlen[2] = hkey.length();
	auto connect = _con_pool->getConnection();
	if (connect == nullptr) {
		std::cout << "Failed to get Redis connection from pool." << std::endl;
		return "";
	}
	auto reply = (redisReply*)redisCommandArgv(connect, 3, argv, argvlen);
	if (reply == NULL || reply->type == REDIS_REPLY_NIL) {
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		std::cout << "Execute command [ HGET " << key << " " << hkey << " ] failed !" << std::endl;
		return "";
	}

	std::string value = reply->str;
	freeReplyObject(reply);
	_con_pool->returnConnection(connect);
	std::cout << "Execute command [ HGET " << key << " " << hkey << " ] success !" << std::endl;
	return value;
}

bool RedisMgr::HDel(const std::string& key, const std::string& field)
{
	auto connect = _con_pool->getConnection();
	if (connect == nullptr) {
		return false;
	}

	Defer defer([&connect, this]() {
		_con_pool->returnConnection(connect);
		});

	redisReply* reply = (redisReply*)redisCommand(connect, "HDEL %s %s", key.c_str(), field.c_str());
	if (reply == nullptr) {
		std::cerr << "HDEL command failed" << std::endl;
		return false;
	}

	bool success = false;
	if (reply->type == REDIS_REPLY_INTEGER) {
		success = reply->integer > 0;
	}

	freeReplyObject(reply);
	return success;
}

bool RedisMgr::Del(const std::string& key)
{
	auto connect = _con_pool->getConnection();
	if (connect == nullptr) {
		std::cout << "Failed to get Redis connection from pool." << std::endl;
		return false;
	}
	auto reply = (redisReply*)redisCommand(connect, "DEL %s", key.c_str());
	if (reply == NULL || reply->type != REDIS_REPLY_INTEGER) {
		std::cout << "Execute command [ DEL " << key << " ] failed !" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}
	std::cout << "Execute command [ DEL " << key << " ] success !" << std::endl;
	freeReplyObject(reply);
	_con_pool->returnConnection(connect);
	return true;
}

// 判断键值是否存在
bool RedisMgr::ExistsKey(const std::string& key)
{
	auto connect = _con_pool->getConnection();
	if (connect == nullptr) {
		std::cout << "Failed to get Redis connection from pool." << std::endl;
		return false;
	}
	auto reply = (redisReply*)redisCommand(connect, "EXISTS %s", key.c_str());
	if (reply == NULL || reply->type != REDIS_REPLY_INTEGER || reply->integer == 0) {
		std::cout << "Not Found [ Key " << key << " ]  ! " << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}
	std::cout << "Found [ Key " << key << " ]  exists! " << std::endl;
	freeReplyObject(reply);
	_con_pool->returnConnection(connect);
	return true;
}

void RedisMgr::Close() {
	_con_pool->Close();
}
