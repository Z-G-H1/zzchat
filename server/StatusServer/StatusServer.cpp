#include <iostream>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "const.h"
#include "ConfigMgr.h"
#include "hiredis.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"
#include "AsioIOServicePool.h"
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <boost/asio.hpp>
#include "StatusServiceImpl.h"

void RunServer() {
	auto& cfg = ConfigMgr::Inst();
	std::string server_address(cfg["StatusServer"]["Host"] + ":" + cfg["StatusServer"]["Port"]);
	StatusServiceImpl service;

	grpc::ServerBuilder builder;
	// 监听端口添加服务
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);

	// 构建并启动grpc服务器
	std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

	if (server == nullptr) {
		std::cerr << "Server failed to start on " << server_address
			<< ". Is the port already in use?" << std::endl;
		return; // 或者 throw std::runtime_error("Server failed to start");
	}

	std::cout << "Server listening on " << server_address << std::endl;

	// 创建boost.asio的io_context
	boost::asio::io_context io_context;
	// 创建signal_set用于捕获SIGINT
	boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);

	// 设置异步等待SIGINT信号
	signals.async_wait([&server](const boost::system::error_code& error, int signal_number) {
		if (!error) {
			std::cout << "shutting down server..." << std::endl;
			server->Shutdown(); // 关闭grpc服务器
		}
	});
	// 在单独的线程中运行io_context
	std::thread([&io_context]() { io_context.run(); }).detach();

	// 等待服务器关闭
	server->Wait();
	//io_context.stop(); // 停止io_context
}

int main(int argc, char** argv) {
	try {
		RunServer();
	}
	catch (std::exception const& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return 0;
}