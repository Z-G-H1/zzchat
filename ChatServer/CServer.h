#pragma once
#include "const.h"
#include "CSession.h"

class CServer : public std::enable_shared_from_this<CServer>
{    
public:
    CServer(net::io_context &ioc, unsigned short port);
    void ClearSession(std::string uuid);
    ~CServer();
private:
    void HandleAccept(std::shared_ptr<CSession>, const boost::system::error_code & error);
    void StartAccept();

    net::io_context& _ioc;       // 上下文iocontext服务
    short _port;
    tcp::acceptor _acceptor;    // 接收器
    std::mutex _mutex;
    std::map<std::string, std::shared_ptr<CSession>> _sessions;
};

