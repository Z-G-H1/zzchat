#include "tcpmgr.h"
#include <QAbstractSocket>

TcpMgr::TcpMgr() : _host(""), _port(0), _b_recv_pending(false), _message_id(0), _message_len()
{
    QObject::connect(&_socket, &QTcpSocket::connected, [&](){
        qDebug() << "Connected to server!";
        // 连接成功后发送信号
        emit sig_con_success(true);
    });

    QObject::connect(&_socket, &QTcpSocket::readyRead, [&](){
        // 当有数据可以读时，读取数据
        // 读取所有数据到缓冲区
        _buffer.append(_socket.readAll());

        QDataStream stream(&_buffer, QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_5_0);

        forever{
            // 解析头部
            if(!_b_recv_pending){
                // 检查缓冲区中的数据是否足够解析出一个信息头（消息id + 消息长度）
                if(_buffer.size() < static_cast<int>(sizeof(quint16) * 2)){
                    return ; // 数据不够
                }
                // 读取消息ID 和 消息长度， 但不从缓冲区移除
                stream >> _message_id >> _message_len;

                // 将buffer 中的前四个字节移除
                _buffer = _buffer.mid(sizeof(quint16) * 2);

                // d打印读取数据
                qDebug() << "Message ID: " << _message_id << ", Length:" << _message_len;
            }

            // buffer 剩余长度是否够 消息长度
            if(_buffer.size() < _message_len) {
                _b_recv_pending = true;
                return ;
            }

            _b_recv_pending = false;
            // 读取消息体
            QByteArray messageBody = _buffer.mid(0, _message_len);
            qDebug() << "receive body msg is " << messageBody ;

            _buffer = _buffer.mid(_message_len);
            handleMsg(ReqId(_message_id), _message_len, messageBody);
        }
    });

    QObject::connect(&_socket, static_cast<void (QTcpSocket::*)(QTcpSocket::SocketError)>(&QTcpSocket::error),
                                [&](QTcpSocket::SocketError socketError) {
        qDebug() << "Error:" << _socket.errorString() ;
        switch (socketError) {
            case QTcpSocket::ConnectionRefusedError:
                qDebug() << "Connection Refused!";
                emit sig_con_success(false);
                break;
            case QTcpSocket::RemoteHostClosedError:
                qDebug() << "Remote Host Closed Connection!";
                break;
            case QTcpSocket::HostNotFoundError:
                qDebug() << "Host Not Found!";
                emit sig_con_success(false);
                break;
            case QTcpSocket::SocketTimeoutError:
                qDebug() << "Connection Timeout!";
                emit sig_con_success(false);
                break;
            case QTcpSocket::NetworkError:
                qDebug() << "Network Error!";
                break;
            default:
                qDebug() << "Other Error!";
                break;
        }
    });

    // 处理断开连接
    QObject::connect(&_socket, &QTcpSocket::disconnected, [&](){
        qDebug() << "Disconnected from server.";
    });

    QObject::connect(this, &TcpMgr::sig_send_data, this, &TcpMgr::slot_send_data);
    //注册消息
    initHandlers();
}

void TcpMgr::initHandlers()
{
    _handlers.insert(ID_CHAT_LOGIN_RSP,[this](ReqId id, int len, QByteArray data){
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将数组转换为json
        QJsonDocument jsonObj = QJsonDocument::fromJson(data);

        //检查转换是否成功
        if(jsonObj.isNull()){
            qDebug() << "Failed to create QJsonDocument.";
            return ;
        }

        int err = jsonObj["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "Login Failed, err is " << err;
            emit sig_login_failed(err);
            return ;
        }

    });
}

void TcpMgr::handleMsg(ReqId id, int len, QByteArray data)
{
    auto find_iter = _handlers.find(id);
    if(find_iter == _handlers.end()){
        qDebug() << "not found id[" << id << "] to handle";
        return;
    }
    find_iter.value()(id, len, data);
}

void TcpMgr::slot_tcp_connect(ServerInfo si)
{
    qDebug() << "receive tcp connect signal";
    // 尝试连接服务器
    qDebug() << "Connecting to server ...";
    _host = si.Host;
    _port = static_cast<uint16_t>(si.Port.toInt());
    _socket.connectToHost(_host, _port);
}

void TcpMgr::slot_send_data(ReqId reqId, QString data)
{
    uint16_t id = reqId;

    // 将字符串转换为UTF-8 字节数组
    QByteArray dataBytes = data.toUtf8();

    // 计算长度 （使用网络字节序转换
    quint16 len = static_cast<qint16>(data.size());

    // 创建数组存储数据
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    // 设置数据流网络字节序
    out.setByteOrder(QDataStream::BigEndian);

    //写入ID 和 长度
    out << id << len;

    // 添加字符串
    block.append(data);

    // 发送数据
    _socket.write(block);

}
