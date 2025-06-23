#include "CServer.h"
#include "AsioIOServicePool.h"
CServer::CServer(boost::asio::io_context& io_context, short port): _io_context(io_context), _port(port),
_acceptor(io_context, tcp::endpoint(tcp::v4(), port)) {
	// 初始化 _acceptor 对象，它负责监听并接受新的连接
	std::cout << "Server started success, listen on port : " << _port << std::endl;
	StartAccept();
}

CServer::~CServer() {
	cout << "Server destruct listen on port : " << _port << endl;
}

/**
 * @brief 清除会话的函数
 * @param uuid 
 */
void CServer::ClearSession(std::string uuid)
{
	lock_guard<mutex> lock(_mutex);
	_sessions.erase(uuid);
}

/**
 * @brief 处理接受新连接的异步操作完成后的回调函数
 * @param new_session 刚接受的客户端连接对应的 CSession 智能指针
 * @param error Boost.Asio 操作结果的错误码
 */
void CServer::HandleAccept(shared_ptr<CSession> new_session, const boost::system::error_code& error)
{
	if (!error) {
		// 如果没有错误，说明连接成功建立，启动新会话的数据收发操作
		new_session->Start();
		// 使用 std::lock_guard 锁定互斥量，确保线程安全，将新会话添加到会话管理列表
		lock_guard<mutex> lock(_mutex);
		_sessions.insert(make_pair(new_session->GetUuid(), new_session));
	}
	else {
		cout << "session accept failed , error is " << error.what() << endl;
	}
	// 无论接受成功与否，都继续启动下一次接受新连接的异步操作，以便服务器能持续接受连接
	StartAccept();
}

// 启动一个异步操作，等待并接受一个新的客户端连接
void CServer::StartAccept()
{
	// 从 AsioIOServicePool 中获取一个 io_context 实例
	auto& io_context = AsioIOServicePool::GetInstance()->GetIOService();
	// 创建一个新的 CSession 对象来处理即将到来的连接      将其包装在 shared_ptr 中，确保在异步操作完成前不会被销毁
	shared_ptr<CSession> new_session = make_shared<CSession>(io_context, this);
	// 启动一个异步接受操作
	// 当有新的客户端连接到来时，_acceptor 会将其接受到 new_session 的套接字中
	// 接受完成后，会调用 CServer::HandleAccept 方法来处理
	// std::bind 用于将 CServer::HandleAccept 成员函数绑定到当前 CServer 对象，并预留一个占位符给错误码
	_acceptor.async_accept(new_session->GetSocket(), std::bind(&CServer::HandleAccept, this, new_session, placeholders::_1));
}

