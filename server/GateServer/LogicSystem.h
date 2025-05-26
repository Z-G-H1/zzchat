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

	// ����·�ɺͻص�������Ϊ����
	void RegGet(std::string, HttpHandler handler);
private:
	LogicSystem();
	// post�����get����� �ص�����map, keyΪ·�ɣ� valΪ�ص�����
	std::map<std::string, HttpHandler> _post_handlers;
	std::map <std::string, HttpHandler> _get_handlers;
};

