#include "CServer.h"
#include "HttpConnection.h"

CServer::CServer(net::io_context &ioc, unsigned short &port)
    : _ioc(ioc), _acceptor(ioc,tcp::endpoint(tcp::v4(),port)), _socket(ioc)
{

}


void CServer::Start(){
    auto self = shared_from_this();
    // 在内部创建HttpConnection智能指针，将socket转交给HttpConnection
    _acceptor.async_accept(_socket, [self](boost::beast::error_code ec){
        try{
            if(ec){
                // 处理错误, 放弃这个连接，启动新的监听
                self->Start();
                return;
            }
            // 创建httpConnection指针 管理socket
            std::make_shared<HttpConnection>(std::move(self->_socket))->Start();
            // 继续监听
            self->Start();
        }
        catch(const std::exception& e){
            std::cerr << e.what() << '\n';
        }
        
    });
}