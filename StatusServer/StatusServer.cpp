#include <grpcpp/server_builder.h>

#include "const.h"
#include "ConfigMgr.h"
#include "hiredis/hiredis.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"
#include "AsioIOServicePool.h"
#include "StatusServiceImpl.h"


void RunServer(){
    auto &cfg = ConfigMgr::Inst();
    
    std::string server_address(cfg["StatusServer"]["Host"]+":"+ cfg["StatusServer"]["Port"]);
    StatusServiceImpl service;

    grpc::ServerBuilder builder;
    // 监听端口
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    // 构建并启动grpc服务器
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    // 创建iocontext
    boost::asio::io_context io_context;
    // 创建signal set 用于捕获SIGINT
    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);

    // 设置异步等待 SIGINT信号
    signals.async_wait([&server](const boost::system::error_code& error, int signal_number){
        if(!error){
            std::cout << "Shutting down server..." << std::endl;
            server->Shutdown();
        }
    });
    // 在单独的线程中运行io_context
    std::thread([&io_context]() { io_context.run(); }).detach();

    // 等待服务器关闭
    server->Wait();
    io_context.stop(); // 停止io_context
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