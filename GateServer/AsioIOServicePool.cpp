#include "AsioIOServicePool.h"

// 创建n个线程， 每个线程跑一个iocontext
AsioIOServicePool::AsioIOServicePool(std::size_t size):_ioService(size),_works(size), _nextIOService(0) {
    // _ioService 初始化过了， 调用IOService的默认构造函数，进行了构造，size大小
    for(size_t i=0; i<size; i++){
        _works[i] = std::unique_ptr<Work>(new Work(_ioService[i]));
    }

    for(size_t i=0; i<size; i++){
        _threads.emplace_back([this, i]() {
            _ioService[i].run();
        });
    }

}

//获取一个iocontext
boost::asio::io_context& AsioIOServicePool::GetIoService(){
    auto& service = _ioService[_nextIOService++];
    if(_nextIOService == _ioService.size()){
        _nextIOService = 0;
    }
    return service;
}

void AsioIOServicePool::Stop(){
    //work.reset()是让unique指针置空并释放s
    for(auto & work : _works){
        // 先停止服务
        work->get_io_context().stop();
        work.reset();
    }

    for(auto &thread : _threads){
        thread.join();
    }
}


AsioIOServicePool::~AsioIOServicePool(){
    Stop();
    std::cout << "AsioIOServicePool destruct" << std::endl;
}