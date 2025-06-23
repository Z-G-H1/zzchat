#pragma once
#include "const.h"
#include "data.h"
#include "CSession.h"

typedef  function<void(shared_ptr<CSession>, const short& msg_id, const string& msg_data)> FunCallBack;

class LogicSystem :public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;

public:
	~LogicSystem();
	void PostMsgToQue(shared_ptr < LogicNode> msg);
private:
	LogicSystem();
	void DealMsg();
	void RegisterCallBacks();
	void LoginHandler(shared_ptr<CSession>, const short& msg_id, const string& msg_data);


	std::thread _worker_thread;
	std::queue<shared_ptr<LogicNode>> _msg_que;
	std::mutex _mutex;
	std::condition_variable _consume;
	bool _b_stop;
	std::map<short, FunCallBack> _fun_callbacks;
	std::unordered_map<int, std::shared_ptr<UserInfo>> _users;

};

