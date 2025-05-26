#ifndef SINGLETON_H
#define SINGLETON_H
#include <memory>
#include <mutex>
#include <iostream>
using namespace std;

template <typename T>
class Singleton {
protected:
    Singleton() = default;
    Singleton(const Singleton<T>& ) = delete;   // 禁用拷贝构造函数
    Singleton& operator=(const Singleton<T>& st) = delete;  //

    static std::shared_ptr<T> _instance;
public:
    static std::shared_ptr<T> GetInstance() {
        //这个创建过程是“懒汉式”的 (Lazy Initialization)，即只在第一次调用 GetInstance() 时才创建实例。
        // 确保多线程下只有一个实例被创建，主要是实现线程安全。
        static std::once_flag s_flag;
        std::call_once(s_flag,[&](){
            _instance = std::shared_ptr<T>(new T );
        });
        return _instance;
    }

    void PrintAddress() {
        std::cout << _instance.get() << std::endl;
    }

    ~Singleton() {
        std::cout << "this is singleton destruct " << std::endl;
    }
};

template <typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;

#endif // SINGLETON_H
