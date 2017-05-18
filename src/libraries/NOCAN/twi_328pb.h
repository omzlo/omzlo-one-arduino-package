#ifndef _TWI_328PB_H_
#define _TWI_328PB_H_

#include <stdint.h>

void twi_init(void);

int8_t twi_start(uint8_t address);
int8_t twi_re_start(uint8_t address);
void twi_stop(void);

int8_t twi_write(uint8_t c);
uint8_t twi_read(uint8_t ack);

#define TWI_NACK 0
#define TWI_ACK 1

#endif
