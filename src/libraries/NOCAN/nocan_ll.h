#ifndef _NOCAN_LL_H_
#define _NOCAN_LL_H_

#include <stdint.h>

typedef struct {
    uint8_t node_id;
    uint16_t channel_id;
    uint8_t dlen;
    uint8_t data[64];
} nocan_msg_t;

#define LL_SYSBIT                       0x02

#define LL_SYS_ANY                      0
#define LL_SYS_ADDRESS_REQUEST          1
#define LL_SYS_ADDRESS_CONFIGURE        2
#define LL_SYS_ADDRESS_CONFIGURE_ACK    3
#define LL_SYS_ADDRESS_LOOKUP           4
#define LL_SYS_ADDRESS_LOOKUP_ACK       5
#define LL_SYS_NODE_BOOT_REQUEST        6
#define LL_SYS_NODE_BOOT_ACK            7
#define LL_SYS_NODE_PING                8
#define LL_SYS_NODE_PING_ACK            9

#define LL_SYS_CHANNEL_REGISTER           10
#define LL_SYS_CHANNEL_REGISTER_ACK       11
#define LL_SYS_CHANNEL_UNREGISTER         12
#define LL_SYS_CHANNEL_UNREGISTER_ACK     13

#define LL_SYS_CHANNEL_SUBSCRIBE          14
#define LL_SYS_CHANNEL_UNSUBSCRIBE        15
#define LL_SYS_CHANNEL_LOOKUP             16
#define LL_SYS_CHANNEL_LOOKUP_ACK         17

#define LL_SYS_BOOTLOADER_GET_SIGNATURE		18
#define LL_SYS_BOOTLOADER_GET_SIGNATURE_ACK	19
#define LL_SYS_BOOTLOADER_SET_ADDRESS		20
#define LL_SYS_BOOTLOADER_SET_ADDRESS_ACK	21
#define LL_SYS_BOOTLOADER_WRITE			22
#define LL_SYS_BOOTLOADER_WRITE_ACK		23
#define LL_SYS_BOOTLOADER_READ			24
#define LL_SYS_BOOTLOADER_READ_ACK		25
#define LL_SYS_BOOTLOADER_LEAVE			26
#define LL_SYS_BOOTLOADER_LEAVE_ACK		27

#define NOCAN_LL_OK         0
#define NOCAN_LL_ERROR      (-1)
#define NOCAN_LL_TIMEOUT    (-2)

#define TWI_STATUS_RX_SYS   0x01 
#define TWI_STATUS_RX_MSG   0x02
#define TWI_STATUS_RX_ERR   0x04
#define TWI_STATUS_TX_RDY   0x08
#define TWI_STATUS_TX_ERR   0x10


int8_t nocan_ll_init(void);

int8_t nocan_ll_request_node_id(void);

int8_t nocan_ll_sys_send(int8_t node_id, uint8_t function, uint8_t param, uint8_t dlen, const uint8_t *data);

void nocan_ll_sys_recv_any(uint8_t *function, uint8_t *param, uint8_t *len, uint8_t *data);

int8_t nocan_ll_sys_recv(uint8_t function, uint8_t *param, uint8_t *len, uint8_t *data);

int8_t nocan_ll_msg_send(const nocan_msg_t *msg);

int8_t nocan_ll_msg_recv(nocan_msg_t *msg);

int8_t nocan_ll_msg_filter_add(uint16_t channel_id);

int8_t nocan_ll_msg_filter_remove(uint16_t channel_id);

int8_t nocan_ll_sys_filter_set(int8_t node_id);

uint8_t nocan_ll_signal(void);

uint8_t nocan_ll_status(void);

int8_t nocan_ll_get_udid(uint8_t *dest);

void nocan_ll_led(int on);

#endif
