#include "CSession.h"
#include "LogicSystem.h"

CSession::CSession(net::io_context& ioc, CServer* server)
    : _socket(ioc), _server(server), _b_head_parse(false)
{
    boost::uuids::uuid  a_uuid = boost::uuids::random_generator()();
	_uuid = boost::uuids::to_string(a_uuid);
    _recv_head_node = std::make_shared<MsgNode>(HEAD_TOTAL_LEN);
}

void CSession::Start(){
    AsyncReadHead(HEAD_TOTAL_LEN);
}

tcp::socket& CSession::GetSocket(){
    return _socket;
}

std::string& CSession::GetUuid(){
    return _uuid;
}


void CSession::AsyncReadHead(int total_len){

    AsyncReadFull(HEAD_TOTAL_LEN,[self = shared_from_this(), this](const boost::system::error_code& ec, std::size_t bytes_transfered){
        try{
            if(ec){
                std::cout << "handle read failed, error is " << ec.what() << std::endl;
                Close();
                _server->ClearSession(_uuid);
                return;
            } 
            // 没有错误，读取到了bytes字节的数据
            if(bytes_transfered < HEAD_TOTAL_LEN){
                std::cout << "read length not match, read [" << bytes_transfered << "] , total ["
                    << HEAD_TOTAL_LEN << "]" << std::endl;
                Close();
                _server->ClearSession(_uuid);
                return;
            }

            _recv_head_node->Clear();
            memcpy(_recv_head_node->_data, _data, bytes_transfered);

            // 获取消息id
            short msg_id;
            memcpy(_recv_head_node->_data, &msg_id, HEAD_ID_LEN);
            // 字节序转换， 将网络字节序转换为本地
            msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
            std::cout << "msg_id is " << msg_id << std::endl;
            // id 非法
            if(msg_id > MAX_LENGTH){
                std::cout << "invalid msg_id is " << msg_id << std::endl;
                _server->ClearSession(_uuid);
                return;
            }

            // 获取消息长度
            short msg_len;
            memcpy(_recv_head_node->_data, &msg_len, HEAD_DATA_LEN);
            // 转换字节序
            msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
            std::cout << "msg_len is "<< msg_len << std::endl;         
            // msglen 非法
            if(msg_len > MAX_LENGTH){
                std::cout << "invalid msg_len is " << msg_len << std::endl;
                _server->ClearSession(_uuid);
                return;
            }

            // 创建消息节点
            _recv_msg_node = std::make_shared<RecvNode>(msg_len, msg_id);
            AsyncReadBody(msg_len);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        
    });
}

void CSession::AsyncReadBody(int total_len){
    auto self = shared_from_this();
    AsyncReadFull(total_len, [self, this, total_len](const boost::system::error_code& ec, std::size_t bytes_transfered) {
        try {
            if (ec) {
                std::cout << "handle read failed, error is " << ec.what() << std::endl;
                Close();
                _server->ClearSession(_uuid);
                return;
            }

            if (bytes_transfered < total_len) {
                std::cout << "read length not match, read [" << bytes_transfered << "] , total ["
                    << total_len<<"]" << std::endl;
                Close();
                _server->ClearSession(_uuid);
                return;
            }

            memcpy(_recv_msg_node->_data , _data , bytes_transfered);
            _recv_msg_node->_cur_len += bytes_transfered;
            _recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
            std::cout << "receive data is " << _recv_msg_node->_data << std::endl;
            //此处将消息投递到逻辑队列中
            LogicSystem::GetInstance()->PostMsgToQue(std::make_shared<LogicNode>(shared_from_this(), _recv_msg_node));
            //继续监听头部接受事件
            AsyncReadHead(HEAD_TOTAL_LEN);
        }
        catch (std::exception& e) {
            std::cout << "Exception code is " << e.what() << std::endl;
        }
        });
}

void CSession::AsyncReadFull(std::size_t maxLength, std::function<void(const boost::system::error_code&, std::size_t)> handler){
    memset(_data, 0, MAX_LENGTH);
    AsyncReadLen(0, maxLength, handler);
}

void CSession::AsyncReadLen(std::size_t read_len, std::size_t total_len, std::function<void(const boost::system::error_code&, std::size_t)> handler){
    auto self = shared_from_this();
    _socket.async_read_some(boost::asio::buffer(_data+read_len, total_len-read_len),
        [handler, read_len, total_len, self](const boost::system::error_code& ec, std::size_t  bytesTransfered){
            if(ec){
                handler(ec, bytesTransfered + read_len);
                return;
            }

            if(bytesTransfered + read_len >= total_len){
                handler(ec, read_len + bytesTransfered);
                return ;
            }

            // 没有错误， 但是读取的长度还不够
            self->AsyncReadLen(read_len+bytesTransfered, total_len, handler);
    });
}
