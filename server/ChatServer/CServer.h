#pragma once
#include <boost/asio.hpp>
#include "CSession.h"
#include <memory.h>
#include <map>
#include <mutex>
using namespace std;
using boost::asio::ip::tcp;

class CServer
{
public:
	// 构造函数中监听对方连接
	CServer(boost::asio::io_context& io_context, short port);
	~CServer();
	void ClearSession(std::string);

private:
	void HandleAccept(shared_ptr<CSession> new_session, const boost::system::error_code& error);
	void StartAccept();

	boost::asio::io_context &_io_context;
	short _port;
	tcp::acceptor _acceptor;
	std::map<std::string, shared_ptr<CSession>> _sessions;
	std::mutex _mutex;
};