#include "CServer.h"
#include "HttpConnection.h"
#include "AsioIOServicePool.h"

CServer::CServer(net::io_context &ioc, unsigned short port)
    : _ioc(ioc), _acceptor(ioc,tcp::endpoint(tcp::v4(),port))
{

}


void CServer::Start(){
    auto self = shared_from_this();
    auto& io_context =  AsioIOServicePool::GetInstance()->GetIoService();
    std::shared_ptr<HttpConnection> new_con = std::make_shared<HttpConnection>(io_context);
    // 在内部创建HttpConnection智能指针，将socket转交给HttpConnection
    _acceptor.async_accept(new_con->GetSocket(), [self,new_con](boost::beast::error_code ec){
        try{
            if(ec){
                // 处理错误, 放弃这个连接，启动新的监听
                self->Start();
                return;
            }
            // 创建httpConnection指针 管理socket
            new_con->Start();
            // 继续监听
            self->Start();
        }
        catch(const std::exception& e){
            std::cerr << e.what() << '\n';
        }
        
    });
}