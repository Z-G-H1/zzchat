#pragma once
#include <mutex>
#include <memory>

template <typename T>
class Singleton{
protected:
    Singleton() = default;
    //拷贝构造函数必须使用引用作为参数，否则会引发一个无限递归的问题。
    Singleton(Singleton<T>&) = delete;
    // 赋值运算符（operator=）通常返回一个对当前对象本身的引用。这样做是为了支持链式赋值 
    Singleton& operator=(const Singleton<T>&) = delete;     // Singleton& 表示返回值 

    static std::shared_ptr<T> _instance;
public:
    static std::shared_ptr<T> GetInstance(){
        static std::once_flag flag;
        std::call_once(flag, [&](){
            _instance = std::shared_ptr<T>(new T);
            // 没有传入参数，只适合无参构造函数
        });

        return _instance;
    }    

    ~Singleton() {
        std::cout << "this is singleton destruct" << std::endl;
    }
};

// 静态成员变量的类外定义
// 1.分离声明和定义  声明不分配内存 定义时分配内存
// 2.因为静态成员变量只存在一份，它不属于任何一个具体的类对象，所以它的内存空间必须在全局范围内被分配。
template <typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;
