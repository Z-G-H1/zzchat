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
        : poolSize_(poolsize), url_(url), user_(user), pass_(pass), schema_(schema), b_stop_(false)
    {
    //     std::cout << "[INFO] Initializing MySQL connection pool for schema '" << schema_ << "' with target size " << poolSize_ << "..." << std::endl;
    // try {
    //     // 1. 获取驱动实例（在循环外获取一次即可）
    //     sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    //     for (size_t i = 0; i < poolSize_; ++i) {
    //         std::cout << "[INFO] Creating connection " << (i + 1) << "/" << poolSize_ << "..." << std::endl;
    //         // 2. 建立连接
    //         std::unique_ptr<sql::Connection> con(driver->connect(url_, user_, pass_));
    //         if (!con || !con->isValid()) {
    //             // 如果因为某些原因connect返回了空指针或无效连接，则主动抛出异常
    //             throw std::runtime_error("Failed to create a valid MySQL connection object.");
    //         }
    //         // 3. 设置数据库 schema
    //         con->setSchema(schema_);
    //         // 4. 将成功的连接放入池中
    //         connections_.push(std::move(con));
    //     }
    //     std::cout << "[SUCCESS] MySQL connection pool initialized successfully with " << connections_.size() << " connections." << std::endl;

    // } catch (const sql::SQLException& e) {
    //     // 捕获所有 MySQL 相关的异常
    //     std::cerr << "[FATAL] Failed to initialize MySQL connection pool due to an SQL error." << std::endl;
    //     std::cerr << "  Error Code: " << e.getErrorCode() << std::endl;
    //     std::cerr << "  SQLState: " << e.getSQLState() << std::endl;
    //     std::cerr << "  Message: " << e.what() << std::endl;
    //     throw; 
    // } catch (const std::runtime_error& e) {
    //     // 捕获其他运行时错误
    //     std::cerr << "[FATAL] A runtime error occurred during pool initialization: " << e.what() << std::endl;
    //     throw;
    // }
        for(size_t i=0; i<poolSize_; i++){
            sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
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
    bool GetApplyList(int touid, std::vector<std::shared_ptr<ApplyInfo>>& applyList, int begin, int limit);
    bool AuthFriendApply(const int fromuid, const int touid);
    bool AddFriend(const int fromuid, const int touid, const std::string& bakname);
    bool GetFriendList(const int uid, std::vector<std::shared_ptr<UserInfo>> &list);

private:
    std::unique_ptr<MysqlPool> pool_;
};