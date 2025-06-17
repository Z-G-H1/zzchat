#include "CServer.h"
#include "AsioIOServicePool.h"
CServer::CServer(boost::asio::io_context& io_context, short port): _io_context(io_context), _port(port),
_acceptor(io_context, tcp::endpoint(tcp::v4(), port)) {
	// ��ʼ�� _acceptor ��������������������µ�����
	std::cout << "Server started success, listen on port : " << _port << std::endl;
	StartAccept();
}

CServer::~CServer() {
	cout << "Server destruct listen on port : " << _port << endl;
}

/**
 * @brief ����Ự�ĺ���
 * @param uuid 
 */
void CServer::ClearSession(std::string uuid)
{
	lock_guard<mutex> lock(_mutex);
	_sessions.erase(uuid);
}

/**
 * @brief ������������ӵ��첽������ɺ�Ļص�����
 * @param new_session �ս��ܵĿͻ������Ӷ�Ӧ�� CSession ����ָ��
 * @param error Boost.Asio ��������Ĵ�����
 */
void CServer::HandleAccept(shared_ptr<CSession> new_session, const boost::system::error_code& error)
{
	if (!error) {
		// ���û�д���˵�����ӳɹ������������»Ự�������շ�����
		new_session->Start();
		// ʹ�� std::lock_guard ������������ȷ���̰߳�ȫ�����»Ự��ӵ��Ự�����б�
		lock_guard<mutex> lock(_mutex);
		_sessions.insert(make_pair(new_session->GetUuid(), new_session));
	}
	else {
		cout << "session accept failed , error is " << error.what() << endl;
	}
	// ���۽��ܳɹ���񣬶�����������һ�ν��������ӵ��첽�������Ա�������ܳ�����������
	StartAccept();
}

// ����һ���첽�������ȴ�������һ���µĿͻ�������
void CServer::StartAccept()
{
	// �� AsioIOServicePool �л�ȡһ�� io_context ʵ��
	auto& io_context = AsioIOServicePool::GetInstance()->GetIOService();
	// ����һ���µ� CSession ����������������������      �����װ�� shared_ptr �У�ȷ�����첽�������ǰ���ᱻ����
	shared_ptr<CSession> new_session = make_shared<CSession>(io_context, this);
	// ����һ���첽���ܲ���
	// �����µĿͻ������ӵ���ʱ��_acceptor �Ὣ����ܵ� new_session ���׽�����
	// ������ɺ󣬻���� CServer::HandleAccept ����������
	// std::bind ���ڽ� CServer::HandleAccept ��Ա�����󶨵���ǰ CServer ���󣬲�Ԥ��һ��ռλ����������
	_acceptor.async_accept(new_session->GetSocket(), std::bind(&CServer::HandleAccept, this, new_session, placeholders::_1));
}

