#include "CServer.h"
#include "HttpConnection.h"
#include "AsioIOServicePool.h"

CServer::CServer(net::io_context &ioc, unsigned short port)
    : _ioc(ioc), _acceptor(ioc,tcp::endpoint(tcp::v4(),port))
{
    std::cout << "Server start success, listen on port : " << _port << std::endl;
    StartAccept();
}

void CServer::StartAccept(){
    auto &io_context = AsioIOServicePool::GetInstance()->GetIoService();
    std::shared_ptr<CSession> new_session = std::make_shared<CSession>(io_context, this);
    _acceptor.async_accept(new_session->GetSocket(), [this,new_session](const boost::system::error_code& error){
        HandleAccept(new_session, error);
    });
}

void CServer::HandleAccept(std::shared_ptr<CSession> new_session, const boost::system::error_code & error){
    if(!error){
        new_session->Start();
        std::lock_guard<std::mutex> lock(_mutex);
        _sessions.insert(std::make_pair(new_session->GetUuid(), new_session));
    }else{
        std::cout << "session accept failed , error is " << error.what() << std::endl;
    }
    StartAccept();
}

CServer::~CServer() {
	std::cout << "Server destruct listen on port : " << _port << std::endl;
}


void CServer::ClearSession(std::string uuid)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_sessions.erase(uuid);
}