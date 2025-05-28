#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VerifyGrpcClient.h"

bool LogicSystem::HandleGet(std::string path, std::shared_ptr<HttpConnection> con)
{
	if (_get_handlers.find(path) == _get_handlers.end()) {
		return false;
	}

	_get_handlers[path](con);
	return true;
}

bool LogicSystem::HandlePost(std::string path, std::shared_ptr<HttpConnection> con)
{
	if (_post_handlers.find(path) == _post_handlers.end()) {
		return false;
	}
	_post_handlers[path](con);
	return true;
}

void LogicSystem::RegGet(std::string url, HttpHandler handler)
{
	// 接受路由和回调函数作为参数
	_get_handlers.insert(make_pair(url, handler));
}

void LogicSystem::RegPost(std::string url, HttpHandler handler)
{
	_post_handlers.insert(make_pair(url, handler));
}

LogicSystem::LogicSystem()
{
	RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
		beast::ostream(connection->_response.body()) << "receive get_test req " << std::endl;
		int i = 0;
		for (auto &elem : connection->_get_params) {
			i++;
			beast::ostream(connection->_response.body()) << "param" << i << "key is " << elem.first;
			beast::ostream(connection->_response.body()) << ", " << "value is " << elem.second << std::endl;
		}
	});

	RegPost("/get_varifycode", [](std::shared_ptr<HttpConnection> connection) {
		// 获取请求体
		auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
		connection->_response.set(http::field::content_type, "text/json");
		std::cout << "receive body is " << body_str << std::endl;
		Json::Value root;       // 用于构建我们将要发送的响应 JSON。
		Json::Reader reader;    // 用于解析接收到的 JSON 字符串。
		Json::Value src_root;   // 用于存储从请求体中解析出来的 JSON 数据。
		bool parse_success = reader.parse(body_str, src_root);

		if (!parse_success) {
			std::cout << "Failed to parse Json data! " << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		if( !src_root.isMember("email")) {
			std::cout << "Failed to parse JSON data! " << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}
		auto email = src_root["email"].asString();

		GetVerifyRsp resp = VerifyGrpcClient::GetInstance()->GetVarifyCode(email);

		std::cout << "email is " << email << std::endl;
		root["error"] = resp.error();
		root["email"] = src_root["email"];
		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->_response.body()) << jsonstr;
		return true;
	});
}