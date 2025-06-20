#pragma once
#include "const.h"

using namespace std;
using boost::asio::ip::tcp;
class MsgNode
{
public:
	MsgNode(short max_len) : _total_len(max_len), _cur_len(0) {
		_data = new char[_total_len+1]();
		_data[_total_len] = '\0'; // 确保字符串以null结尾
	}

	~MsgNode() {
		delete[] _data;
		cout << "destruct MsgNode" << endl;
	}

	void Clear() {
		_cur_len = 0;
		memset(_data, 0, _total_len); // 清空数据
	}

	short _cur_len; // 当前消息长度
	short _total_len; // 总消息长度
	char* _data;		// 消息数据
};

class RecvNode : public MsgNode
{
	friend class LogicSystem;
public:
	RecvNode(short max_len, short msg_id);

private:
	short _msg_id; // 消息ID
};

class SendNode : public MsgNode
{
	friend class LogicSystem;
public:
	SendNode(const char* msg,short max_len, short msg_id);
private:
	short _msg_id; // 消息ID
};