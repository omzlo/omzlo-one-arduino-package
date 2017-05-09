#ifndef _TWI_328PB_H_
#define _TWI_328PB_H_

#include <stdint.h>

void twi_init(void);

void twi_start(void);
void twi_stop(void);

void twi_write(uint8_t c);
uint8_t twi_read(uint8_t ack);

#define TWI_NACK 0
#define TWI_ACK 1

#endif
