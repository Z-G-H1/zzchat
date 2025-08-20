#include "const.h"
#include <mysql-cppconn/jdbc/mysql_driver.h>
#include <mysql-cppconn/jdbc/mysql_connection.h>
#include <mysql-cppconn/jdbc/cppconn/prepared_statement.h>
#include <mysql-cppconn/jdbc/cppconn/resultset.h>
#include <mysql-cppconn/jdbc/cppconn/statement.h>
#include <mysql-cppconn/jdbc/cppconn/exception.h>


class MysqlPool{
public:
    MysqlPool(size_t poolsize, const std::string& url,const std::string& user, const std::string& pass,const std::string& schema)
        : poolSize_(poolsize), url_(url), user_(user), pass_(pass), schema_(schema)
    {
        for(size_t i=0; i<poolSize_; i++){
            sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();
            std::unique_ptr<sql::Connection> con(driver->connect(url_,user_,pass_));
            con->setSchema(schema_);
            connections_.push(std::move(con));
        }
    }

    ~MysqlPool(){
        std::lock_guard<std::mutex> lock(mutex_);
        while(!connections_.empty()){
            connections_.pop();
        }
    }

    std::unique_ptr<sql::Connection> getConnection(){
        std::unique_lock<std::mutex> lock(mutex_);

        _cond.wait(lock,[this]{
            if(b_stop_){
                return true;
            }
            return !connections_.empty();
        });

        if(b_stop_){
            return nullptr;
        }
        auto con = std::move(connections_.front());
        connections_.pop();
        return con;
    }

    void returnConncetion(std::unique_ptr<sql::Connection> con){
        std::lock_guard<std::mutex> lock(mutex_);
        if(b_stop_){
            return;
        }
        connections_.push(std::move(con));
        _cond.notify_one();
        return;
    }

    void Close(){
        b_stop_ = true;
        _cond.notify_all();
    }
private:
    std::atomic<bool> b_stop_;
    size_t poolSize_;
    std::queue<std::unique_ptr<sql::Connection>> connections_;
    std::mutex mutex_;
    std::condition_variable _cond;
    std::string url_;
    std::string user_;
    std::string pass_;
    std::string schema_;
};


class MysqlDao{
public:
    MysqlDao();
    ~MysqlDao();
    int RegUser(const std::string& name, const std::string& email, const std::string& pwd);
    bool UpdatePwd(const std::string& name, const std::string& newpwd);
    bool CheckEmail(const std::string& name, const std::string& email);
    bool CheckPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo);
    std::shared_ptr<UserInfo> GetUser(int uid);
    std::shared_ptr<UserInfo> GetUser(std::string name);
    bool AddFriendApply(const int fromuid,const int touid);

private:
    std::unique_ptr<MysqlPool> pool_;
};