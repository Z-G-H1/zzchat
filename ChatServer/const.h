#pragma once
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <memory>
#include <iostream>
#include <functional>
#include <map>
#include <unordered_map>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/reader.h>
#include <atomic>
#include <queue>
#include <boost/uuid/uuid.hpp>  
#include <boost/uuid/uuid_generators.hpp>  
#include <boost/uuid/uuid_io.hpp> 

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
#define CODEPREFIX  "code_"


enum ErrorCodes {
    Success = 0,
	Error_Json = 1001,  //Json解析错误
	RPCFailed = 1002,  //RPC请求错误
	VarifyExpired = 1003, //验证码过期
	VarifyCodeErr = 1004, //验证码错误
	UserExist = 1005,       //用户已经存在
	PasswdErr = 1006,    //密码错误
	EmailNotMatch = 1007,  //邮箱不匹配
	PasswdUpFailed = 1008,  //更新密码失败
	PasswdInvalid = 1009,   //密码更新失败
	TokenInvalid = 1010,   //Token失效
	UidInvalid = 1011,  //uid无效
};


#define MAX_LENGTH  1024*2
//头部总长度
#define HEAD_TOTAL_LEN 4
//头部id长度
#define HEAD_ID_LEN 2
//头部数据长度
#define HEAD_DATA_LEN 2
#define MAX_RECVQUE  10000
#define MAX_SENDQUE 1000

struct UserInfo {
	std::string name;
	std::string pwd;
	int uid;
	std::string email;
};

enum MSG_IDS {
	MSG_CHAT_LOGIN = 1005, //用户登陆
	MSG_CHAT_LOGIN_RSP = 1006, //用户登陆回包
};