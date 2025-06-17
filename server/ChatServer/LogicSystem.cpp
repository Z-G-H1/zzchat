#include "LogicSystem.h"

void LogicSystem::RegisterCallBacks()
{
	_fun_callbacks[MSG_CHAT_LOGIN] = std::bind(&LogicSystem::LoginHandler, this, 
		placeholders::_1, placeholders::_2, placeholders::_3);
}

void LogicSystem::LoginHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data)
{
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);

	std::cout << "user login uid is " << root["uid"].asInt() << " user token is"
		<< root["token"].asString() << std::endl;

	std::string return_str = root.toStyledString();
	session->Send(return_str, msg_id);
}

LogicSystem::~LogicSystem()
{
}

void LogicSystem::PostMsgToQue(shared_ptr<LogicNode> msg)
{
}

LogicSystem::LogicSystem() :_b_stop(false) {
	RegisterCallBacks();
	_worker_thread = std::thread(&LogicSystem::DealMsg, this);
}

void LogicSystem::DealMsg()
{
}
