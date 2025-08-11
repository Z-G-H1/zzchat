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
            std::cout << "11111111111111111111111111" << std::endl;
            return false;
        }

        // 准备调用存贮过程
        std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement("CALL reg_user(?,?,?,@result)"));
        // 设置输入参数
            std::cout << "Setting parameters: " << name << ", " << email << ", " << pwd << std::endl;

        stmt->setString(1, name);
        stmt->setString(2, email);
        stmt->setString(3, pwd);
        //执行
            std::cout << "Executing stored procedure..." << std::endl;

        stmt->execute();
            std::cout << "Querying for result..." << std::endl;

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
