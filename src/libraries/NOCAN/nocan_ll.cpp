#include "nocan_ll.h"
#include "twi_328pb.h"
#include <avr/io.h>
#include <util/delay.h>

#define TWI_NOCAN_NOP               0x00
#define TWI_NOCAN_SET_LED           0x01
#define TWI_NOCAN_RESET             0x02
#define TWI_NOCAN_GET_STATUS        0x03
#define TWI_NOCAN_SEND              0x04
#define TWI_NOCAN_RECV_SYS          0x05
#define TWI_NOCAN_RECV_MSG          0x06
#define TWI_NOCAN_MSG_FILTER_ADD    0x07
#define TWI_NOCAN_MSG_FILTER_REM    0x08
#define TWI_NOCAN_SYS_FILTER_SET    0x09
#define TWI_NOCAN_GET_UDID          0x0A

#define TWI_ADDR                    (0x12<<1)
#define TWI_READ                    0x01

#ifdef PB0_CAN_INT_GPIO
    #define DDR_CAN_INT                 DDRB
    #define GPIO_CAN_INT                PB0
    #define PIN_CAN_INT                 PINB
#else
    #define DDR_CAN_INT                 DDRE
    #define GPIO_CAN_INT                PE2
    #define PIN_CAN_INT                 PINE
#endif

int8_t nocan_ll_init(void)
{
    twi_start();
    twi_write(TWI_ADDR);
    twi_write(TWI_NOCAN_RESET);
    twi_write(0x01);
    twi_stop();

    DDR_CAN_INT &= ~(1<<GPIO_CAN_INT); 

    return NOCAN_LL_OK;
}

int8_t nocan_ll_request_node_id(void)
{
    int8_t status;
    int8_t node_id;
    uint8_t udid_send[8]; 
    uint8_t udid_recv[8];
    uint8_t attempts,i,rlen;

    if (nocan_ll_get_udid(udid_send)<0)
        return NOCAN_LL_ERROR;

    for (attempts=0;attempts<3;attempts++) 
    {
        status = nocan_ll_sys_send(0,LL_SYS_ADDRESS_REQUEST,0,8,udid_send);
    
        if (status!=0)
            return status;

        status = nocan_ll_sys_recv(LL_SYS_ADDRESS_CONFIGURE,(uint8_t *)&node_id,&rlen,udid_recv);

        if (status==0 && rlen==8)
        {
            for (i=0;i<8;i++)
                if (udid_send[i]!=udid_recv[i]) break;

            if (i==8) 
            {
                nocan_ll_sys_send(0,LL_SYS_ADDRESS_CONFIGURE_ACK,node_id,0,0);
                nocan_ll_led(1);
                return node_id;
            }
        }
    }
    return NOCAN_LL_ERROR;
}

int8_t _ll_send(uint8_t h1, uint8_t h2, uint8_t h3, uint8_t h4, uint8_t dlen, const uint8_t *data)
{
    uint16_t timeout;
    uint8_t status,i;

    timeout = 0;
    for (;;) {
        status = nocan_ll_status();
        if ((status & TWI_STATUS_TX_RDY)!=0)
            break;
        _delay_us(100);
        if (timeout++>1000) return NOCAN_LL_TIMEOUT;
    }

    twi_start();
    twi_write(TWI_ADDR);
    twi_write(TWI_NOCAN_SEND);
    twi_write(h1);
    twi_write(h2);
    twi_write(h3);
    twi_write(h4);
    twi_write(dlen);
    for (i=0;i<dlen;i++)
        twi_write(data[i]);
    twi_stop();

    /*
    timeout = 0;
    while (timeout<1000) {
        status = nocan_ll_status();
        if (status & TWI_STATUS_TX_RDY)!=0)
            return NOCAN_LL_OK;
        _delay_us(100);
        timeout++;
    }
    return NOCAN_LL_TIMEOUT;
    */
    return NOCAN_LL_OK;
}

int8_t nocan_ll_sys_send(int8_t node_id, uint8_t function, uint8_t param, uint8_t dlen, const uint8_t *data)
{
    return _ll_send(node_id>>3, node_id<<5 | (1<<2), function, param, dlen, data);
}

void nocan_ll_sys_recv_any(uint8_t *function, uint8_t *param, uint8_t *len, uint8_t *data)
{
    uint8_t i;
    uint8_t rlen,h4;

    twi_start();
    twi_write(TWI_ADDR);
    twi_write(TWI_NOCAN_RECV_SYS);
    twi_start();
    twi_write(TWI_ADDR|TWI_READ);
    twi_read(TWI_ACK);
    twi_read(TWI_ACK);
    *function = twi_read(TWI_ACK);
    h4 = twi_read(TWI_ACK);
    rlen = twi_read(TWI_ACK);
    if (data==0)
    {
        twi_read(TWI_NACK);
    }
    else
    {
        if (rlen>0) 
        {
            for (i=0;i<rlen-1;i++)
                data[i] = twi_read(TWI_ACK);
            data[rlen-1] = twi_read(TWI_NACK);
        }
        else
        {
            twi_read(TWI_NACK);
        }
    }
    twi_stop();

    if (len) *len=rlen;

    if (param) *param = h4;
}

int8_t nocan_ll_sys_recv(uint8_t function, uint8_t *param, uint8_t *len, uint8_t *data)
{
    uint16_t timeout;
    uint8_t status;
    uint8_t afunction;

    timeout = 0;
    for (;;)
    {
        for (;;) {
            if (nocan_ll_signal())
            {
                status = nocan_ll_status();
                if ((status & TWI_STATUS_RX_SYS)!=0)
                    break;
            }
            _delay_us(100);
            if (timeout++>1000) return NOCAN_LL_TIMEOUT;
        }

        nocan_ll_sys_recv_any(&afunction, param, len, data);

        if (afunction == function)
        {
            break;
        }
    }
    return NOCAN_LL_OK; 
}

int8_t nocan_ll_msg_send(const nocan_msg_t *msg)
{
    return _ll_send(msg->node_id>>3, msg->node_id<<5, msg->channel_id>>8, msg->channel_id&0xFF, msg->dlen, msg->data);
}

int8_t nocan_ll_msg_recv(nocan_msg_t *msg)
{
    uint16_t timeout;
    uint8_t status,i;
    uint8_t h1,h2,h3,h4;

    timeout = 0;
    for (;;) {
        if (nocan_ll_signal())
        {
            status = nocan_ll_status();
            if ((status & TWI_STATUS_RX_MSG)!=0)
                break;
        }
        _delay_us(100);
        if (timeout++>1000) return NOCAN_LL_TIMEOUT;
    }

    twi_start();
    twi_write(TWI_ADDR);
    twi_write(TWI_NOCAN_RECV_MSG);
    twi_start();
    twi_write(TWI_ADDR|TWI_READ);
    h1 = twi_read(TWI_ACK);
    h2 = twi_read(TWI_ACK);
    h3 = twi_read(TWI_ACK);
    h4 = twi_read(TWI_ACK);
    msg->dlen = twi_read(TWI_ACK);

    if (msg->dlen>64)
        msg->dlen=64;

    if (msg->dlen>0) 
    {
        for (i=0;i<msg->dlen-1;i++)
            msg->data[i] = twi_read(TWI_ACK);
        msg->data[msg->dlen-1] = twi_read(TWI_NACK);
    }
    else
    {
        twi_read(TWI_NACK);
    }
    twi_stop();

    msg->channel_id = ((uint16_t)h3<<8)|(uint16_t)h4;
    msg->node_id = ((h1&0xF)<<3)|(h2>>5);

    return NOCAN_LL_OK; 

}

int8_t nocan_ll_msg_filter_add(uint16_t channel_id)
{
    uint8_t ss;

    twi_start();
    twi_write(TWI_ADDR);
    twi_write(TWI_NOCAN_MSG_FILTER_ADD);
    twi_write(channel_id>>8);
    twi_write(channel_id&0xFF);
    twi_start();
    twi_write(TWI_ADDR|TWI_READ);
    ss = twi_read(TWI_NACK);
    twi_stop();

    return ss;
}

int8_t nocan_ll_msg_filter_remove(uint16_t channel_id)
{
    uint8_t ss;

    twi_start();
    twi_write(TWI_ADDR);
    twi_write(TWI_NOCAN_MSG_FILTER_REM);
    twi_write(channel_id>>8);
    twi_write(channel_id&0xFF);
    twi_start();
    twi_write(TWI_ADDR|TWI_READ);
    ss = twi_read(TWI_NACK);
    twi_stop();

    return ss;
}

int8_t nocan_ll_sys_filter_set(int8_t node_id)
{
    uint8_t ss;

    twi_start();
    twi_write(TWI_ADDR);
    twi_write(TWI_NOCAN_SYS_FILTER_SET);
    twi_write(node_id);
    twi_start();
    twi_write(TWI_ADDR|TWI_READ);
    ss = twi_read(TWI_NACK);
    twi_stop();

    return ss;
}

uint8_t nocan_ll_signal(void)
{
    return bit_is_clear(PIN_CAN_INT,GPIO_CAN_INT);
}

uint8_t nocan_ll_status(void)
{
    uint8_t ss;

    twi_start();
    twi_write(TWI_ADDR);
    twi_write(TWI_NOCAN_GET_STATUS);
    twi_start();
    twi_write(TWI_ADDR|TWI_READ);
    ss = twi_read(TWI_NACK);
    twi_stop();

    return ss;
}

int8_t nocan_ll_get_udid(uint8_t *dest)
{
    uint8_t i;

    twi_start();
    twi_write(TWI_ADDR);
    twi_write(TWI_NOCAN_GET_UDID);
    twi_start();
    twi_write(TWI_ADDR|TWI_READ);
    for (i=0;i<7;i++)
        dest[i]=twi_read(TWI_ACK);
    dest[7]=twi_read(TWI_NACK);
    twi_stop();

    return NOCAN_LL_OK;
}

void nocan_ll_led(int on)
{
    twi_start();
    twi_write(TWI_ADDR);
    twi_write(TWI_NOCAN_SET_LED);
    twi_write(on);
    twi_stop();
}

