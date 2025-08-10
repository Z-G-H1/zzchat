#include "CServer.h"
#include "ConfigMgr.h"
#include "AsioIOServicePool.h"
int main()
{
    try
    {
        auto &gCfgMgr = ConfigMgr::Inst();
        // 这里用到了重构的 [] 运算符。
        std::string gate_port_url = gCfgMgr["GateServer"]["Port"];
        unsigned short gate_port = atoi(gate_port_url.c_str());

        net::io_context ioc{ 1 };
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const boost::system::error_code& error, int signal_number) {

            if (error) {
                return;
            }
            ioc.stop();
            });
        std::make_shared<CServer>(ioc, gate_port)->Start();
        ioc.run();
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
