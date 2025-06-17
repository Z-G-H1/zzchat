#include "CSession.h"
#include "CServer.h"
#include "LogicSystem.h"

CSession::CSession(boost::asio::io_context& io_context, CServer* server)
	: _socket(io_context), _server(server), _b_close(false), _b_head_parse(false)
{
	boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
	_uuid = boost::uuids::to_string(a_uuid);
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

std::string& CSession::GetUuid()
{
	return _uuid;
}

void CSession::Start()
{
	AsyncReadHead(HEAD_TOTAL_LEN);
}

void CSession::Close() {

}
void CSession::Send(char* msg, short max_length, short msgid)
{
}

void CSession::Send(std::string msg, short msgid)
{
}


// �ȶ�ȡͷ������
void CSession::AsyncReadHead(int total_len)
{
	// ��ȡ��ǰ����� shared_ptr��ȷ�����첽���������ڼ� CSession ���󲻻ᱻ���١�
	auto self = shared_from_this();

	// ����һ���첽��ȡ������������ȡ HEAD_TOTAL_LEN ���ȵ����ݡ�
	asyncReadFull(HEAD_TOTAL_LEN, [self, this](const boost::system::error_code& ec, std::size_t bytes_transfered) {
		// ���� self ��Ϊ���������ڹ������� this ��Ϊ�˷������ CSession ��Ա��

		try {

			//����첽��ȡ�����Ƿ�������
			if (ec) {
				std::cout << "handler read failed, error is " << ec.what() << std::endl;
				Close();
				_server->ClearSession(_uuid);
				return;
			}

			// ���ʵ�ʶ�ȡ���ֽ����Ƿ����Ԥ��ͷ������
			if (bytes_transfered < HEAD_TOTAL_LEN) {
				std::cout << "read length not match, read [" << bytes_transfered << "] , total [" << HEAD_TOTAL_LEN << "]" << std::endl;
				Close();
				_server->ClearSession(_uuid);
				return;
			}

			// �����ݿ����� ����ͷ���ڵ�
			// �������绺���� _data �ж�ȡ����ͷ�����ݿ����� _recv_head_node ����������
			_recv_head_node->Clear();
			memcpy(_recv_head_node->_data, _data, bytes_transfered);

			// ��ȡͷ��msg id
			short msg_id = 0;
			// ��ͷ�����ݵ�ǰ HEAD_ID_LEN �ֽ��п�������Ϣ ID
			memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);

			// �����ֽ���ת��Ϊ�����ֽ���
			msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
			std::cout << "msg_id is " << msg_id << endl;

			//��� id�Ƿ�
			if (msg_id > MAX_LENGTH) {
				std::cout << "invalid msg_id is " << msg_id << endl;
				_server->ClearSession(_uuid); // �Ƴ��Ƿ��Ự
				return;
			}

			// --- ���������ֶ� ---
			short msg_len = 0;
			memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
			// �����ֽ���ת��Ϊ�����ֽ���
			msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
			std::cout << "msg_len is " << msg_len << endl;

			//msg_len�Ƿ�
			if (msg_len > MAX_LENGTH) {
				std::cout << "invalid data length is " << msg_len << endl;
				_server->ClearSession(_uuid);
				return;
			}

			// --- ׼��������Ϣ�� ---
			// ���ݽ���������Ϣ�峤�Ⱥ���Ϣ ID������һ���µ� RecvNode ���洢��������Ϣ
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
				_server->ClearSession(_uuid);
				return;
			}

			if (bytes_transfered < total_len) {
				std::cout << "handle read failed, error is " << ec.what() << endl;
				Close();
				_server->ClearSession(_uuid);
				return;
			}

			memcpy(_recv_msg_node->_data, _data, bytes_transfered);
			_recv_msg_node->_cur_len += bytes_transfered;
			_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
			cout << "receive data is " << _recv_msg_node->_data << endl;

			//�˴�����ϢͶ�ݵ��߼�������
			LogicSystem::GetInstance()->PostMsgToQue(make_shared<LogicNode>(shared_from_this(), _recv_msg_node));
			//��������ͷ�������¼�
			AsyncReadHead(HEAD_TOTAL_LEN);
		}
		catch (std::exception& e) {
			std::cout << "Exception code is " << e.what() << endl;
		}
	});
}

// ��ȡ�������ȣ� Ȼ������յ������ݣ� ǰ�����ֽ�Ϊid Ȼ���ǳ��ȣ� ��n���ֽ�Ϊ����
void CSession::asyncReadFull(std::size_t maxLength, std::function<void(const boost::system::error_code&, std::size_t)> handler)
{
	memset(_data, 0, MAX_LENGTH);
	asyncReadLen(0, maxLength, handler);
}

// ��ȡָ�����ȵ�����
void CSession::asyncReadLen(std::size_t read_len, std::size_t total_len, std::function<void(const boost::system::error_code&, std::size_t)> handler)
{
	auto self = shared_from_this();
	_socket.async_read_some(boost::asio::buffer(_data + read_len, total_len - read_len), 
		[read_len, total_len, handler, self](const boost::system::error_code& ec, std::size_t  bytesTransfered) {

			// ���ִ��� ���ûص�����
			if(ec) {
				handler(ec, bytesTransfered);
				return;
			}

			if (read_len + bytesTransfered >= total_len) {
				// ���ȹ��˾͵��ûص�����
				handler(ec, read_len + bytesTransfered);
				return;
			}

			// û�д��� �ҳ��Ȳ��� ������ȡ
			self->asyncReadLen(read_len + bytesTransfered, total_len, handler);
	});
}

void CSession::HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> shared_self)
{
}

LogicNode::LogicNode(shared_ptr<CSession>, shared_ptr<RecvNode>)
{
}
