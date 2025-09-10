#include "MysqlMgr.h"

MysqlMgr::MysqlMgr(){

}

MysqlMgr::~MysqlMgr(){
	std::cout << "Destruct mysql" << std::endl;

}

int MysqlMgr::RegUser(const std::string &name, const std::string &email, const std::string &pwd){
    return _dao.RegUser(name, email, pwd);
}

bool MysqlMgr::UpdatePwd(const std::string& name, const std::string& newpwd){
    return _dao.UpdatePwd(name, newpwd);
}

bool MysqlMgr::CheckEmail(const std::string& name, const std::string& email){
    return _dao.CheckEmail(name, email);
}

bool MysqlMgr::CheckPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo){
    return _dao.CheckPwd(name, pwd, userInfo);
}

std::shared_ptr<UserInfo> MysqlMgr::GetUser(int uid){
    return _dao.GetUser(uid);
}

std::shared_ptr<UserInfo> MysqlMgr::GetUser(std::string name){
    return _dao.GetUser(name);
}

bool MysqlMgr::AddFriendApply(const int fromuid,const int touid){
    return _dao.AddFriendApply(fromuid, touid);
}

bool MysqlMgr::GetApplyList(int touid, std::vector<std::shared_ptr<ApplyInfo>>& applyList, int begin, int limit){
    return _dao.GetApplyList(touid, applyList, begin, limit);
}

bool MysqlMgr::AuthFriendApply(const int fromuid, const int touid){
    return _dao.AuthFriendApply(fromuid, touid);
}

bool MysqlMgr::AddFriend(const int fromuid, const int touid, const std::string& bakname){
    return _dao.AddFriend(fromuid, touid, bakname);
}

bool MysqlMgr::GetFriendList(const int uid, std::vector<std::shared_ptr<UserInfo>> &list){
    return _dao.GetFriendList(uid, list);
}