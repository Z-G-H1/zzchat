#include "const.h"
#include "MsgNode.h"
#include "CServer.h"


class CSession : public std::enable_shared_from_this<CSession>{

public:
    CSession(net::io_context& ioc, CServer* server);
    void Start();
    tcp::socket& GetSocket();
    std::string& GetUuid();
    void Send(char* msg, int max_len, short msg_id);
    void Send(std::string msg, short msgid);
    void Close();
    void AsyncReadBody(int length);
    void AsyncReadHead(int total_len);
private:
    void AsyncReadFull(std::size_t maxLength, std::function<void(const boost::system::error_code&, std::size_t)> handler);
    void AsyncReadLen(std::size_t read_len, std::size_t total_len, std::function<void(const boost::system::error_code&, std::size_t)> handler);
    void HandleWrite(const boost::system::error_code& error, size_t bytes_transferred);

    tcp::socket _socket;
    std::string _uuid;
    char _data[MAX_LENGTH];
    CServer* _server;
    std::queue<std::shared_ptr<SendNode>> _send_que;
    std::mutex _send_lock;

    bool _b_head_parse;
    std::shared_ptr<MsgNode> _recv_head_node;
    std::shared_ptr<RecvNode> _recv_msg_node;
};

class LogicNode {
	friend class LogicSystem;
public:
	LogicNode(std::shared_ptr<CSession>, std::shared_ptr<RecvNode>);
private:
	std::shared_ptr<CSession> _session;
	std::shared_ptr<RecvNode> _recv_node;
};