#pragma once
#include "const.h"
#include "Singleton.h"

class HttpConnection;

typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;
class LogicSystem : public Singleton<LogicSystem>
{
    friend class Singleton<LogicSystem>;
public:
    ~LogicSystem();
    bool HandleGet(std::string, std::shared_ptr<HttpConnection>);
    void RegGet(std::string, HttpHandler handler);
    bool HandlePost(std::string, std::shared_ptr<HttpConnection>);
    void RegPost(std::string, HttpHandler handler);
private:
    LogicSystem();
    // key 是路由， value 是处理函数
    std::map<std::string, HttpHandler> _post_handler;
    std::map<std::string, HttpHandler> _get_handler;
};