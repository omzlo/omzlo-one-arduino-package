#include "twi_328pb.h"
#include <avr/io.h>
#include <util/twi.h>

#define TWSR TWSR1
#define TWBR TWBR1
#define TWCR TWCR1
#define TWDR TWDR1

#ifndef TWIE 
    #define TWIE TWIE1
    #define TWEN TWEN1 
    #define TWWC TWWC1
    #define TWSTO TWSTO1
    #define TWSTA TWSTA1
    #define TWEA TWEA1
    #define TWINT TWINT1
#endif

uint8_t TWI_ERROR_COUNT = 0;

void twi_init(void)
{
                // prescale == 0
    TWSR = 0x00;
                // for 100Khz @16Mhz
    TWBR = 72;  
                /*
                     16.000.000
                ---------------------
                16+2(TWBR).(prescale)

                presecale = TWSR lower 2 bits, 0 => 1, 1=>4, 2=>16, 3=>64

                16+(2*12) = 40, 16m / 40 => 400k
                16+(2*72) = 160, 16m / 160 => 100k
    */
}

int8_t twi_re_start(uint8_t address)
{
    uint8_t status;

    // TWCR => TW control reg
    // TWINT => write one, clears bit
    // TWSTA => send start condition
    // TWEN => enable twi
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    // wait until WINT is set, indicates START condition transmitted
    while ((TWCR & (1<<TWINT)) == 0);
    // We could check for errors by looking into TWSR higher 5 bits

    status = TWSR & 0xF8;
    if ((status!=TW_START) && (status!=TW_REP_START)) {
        TWI_ERROR_COUNT++;
        return -1;
    }

    // send addr
    TWDR = address;
    TWCR = (1<<TWINT) | (1<<TWEN);

    // wait again
    while ((TWCR & (1<<TWINT)) == 0);

    status = TWSR & 0xF8;

    if ((status==TW_MT_SLA_NACK) || (status==TW_MR_DATA_NACK))
    {
        twi_stop();
        TWI_ERROR_COUNT++;
        return -1;
    }

    return 0;
}

int8_t twi_start(uint8_t address)
{
    uint8_t status;

    for (;;)
    {
        if (twi_re_start(address)==0) break;
    }

    return 0;
}

void twi_stop(void)
{
                // TWSTO => stop condition
    TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
    while ((TWCR & (1 << TWSTO)) == 0);
}

int8_t twi_write(uint8_t c)
{
    uint8_t status;

    TWDR = c;                               // TWDR => data or address register
    TWCR = (1<<TWINT)|(1<<TWEN);            // TWINT => writing one clears it
    while ((TWCR & (1<<TWINT)) == 0);

    status = TWSR & 0xF8;
    if (status!=TW_MT_DATA_ACK) {
        TWI_ERROR_COUNT++;
        return -1;
    }

    return 0;
}

uint8_t twi_read(uint8_t ack)
{
    if (ack == TWI_ACK)
        TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    else
        TWCR = (1<<TWINT)|(1<<TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
    return TWDR;
}

