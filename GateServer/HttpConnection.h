#pragma once
#include "const.h"

class HttpConnection : public std::enable_shared_from_this<HttpConnection>
{
    friend class LogicSystem;
public:
    HttpConnection(tcp::socket socket);
    tcp::socket& GetSocket(){ return _socket; }
    void Start();

private:
    void CheckDeadline();
    void WriteResponse();
    void HandleReq();
    void PreParseGetParam();

    tcp::socket _socket;
    beast::flat_buffer _buffer{8192};   //flat_buffer 可以重复使用,不需要频繁的分配和释放内存
    http::request<http::dynamic_body> _request;     // 用来解析请求   http::dynamic_body动态缓冲区实现 当你不知道请求主体的大小或类型时，比较灵活
    http::response<http::dynamic_body> _response;    // 用来回应客户端
    net::steady_timer deadline_{
        _socket.get_executor(), std::chrono::seconds(60) };

    std::string _get_url;
    std::unordered_map<std::string, std::string> _get_params;
};

