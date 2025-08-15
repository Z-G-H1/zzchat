#pragma once
#include "Singleton.h"
#include "const.h"

class AsioIOServicePool : public Singleton<AsioIOServicePool>{
    friend Singleton<AsioIOServicePool>;
public:
    using IOService = boost::asio::io_context;
    // iocontext的run方法会在所有任务完成后停止，使用 executor_work_guard 来“占位”，让iocontext不退出
    //当 executor_work_guard 被销毁时，io_context 可以正常退出（如果没有其他任务）
    using Work = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
    using WorkPtr = std::unique_ptr<Work>;

    ~AsioIOServicePool();
    AsioIOServicePool(const AsioIOServicePool&) = delete;
    AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;

    boost::asio::io_context& GetIoService();
    void Stop();
private:
    AsioIOServicePool(std::size_t size = 2);

    std::vector<IOService> _ioService;
    std::vector<WorkPtr> _works;
    std::vector<std::thread> _threads;
    std::size_t _nextIOService;
};