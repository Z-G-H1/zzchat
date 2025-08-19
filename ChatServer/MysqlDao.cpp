#include "MysqlDao.h"
#include "ConfigMgr.h"

MysqlDao::MysqlDao(){
    auto& cfg = ConfigMgr::Inst();
    const auto& host = cfg["Mysql"]["Host"];
    const auto& port = cfg["Mysql"]["Port"];
    const auto& pwd = cfg["Mysql"]["Passwd"];
    const auto& schema = cfg["Mysql"]["Schema"];
    const auto& user = cfg["Mysql"]["User"];
    pool_.reset(new MysqlPool(5, host+":"+port, user, pwd, schema));
}
MysqlDao::~MysqlDao(){
    pool_->Close();
}

// 注册用户，返回用户id； 如果用户或邮箱已存在，返回0或-1
int MysqlDao::RegUser(const std::string &name, const std::string &email, const std::string &pwd){
    auto con = pool_->getConnection();
    try
    {
        if(con == nullptr){
            pool_->returnConncetion(std::move(con));
            return false;
        }

        // 准备调用存贮过程
        std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement("CALL reg_user(?,?,?,@result)"));


        stmt->setString(1, name);
        stmt->setString(2, email);
        stmt->setString(3, pwd);

        stmt->execute();

        std::unique_ptr<sql::Statement> stmtResult(con->createStatement());
        std::unique_ptr<sql::ResultSet> res(stmtResult->executeQuery("SELECT @result AS result"));

        if(res->next()){
            int result = res->getInt("result");
            std::cout << "Result: " << result << std::endl;
            pool_->returnConncetion(std::move(con));
            return result;
        }else {
            std::cerr << "Failed to get result from stored procedure." << std::endl;
            pool_->returnConncetion(std::move(con));
            return -1;
        }
        pool_->returnConncetion(std::move(con));
        return -1;
    }
    catch (sql::SQLException& e) {
        pool_->returnConncetion(std::move(con));
        std::cerr << "SQLException: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return -1;
    }
    return 0;
}

bool MysqlDao::UpdatePwd(const std::string& name, const std::string& newpwd){
    auto con = pool_->getConnection();
    try
    {
        if(con == nullptr){
            pool_->returnConncetion(std::move(con));
            return false;
        }

        // 准备调用存贮过程
        std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement("UPDATE user SET pwd = ? WHERE name = ?"));

        stmt->setString(1, newpwd);
        stmt->setString(2, name);
        //执行
        int updateCount = stmt->executeUpdate();

        std::cout << "Updated rows: " << updateCount << std::endl;
        pool_->returnConncetion(std::move(con));
        return true;
    }
    catch (sql::SQLException& e) {
        pool_->returnConncetion(std::move(con));
        std::cerr << "SQLException: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}

bool MysqlDao::CheckEmail(const std::string& name, const std::string& email){
    auto con = pool_->getConnection();
    try
    {
        if(con == nullptr){
            pool_->returnConncetion(std::move(con));
            return false;
        }

        // 准备调用存贮过程
        std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement("SELECT email from user where name = ?"));

        stmt->setString(1, name);
        //执行
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

        // 遍历结果集合
        while(res->next()){
            std::cout << "Check Email: " << res->getString("email") << std::endl;
            if(email != res->getString("email")){
                pool_->returnConncetion(std::move(con));
                return false;
            }
            pool_->returnConncetion(std::move(con));
            return true;
        }
    }
    catch (sql::SQLException& e) {
        pool_->returnConncetion(std::move(con));
        std::cerr << "SQLException: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}

bool MysqlDao::CheckPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo){
    auto con = pool_->getConnection();
    try
    {
        if(con == nullptr){
            pool_->returnConncetion(std::move(con));
            return false;
        }

        // 准备调用存贮过程
        std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement("SELECT * from user where name = ?"));

        stmt->setString(1, name);
        //执行
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

        // 遍历结果集合
        while(res->next()){
            // std::cout << "Check Email: " << res->getString("email") << std::endl;
            if(pwd != res->getString("pwd")){
                pool_->returnConncetion(std::move(con));
                return false;
            }
            userInfo.email = res->getString("email");
            userInfo.name = name;
            userInfo.pwd = pwd;
            userInfo.uid = res->getInt("uid");
            break;
        }
        pool_->returnConncetion(std::move(con));
        return true;
    }
    catch (sql::SQLException& e) {
        pool_->returnConncetion(std::move(con));
        std::cerr << "SQLException: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}

std::shared_ptr<UserInfo> MysqlDao::GetUser(int uid){
    auto con = pool_->getConnection();
    try
    {
        if(con == nullptr){
            pool_->returnConncetion(std::move(con));
            return nullptr;
        }

        // 准备调用存贮过程
        std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement("SELECT * from user where uid = ?"));
        stmt->setInt(1, uid);
        //执行
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
        std::shared_ptr<UserInfo> userInfo = nullptr;
        // 遍历结果集合
        while(res->next()){
            // std::cout << "Check Email: " << res->getString("email") << std::endl;
            userInfo.reset(new UserInfo);
            userInfo->email = res->getString("email");
            userInfo->name = res->getString("name");
            userInfo->pwd = res->getString("pwd");
            userInfo->uid = uid;
            userInfo->nick = res->getString("nick");
			userInfo->desc = res->getString("desc");
			userInfo->sex = res->getInt("sex");
            break;
        }
        pool_->returnConncetion(std::move(con));
        return userInfo;
    }
    catch (sql::SQLException& e) {
        pool_->returnConncetion(std::move(con));
        std::cerr << "SQLException: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return nullptr;
    }
}
