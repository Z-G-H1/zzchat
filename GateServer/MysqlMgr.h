#include "const.h"
#include "Singleton.h"
#include "MysqlDao.h"

class MysqlMgr : public Singleton<MysqlMgr>{
    friend Singleton<MysqlMgr>;
public:
    ~MysqlMgr();
    int RegUser(const std::string& name, const std::string& email, const std::string& pwd);
    bool UpdatePwd(const std::string& name, const std::string& newpwd);
    bool CheckEmail(const std::string& name, const std::string& email);
private:
    MysqlMgr();
    MysqlDao _dao;
};