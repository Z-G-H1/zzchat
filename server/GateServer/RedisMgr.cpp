#include "RedisMgr.h"

RedisMgr::RedisMgr() : _connect(nullptr), _reply(nullptr){

}
RedisMgr::~RedisMgr()
{
	_connect = nullptr;
	_reply = nullptr;
}
// ���Ӳ���
bool RedisMgr::Connect(const std::string& host, int port)
{
	this->_connect = redisConnect(host.c_str(), port);
	if (this->_connect != NULL && this->_connect->err) {
		std::cout << "redis connect error " << this->_connect->errstr << std::endl;
		return false;
	}
	return true;
}

// ��ȡkey��Ӧ��value
bool RedisMgr::Get(const std::string& key, std::string& value)
{
	this->_reply = (redisReply*)redisCommand(this->_connect, "GET %s", key.c_str());
	if (this->_reply == NULL) {
		std::cout << "[ GET " << key << "] failed" << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}
	if (this->_reply->type != REDIS_REPLY_STRING) {
		std::cout << "[ GET " << key << " ] failed" << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}

	value = this->_reply->str;
	freeReplyObject(this->_reply);

	std::cout << "Succeed to execute command [ GET " << key << " ]" << std::endl;
	return true;
}

// ���ü���ֵ
bool RedisMgr::Set(const std::string& key, const std::string& value)
{
	// ִ��redis������
	this->_reply = (redisReply*)redisCommand(this->_connect, "SET %s %s", key.c_str(), value.c_str());

	// �������NULL˵��ִ��ʧ��
	if (NULL == this->_reply) {
		std::cout << "Execute command [ SET " << key << " " << value << "	 ] failed !" << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}
	// ���ִ��ʧ�� �ͷ�����
	if (!(this->_reply->type == REDIS_REPLY_STATUS && (strcmp(this->_reply->str, "OK") == 0 || strcmp(this->_reply->str, "ok") == 0))) {
		std::cout << "Execute command [ SET " << key << " " << value << " ] failed !" << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}
	// ִ�гɹ� �ͷ�redisCommand���ص�replyռ�õ��ڴ�
	freeReplyObject(this->_reply);
	std::cout << "Execute command [ SET " << key << " " << value << " ] success !" << std::endl;
	return true;
}

// ������֤
bool RedisMgr::Auth(const std::string& password)
{
	this->_reply = (redisReply*)redisCommand(this->_connect, "AUTH %s", password.c_str());
	if (this->_reply->type == REDIS_REPLY_ERROR) {
		std::cout << "redis ��֤ʧ��" << std::endl;
		// ִ��ʧ�� �ͷ�redisCommand ִ�к󷵻ص�redisReply ռ�õ��ڴ�
		freeReplyObject(this->_reply);
		return false;
	}
	else {
		// ִ�гɹ� �ͷ�redisCommand ִ�к󷵻ص�redisReply ռ�õ��ڴ�
		freeReplyObject(this->_reply);
		std::cout << "redis ��֤�ɹ�" << std::endl;
		return true;
	}
	return true;
}

// ���PUSH
bool RedisMgr::LPush(const std::string& key, const std::string& value)
{
	this->_reply = (redisReply*)redisCommand(this->_connect, "LPUSH %s %s", key.c_str(), value.c_str());
	if (NULL == this->_reply) {
		std::cout << "Excute command [ LPUSH " << key << " " << value << " ] failed !" << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}
	if(this->_reply->type != REDIS_REPLY_INTEGER || this->_reply->integer <= 0) {
		std::cout << "Excute command [ LPUSH " << key << " " << value << " ] failed !" << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}
	std::cout << "Excute command [ LPUSH " << key << " " << value << " ] success !" << std::endl;
	freeReplyObject(this->_reply);
	return true;
}

// ���POP
bool RedisMgr::LPop(const std::string& key, std::string& value)
{
	this->_reply = (redisReply*)redisCommand(this->_connect, "LPOP %s", key.c_str());
	if (this->_reply == NULL || this->_reply->type == REDIS_REPLY_NIL) {
		std::cout << "Excute command [ LPOP " << key << " ] failed !" << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}

	value = this->_reply->str;
	freeReplyObject(this->_reply);
	std::cout << "Excute command [ LPOP " << key << " ] success !" << std::endl;
	return true;
}

// �Ҳ�PUSH
bool RedisMgr::RPush(const std::string& key, const std::string& value)
{
	this->_reply = (redisReply*)redisCommand(this->_connect, "RPUSH %s %s", key.c_str(), value.c_str());
	if (NULL == this->_reply) {
		std::cout << "Execute command [ RPUSH " << key << " " << value << " ] failed !" << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}

	if (this->_reply->type != REDIS_REPLY_INTEGER || this->_reply->integer <= 0) {
		std::cout << "Excute command [ RPUSH " << key << " " << value << " ] failed !" << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}

	std::cout << "Excute command [ RPUSH " << key << " " << value << " ] success !" << std::endl;
	freeReplyObject(this->_reply);
	return true;
}

// �Ҳ�POP
bool RedisMgr::RPop(const std::string& key, std::string& value)
{
	this->_reply = (redisReply*)redisCommand(this->_connect, "RPOP %s", key.c_str());
	if (this->_reply == NULL || this->_reply->type == REDIS_REPLY_NIL) {
		std::cout << "Excute command [ RPOP " << key << " ] failed !" << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}
	value = this->_reply->str;
	freeReplyObject(this->_reply);
	std::cout << "Excute command [ RPOP " << key << " ] success !" << std::endl;
	return true;
}

bool RedisMgr::HSet(const std::string& key, const std::string& hkey, const std::string& value)
{
	this->_reply = (redisReply*)redisCommand(this->_connect, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
	if (this->_reply == NULL || this->_reply->type != REDIS_REPLY_INTEGER) {
		std::cout << "Execute command [ HSET " << key << " " << hkey << " " << value << " ] failed !" << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}
	std::cout << "Execute command [ HSET " << key << " " << hkey << " " << value << " ] success !" << std::endl;
	freeReplyObject(this->_reply);
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
	this->_reply = (redisReply*)redisCommandArgv(this->_connect, 4, argv, argvlen);
	if (this->_reply == NULL || this->_reply->type != REDIS_REPLY_INTEGER) {
		std::cout << "Execute command [ HSET " << key << " " << hkey << " " << hvalue << " ] failed !" << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}
	std::cout << "Execute command [ HSET " << key << " " << hkey << " " << hvalue << " ] success !" << std::endl;
	freeReplyObject(this->_reply);
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
	this->_reply = (redisReply*)redisCommandArgv(this->_connect, 3, argv, argvlen);
	if (this->_reply == NULL || this->_reply->type == REDIS_REPLY_NIL) {
		freeReplyObject(this->_reply);
		std::cout << "Execute command [ HGET " << key << " " << hkey << " ] failed !" << std::endl;
		return "";
	}

	std::string value = this->_reply->str;
	freeReplyObject(this->_reply);
	std::cout << "Execute command [ HGET " << key << " " << hkey << " ] success !" << std::endl;
	return value;
}

bool RedisMgr::Del(const std::string& key)
{
	this->_reply = (redisReply*)redisCommand(this->_connect, "DEL %s", key.c_str());
	if (this->_reply == NULL || this->_reply->type != REDIS_REPLY_INTEGER) {
		std::cout << "Execute command [ DEL " << key << " ] failed !" << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}
	std::cout << "Execute command [ DEL " << key << " ] success !" << std::endl;
	freeReplyObject(this->_reply);
	return true;
}

// �жϼ�ֵ�Ƿ����
bool RedisMgr::ExistsKey(const std::string& key)
{
	this->_reply = (redisReply*)redisCommand(this->_connect, "EXISTS %s", key.c_str());
	if (this->_reply == NULL || this->_reply->type != REDIS_REPLY_INTEGER || this->_reply->integer == 0) {
		std::cout << "Not Found [ Key " << key << " ]  ! " << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}
	std::cout << "Found [ Key " << key << " ]  exists! " << std::endl;
	freeReplyObject(this->_reply);
	return true;
}

void RedisMgr::Close() {
	redisFree(_connect);
}
