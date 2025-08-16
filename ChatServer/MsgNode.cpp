#include "MsgNode.h"

RecvNode::RecvNode(short max_len, short msgid): MsgNode(max_len), msg_id(msgid){

}

SendNode::SendNode(const char *msg, short max_len, short msgid)
    : MsgNode(max_len + HEAD_DATA_LEN), msg_id(msgid)
{
    // 先发送id 
    short msg_id_net = boost::asio::detail::socket_ops::host_to_network_short(msg_id);
    memcpy(_data, &msg_id_net, HEAD_ID_LEN);
    // 最大长度
    short max_len_host = boost::asio::detail::socket_ops::host_to_network_short(max_len);
    memcpy(_data+HEAD_ID_LEN, &max_len_host, HEAD_ID_LEN);
    memcpy(_data+HEAD_ID_LEN+HEAD_DATA_LEN, msg, HEAD_ID_LEN);
}
