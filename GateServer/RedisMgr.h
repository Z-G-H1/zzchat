#include "const.h"
#include "Singleton.h"
#include "hiredis/hiredis.h"

class RedisConPool{
public:
    RedisConPool(size_t poolSize, const std::string& host, int port) 
        : poolSize_(poolSize), b_stop_(false), port_(port), host_(host)
    {
        for(size_t i=0; i<poolSize_; i++){
            std::unique_ptr<redisContext> context(redisConnect(host.c_str(), port));
            if (context == nullptr || context->err != 0) {
				if (context != nullptr) {
					redisFree(context.release());
                    std::cout << "Redis Connect failed!" << std::endl;
				}
				continue;
			}
            connections_.push(std::move(context));
        }
    }

    ~RedisConPool(){
        std::lock_guard<std::mutex> lock(mutex_);
        Close();
        while(!connections_.empty()){
            auto context = std::move(connections_.front());
            redisFree(context.release());
            connections_.pop();
        }
    }
    
    std::unique_ptr<redisContext> getConnection(){
        // 相当于消费者
        std::unique_lock<std::mutex> lock(mutex_);
        // 条件变量
        cond_.wait(lock, [this]{        //阻塞时会自动释放lock锁
            if(b_stop_){
                // 如果已经停止了，直接唤醒该条件变量
                return true;
            }
            // 队列非空，释放
            return !connections_.empty();
        });
        if(b_stop_){
            return nullptr;
        }

        auto context = std::move(connections_.front());
        connections_.pop();
        return context;
    }

    void returnConnection(std::unique_ptr<redisContext> context){
        std::lock_guard<std::mutex> lock(mutex_);
        if(b_stop_){
            return;
        }
        connections_.push(std::move(context));
        // 唤醒
        cond_.notify_one();
        return;
    }

    void Close(){
        b_stop_ = true;
        cond_.notify_all();
    }
private:
    std::atomic<bool> b_stop_;
    size_t poolSize_;
    int port_;
    std::string host_;
    std::queue<std::unique_ptr<redisContext>> connections_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

// 封装hiredis中的操作
class RedisMgr: public Singleton<RedisMgr>{
    friend Singleton<RedisMgr>;
public:
    ~RedisMgr();
    bool Get(const std::string& key, std::string& value);
    bool Set(const std::string &key, const std::string &value);
    bool LPush(const std::string &key, const std::string &value);
    bool LPop(const std::string &key, std::string& value);
    bool RPush(const std::string& key, const std::string& value);
    bool RPop(const std::string& key, std::string& value);
    bool HSet(const std::string &key, const std::string  &hkey, const std::string &value);
    bool HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen);
    std::string HGet(const std::string &key, const std::string &hkey);
    bool Del(const std::string &key);
    bool ExistsKey(const std::string &key);
    void Close();
private:
    RedisMgr();
    
    std::unique_ptr<RedisConPool> _pool;
};
