#pragma once
#include "const.h"

class HttpConnection : public std::enable_shared_from_this<HttpConnection>
{
	friend class LogicSystem;
public:
	HttpConnection(tcp::socket socket);
	void Start();
private:
	void CheckDeadline();
	void WriteResponse();
	void HandleReq();

	tcp::socket _socket;
	// ��������ݵ�buffer
	beast::flat_buffer _buffer{ 8192 };

	// request������Ϣ
	http::request<http::dynamic_body> _request;

	// response message
	http::response<http::dynamic_body> _response;

	// ����������һ����ʱ
	net::steady_timer deadline_{
		_socket.get_executor(), std::chrono::seconds(60) };
};