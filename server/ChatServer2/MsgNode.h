#pragma once
#include "const.h"

using namespace std;
using boost::asio::ip::tcp;
class MsgNode
{
public:
	MsgNode(short max_len) : _total_len(max_len), _cur_len(0) {
		_data = new char[_total_len+1]();
		_data[_total_len] = '\0'; // ȷ���ַ�����null��β
	}

	~MsgNode() {
		delete[] _data;
		cout << "destruct MsgNode" << endl;
	}

	void Clear() {
		_cur_len = 0;
		memset(_data, 0, _total_len); // �������
	}

	short _cur_len; // ��ǰ��Ϣ����
	short _total_len; // ����Ϣ����
	char* _data;		// ��Ϣ����
};

class RecvNode : public MsgNode
{
	friend class LogicSystem;
public:
	RecvNode(short max_len, short msg_id);

private:
	short _msg_id; // ��ϢID
};

class SendNode : public MsgNode
{
	friend class LogicSystem;
public:
	SendNode(const char* msg,short max_len, short msg_id);
private:
	short _msg_id; // ��ϢID
};