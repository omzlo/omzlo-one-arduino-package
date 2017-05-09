#include "nocan.h"

NocanClass Nocan;

#ifndef NULL
#define NULL (0)
#endif

NocanNodeId NocanClass::open(void)
{
    if (nocan_ll_init()<0) return NocanClass::ERROR;

    _node_id = nocan_ll_request_node_id();

    if (_node_id<=0)
        return 0;

    if (nocan_ll_sys_filter_set(_node_id)<0)
    {
        return NocanClass::ERROR;
    }

    return _node_id;
}

int8_t NocanClass::lookupChannel(const char *channel, NocanChannelId &channel_id) const
{
    int8_t status;
    int8_t lookup_success;
    uint8_t channel_len = 0;
    uint8_t rlen;
    uint8_t c[8];

    while (channel_len<64 && channel[channel_len]!=0) channel_len++;
	
    status = nocan_ll_sys_send(_node_id,LL_SYS_CHANNEL_LOOKUP,0,channel_len,(const uint8_t *)channel);
    if (status<0)      
        return status;

    status = nocan_ll_sys_recv(LL_SYS_CHANNEL_LOOKUP_ACK,(uint8_t *)&lookup_success,&rlen,c);

    if (status<0)
        return status;

    if (lookup_success<0 || rlen!=2)
        return NocanClass::ERROR;

    channel_id = ((uint16_t)c[0])<<8 | c[1];

    return NocanClass::OK;

}

int8_t NocanClass::registerChannel(const char *channel, NocanChannelId &channel_id) const
{
    int8_t status;
    uint8_t channel_len = 0;
    int8_t register_success;
    uint8_t rlen;
    uint8_t c[8];

    while (channel_len<64 && channel[channel_len]!=0) channel_len++;
	
    status = nocan_ll_sys_send(_node_id,LL_SYS_CHANNEL_REGISTER,0,channel_len,(const uint8_t *)channel);
    if (status<0)      
        return status;

    status = nocan_ll_sys_recv(LL_SYS_CHANNEL_REGISTER_ACK,(uint8_t *)&register_success,&rlen,c);

    if (status<0)
        return status;

    if (status<0)
        return status;

    if (register_success<0 || rlen!=2)
        return NocanClass::ERROR;

    channel_id = ((uint16_t)c[0])<<8 | c[1];

    return NocanClass::OK;
}

int8_t NocanClass::unregisterChannel(NocanChannelId channel_id) const
{
    int8_t status;
    uint8_t c[2];

    c[0] = channel_id >> 8;
    c[1] = channel_id & 0xFF;

    status = nocan_ll_sys_send(_node_id,LL_SYS_CHANNEL_UNREGISTER,0,2,c);
    if (status<0)
        return status;

    status = nocan_ll_sys_recv(LL_SYS_CHANNEL_UNREGISTER_ACK,NULL,NULL,NULL);
    if (status<0)
        return status;

    return NocanClass::OK;
}

int8_t NocanClass::subscribeChannel(NocanChannelId channel_id) const
{
    int8_t status;
    uint8_t c[2];

    c[0] = channel_id >> 8;
    c[1] = channel_id & 0xFF;

    status = nocan_ll_sys_send(_node_id,LL_SYS_CHANNEL_SUBSCRIBE,0,2,c);

    if (status<0)
        return status;

    return nocan_ll_msg_filter_add(channel_id);
}

int8_t NocanClass::unsubscribeChannel(NocanChannelId channel_id) const
{
    int8_t status;
    uint8_t c[2];

    c[0] = channel_id >> 8;
    c[1] = channel_id & 0xFF;

    status = nocan_ll_sys_send(_node_id,LL_SYS_CHANNEL_UNSUBSCRIBE,0,2,c);

    if (status<0)
        return status;

    return nocan_ll_msg_filter_remove(channel_id);
}

int8_t NocanClass::publishMessage(NocanMessage &msg) const
{
    msg.node_id = _node_id;
    return nocan_ll_msg_send(&msg);
}

int8_t NocanClass::publishMessage(NocanChannelId cid, const char *msg) const
{
    NocanMessage m;
    m.node_id = _node_id;
    m.channel_id = cid;
    m.dlen = 0;
    while (m.dlen<64 && *msg++) m.dlen++;
    return nocan_ll_msg_send(&m);
}

int8_t NocanClass::receiveMessage(NocanMessage &msg) const
{
    return nocan_ll_msg_recv(&msg);
}

