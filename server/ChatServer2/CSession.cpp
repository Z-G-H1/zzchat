#include "CSession.h"
#include "CServer.h"
#include "LogicSystem.h"

CSession::CSession(boost::asio::io_context& io_context, CServer* server)
	: _socket(io_context), _server(server), _b_close(false), _b_head_parse(false)
{
	boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
	_session_id = boost::uuids::to_string(a_uuid);
	_recv_head_node = make_shared<MsgNode>(HEAD_TOTAL_LEN);
}
CSession::~CSession()
{
	std::cout << "~CSession destruct " << std::endl;
}

tcp::socket& CSession::GetSocket()
{
	return _socket;
}

std::string& CSession::GetSessionId()
{
	return _session_id;
}

void CSession::Start()
{
	AsyncReadHead(HEAD_TOTAL_LEN);
}

void CSession::Close() {
	_socket.close();
	_b_close = true;
}
int CSession::GetUserId()
{
	return _user_uid;
}
void CSession::SetUserId(int uid)
{
	_user_uid = uid;
}
std::shared_ptr<CSession> CSession::SharedSelf()
{
	return std::shared_ptr<CSession>();
}
void CSession::Send(char* msg, short max_length, short msgid)
{
	std::lock_guard<std::mutex>lock(_send_lock);
	int send_que_size = _send_que.size();

	if (send_que_size > MAX_SENDQUE) {
		std::cout << "session: " << _session_id << " send que faulled, size is " << MAX_SENDQUE << std::endl;
		return;
	}

	_send_que.push(make_shared<SendNode>(msg, max_length, msgid));
	if (send_que_size > 0) {
		return;
	}
	auto& msgnode = _send_que.front();
	boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
		std::bind(&CSession::HandleWrite, this, std::placeholders::_1, SharedSelf()));
}

void CSession::Send(std::string msg, short msgid)
{
	std::lock_guard<std::mutex> lock(_send_lock);
	int send_que_size = _send_que.size();

	if (send_que_size > MAX_SENDQUE) {
		std::cout << "session: " << _session_id << " send que faulled, size is " << MAX_SENDQUE << std::endl;
		return;
	}

	_send_que.push(make_shared<SendNode>(msg.c_str(), msg.length(), msgid));
	if (send_que_size > 0) {
		return;
	}
	auto& msgnode = _send_que.front();
	boost::asio::async_write(_socket,boost::asio::buffer(msgnode->_data, msgnode->_total_len),
		std::bind(&CSession::HandleWrite, this, std::placeholders::_1, SharedSelf()));
}

// 先读取头部数据
void CSession::AsyncReadHead(int total_len)
{
	// 获取当前对象的 shared_ptr，确保在异步操作进行期间 CSession 对象不会被销毁。
	auto self = shared_from_this();

	// 启动一个异步读取操作，期望读取 HEAD_TOTAL_LEN 长度的数据。
	asyncReadFull(HEAD_TOTAL_LEN, [self, this](const boost::system::error_code& ec, std::size_t bytes_transfered) {
		// 捕获 self 是为了生命周期管理，捕获 this 是为了方便访问 CSession 成员。

		try {
			//检查异步读取操作是否发生错误
			if (ec) {
				std::cout << "handler read failed, error is " << ec.what() << std::endl;
				Close();
				_server->ClearSession(_session_id);
				return;
			}

			// 检查实际读取的字节数是否符合预期头部长度
			if (bytes_transfered < HEAD_TOTAL_LEN) {
				std::cout << "read length not match, read [" << bytes_transfered << "] , total [" << HEAD_TOTAL_LEN << "]" << std::endl;
				Close();
				_server->ClearSession(_session_id);
				return;
			}

			// 将数据拷贝到 接收头部节点
			// 将从网络缓冲区 _data 中读取到的头部数据拷贝到 _recv_head_node 的数据区域
			_recv_head_node->Clear();
			memcpy(_recv_head_node->_data, _data, bytes_transfered);

			// 获取头部msg id
			short msg_id = 0;
			// 从头部数据的前 HEAD_ID_LEN 字节中拷贝出消息 ID
			memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);

			// 网络字节序转化为本地字节序
			msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
			std::cout << "msg_id is " << msg_id << endl;

			//检查 id非法
			if (msg_id > MAX_LENGTH) {
				std::cout << "invalid msg_id is " << msg_id << endl;
				_server->ClearSession(_session_id); // 移除非法会话
				Close();
				return;
			}

			// --- 解析长度字段 ---
			short msg_len = 0;
			memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
			// 网络字节序转化为本地字节序
			msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
			std::cout << "msg_len is " << msg_len << endl;

			//msg_len非法
			if (msg_len > MAX_LENGTH) {
				std::cout << "invalid data length is " << msg_len << endl;
				_server->ClearSession(_session_id);
				return;
			}

			// --- 准备接收消息体 ---
			// 根据解析出的消息体长度和消息 ID，创建一个新的 RecvNode 来存储完整的消息
			_recv_msg_node = make_shared<RecvNode>(msg_len, msg_id);
			AsyncReadBody(msg_len);
		}
		catch (std::exception& e) {
			std::cout << "Exception code is " << e.what() << endl;
		}
	});
}

void CSession::AsyncReadBody(int total_len)
{
	auto self = shared_from_this();
	asyncReadFull(total_len, [self, this, total_len](const boost::system::error_code& ec, std::size_t bytes_transfered) {
		try {
			if (ec) {
				std::cout << "handle read failed, error is " << ec.what() << endl;
				Close();
				_server->ClearSession(_session_id);
				return;
			}

			if (bytes_transfered < total_len) {
				std::cout << "handle read failed, error is " << ec.what() << endl;
				Close();
				_server->ClearSession(_session_id);
				return;
			}

			memcpy(_recv_msg_node->_data, _data, bytes_transfered);
			_recv_msg_node->_cur_len += bytes_transfered;
			_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
			cout << "receive data is " << _recv_msg_node->_data << endl;

			//此处将消息投递到逻辑队列中
			LogicSystem::GetInstance()->PostMsgToQue(make_shared<LogicNode>(shared_from_this(), _recv_msg_node));
			//继续监听头部接受事件
			AsyncReadHead(HEAD_TOTAL_LEN);
		}
		catch (std::exception& e) {
			std::cout << "Exception code is " << e.what() << endl;
		}
	});
}

// 读取整个长度， 然后解析收到的数据， 前两个字节为id 然后是长度， 后n个字节为数据
void CSession::asyncReadFull(std::size_t maxLength, std::function<void(const boost::system::error_code&, std::size_t)> handler)
{
	memset(_data, 0, MAX_LENGTH);
	asyncReadLen(0, maxLength, handler);
}

// 读取指定长度的数据
void CSession::asyncReadLen(std::size_t read_len, std::size_t total_len, std::function<void(const boost::system::error_code&, std::size_t)> handler)
{
	auto self = shared_from_this();
	_socket.async_read_some(boost::asio::buffer(_data + read_len, total_len - read_len), 
		[read_len, total_len, handler, self](const boost::system::error_code& ec, std::size_t  bytesTransfered) {

			// 出现错误 调用回调函数
			if(ec) {
				handler(ec, bytesTransfered);
				return;
			}

			if (read_len + bytesTransfered >= total_len) {
				// 长度够了就调用回调函数
				handler(ec, read_len + bytesTransfered);
				return;
			}

			// 没有错误， 且长度不足 继续读取
			self->asyncReadLen(read_len + bytesTransfered, total_len, handler);
	});
}

void CSession::HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> shared_self)
{
	try {
		if (!error) {
			std::lock_guard<std::mutex> lock(_send_lock);
			_send_que.pop();
			if (!_send_que.empty()) {
				auto& msgnode = _send_que.front();
				boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
					std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_self));
			}
		}
		else {
			std::cout << "handler write failed, error is " << error.what() << std::endl;
			Close();
			_server->ClearSession(_session_id);
		}
	}
	catch (std::exception& e) {
		std::cerr << "Exception code : " << e.what() << endl;
	}
}

LogicNode::LogicNode(shared_ptr<CSession> session, shared_ptr<RecvNode> recvnode)
	: _session(session), _recvnode(recvnode){

}