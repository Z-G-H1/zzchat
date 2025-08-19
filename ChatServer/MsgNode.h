#pragma once
#include "const.h"

// 基类节点， 存储节点的头部
class MsgNode{
public:
    MsgNode(short len) : _cur_len(0), _total_len(len){
        _data = new char[_total_len+1];
        _data[_total_len] = '\0';
    }

    ~MsgNode(){
        delete[] _data;
        std::cout << "Destruct MsgNode " << std::endl;
    }

    void Clear(){
        _cur_len = 0;
        memset(_data, 0, _total_len);
    }

    short _cur_len;
    short _total_len;
    char* _data;
};

class RecvNode : public MsgNode{
    friend class LogicSystem;
public:
    RecvNode(short max_len, short msg_id);
private:
    short msg_id;
};

class SendNode : public MsgNode{
public:
    SendNode(const char *msg,short max_len, short msg_id);
private:
    short msg_id;
};