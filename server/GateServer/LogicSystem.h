#pragma once
#include "const.h"

class HttpConnection;
typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;
class LogicSystem : public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;
public:
	~LogicSystem(){}
	bool HandleGet(std::string, std::shared_ptr<HttpConnection>);

	// 接受路由和回调函数作为参数
	void RegGet(std::string, HttpHandler handler);
private:
	LogicSystem();
	// post请求和get请求的 回调函数map, key为路由， val为回调函数
	std::map<std::string, HttpHandler> _post_handlers;
	std::map <std::string, HttpHandler> _get_handlers;
};

