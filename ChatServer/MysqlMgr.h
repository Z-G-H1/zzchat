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
    bool CheckPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo);
    std::shared_ptr<UserInfo> GetUser(int uid);
    std::shared_ptr<UserInfo> GetUser(std::string name);
    bool AddFriendApply(const int fromuid, const int touid);
    bool GetApplyList(int touid, std::vector<std::shared_ptr<ApplyInfo>>& applyList, int begin, int limit);
    bool AuthFriendApply(const int fromuid, const int touid);
    bool AddFriend(const int fromuid, const int touid, const std::string& bakname);
    bool GetFriendList(const int uid, std::vector<std::shared_ptr<UserInfo>> &list);
private:
    MysqlMgr();
    MysqlDao _dao;
};