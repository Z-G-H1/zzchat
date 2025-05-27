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
	void PreParseGetParam();

	tcp::socket _socket;
	// 处理读数据的buffer
	beast::flat_buffer _buffer{ 8192 };

	// request请求信息
	http::request<http::dynamic_body> _request;

	// response message
	http::response<http::dynamic_body> _response;

	// 给连接设置一个超时
	net::steady_timer deadline_{
		_socket.get_executor(), std::chrono::seconds(60) };

	std::string _get_url;
	std::unordered_map<std::string, std::string> _get_params;

};