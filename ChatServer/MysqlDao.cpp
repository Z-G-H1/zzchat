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

std::shared_ptr<UserInfo> MysqlDao::GetUser(std::string name){
    auto con = pool_->getConnection();
    try
    {
        if(con == nullptr){
            pool_->returnConncetion(std::move(con));
            return nullptr;
        }

        // 准备调用存贮过程
        std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement("SELECT * from user where name = ?"));
        stmt->setString(1, name);
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
            userInfo->uid = res->getInt("uid");
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

bool MysqlDao::AddFriendApply(const int fromuid,const int touid){
    auto con = pool_->getConnection();
    try
    {
        if(con == nullptr){
            pool_->returnConncetion(std::move(con));
            return false;
        }
        
        // 准备调用存贮过程
        std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement("INSERT INTO friend_apply (from_uid, to_uid) values (?,?)"
                "ON DUPLICATE KEY UPDATE from_uid = from_uid, to_uid = to_uid"));
        stmt->setInt(1, fromuid);
        stmt->setInt(2, touid);
        //执行
        int rowsAffected = stmt->executeUpdate();
        if(rowsAffected < 0){
            return false;
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

bool MysqlDao::GetApplyList(int touid, std::vector<std::shared_ptr<ApplyInfo>>& applyList, int begin, int limit){
    auto con = pool_->getConnection();
    try
    {
        if(con == nullptr){
            pool_->returnConncetion(std::move(con));
            return false;
        }

        // 准备调用存贮过程
        std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement("select apply.from_uid, apply.status, user.name, "
                "user.nick, user.sex from friend_apply as apply join user on apply.from_uid = user.uid where apply.to_uid = ? "
            "and apply.id > ? order by apply.id ASC LIMIT ? "));
        stmt->setInt(1, touid);
        stmt->setInt(2, begin);
        stmt->setInt(3, limit);
        //执行
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
        // 遍历结果集合
        while(res->next()){
            auto name = res->getString("name");
            auto uid = res->getInt("from_uid");
            auto status = res->getInt("status");
            auto nick = res->getString("nick");
            auto sex = res->getInt("sex");
            auto apply_ptr = std::make_shared<ApplyInfo>(uid, name, "", "", nick, sex, status);
            applyList.push_back(apply_ptr);
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

bool MysqlDao::AuthFriendApply(const int fromuid, const int touid){
    auto con = pool_->getConnection();
    try{
        if(con == nullptr){
            pool_->returnConncetion(std::move(con));
            return false;
        }
        // 准备执行语句
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("UPDATE friend_apply SET status = 1 "
			"WHERE from_uid = ? AND to_uid = ?"));
        pstmt->setInt(1, touid);
        pstmt->setInt(2, fromuid);
        
        // 执行更新
        int rowAffected = pstmt->executeUpdate();
        if(rowAffected < 0){
            return false;
        }
        return true;
    }
    catch(sql::SQLException& e){
        pool_->returnConncetion(std::move(con));
        std::cerr << "SQLException: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
    
}

bool MysqlDao::AddFriend(const int fromuid, const int touid, const std::string& bakname){
    auto con = pool_->getConnection();
    if(con == nullptr){
        pool_->returnConncetion(std::move(con));
        return false;
    }
    // 要执行多个语句，不自动提交。
    con->setAutoCommit(false);
    try{
        // 准备执行语句
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("INSERT IGNORE INTO friend(self_id, friend_id, back) "
			"VALUES (?, ?, ?) "));
		pstmt->setInt(1, fromuid); // from id
		pstmt->setInt(2, touid);
		pstmt->setString(3, bakname);
        
        // 执行更新
        int rowAffected = pstmt->executeUpdate();
        if(rowAffected < 0){
            con->rollback();
            return false;
        }

        // 准备执行第二个语句
        std::unique_ptr<sql::PreparedStatement> pstmt2(con->prepareStatement("INSERT IGNORE INTO friend(self_id, friend_id, back) "
			"VALUES (?, ?, ?) "
		));
		//反过来的申请时from，验证时to
		pstmt2->setInt(1, touid); // from id
		pstmt2->setInt(2, fromuid);
		pstmt2->setString(3, "");
		// 执行更新
		int rowAffected2 = pstmt2->executeUpdate();
		if (rowAffected2 < 0) {
			con->rollback();
			return false;
		}

		// 提交事务
		con->commit();
		std::cout << "addfriend insert friends success" << std::endl;


        return true;
    }
    catch(sql::SQLException& e){
        // 如果发生错误，回滚事务
		if (con) {
			con->rollback();
		}
        pool_->returnConncetion(std::move(con));
        std::cerr << "SQLException: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}

bool MysqlDao::GetFriendList(const int uid, std::vector<UserInfo> &list){
    auto con = pool_->getConnection();
    if(con == nullptr){
        pool_->returnConncetion(std::move(con));
        return false;
    }
    // 要执行多个语句，不自动提交。
    con->setAutoCommit(false);
    try{
        // 准备执行语句
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("INSERT IGNORE INTO friend(self_id, friend_id, back) "
			"VALUES (?, ?, ?) "));
		pstmt->setInt(1, fromuid); // from id
		pstmt->setInt(2, touid);
		pstmt->setString(3, bakname);
        
        // 执行更新
        int rowAffected = pstmt->executeUpdate();
        if(rowAffected < 0){
            con->rollback();
            return false;
        }

        // 准备执行第二个语句
        std::unique_ptr<sql::PreparedStatement> pstmt2(con->prepareStatement("INSERT IGNORE INTO friend(self_id, friend_id, back) "
			"VALUES (?, ?, ?) "
		));
		//反过来的申请时from，验证时to
		pstmt2->setInt(1, touid); // from id
		pstmt2->setInt(2, fromuid);
		pstmt2->setString(3, "");
		// 执行更新
		int rowAffected2 = pstmt2->executeUpdate();
		if (rowAffected2 < 0) {
			con->rollback();
			return false;
		}

		// 提交事务
		con->commit();
		std::cout << "addfriend insert friends success" << std::endl;


        return true;
    }
    catch(sql::SQLException& e){
        // 如果发生错误，回滚事务
		if (con) {
			con->rollback();
		}
        pool_->returnConncetion(std::move(con));
        std::cerr << "SQLException: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}