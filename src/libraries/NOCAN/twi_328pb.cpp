#include "twi_328pb.h"
#include <avr/io.h>

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
    //enable TWI
    TWCR = (1<<TWEN);
}

void twi_start(void)
{
                // TWCR => TW control reg
                // TWINT => write one, clears bit
                // TWSTA => send start condition
                // TWEN => keep it enabled (it was set in i2c_init)
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
                // wait until WINT is set, indicates START condition transmitted
    while ((TWCR & (1<<TWINT)) == 0);
                // We could check for errors by looking into TWSR higher 5 bits
}

void twi_stop(void)
{
                // TWSTO => stop condition
    TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
    while ((TWCR & (1 << TWSTO)) == 0);
}

void twi_write(uint8_t c)
{
    TWDR = c;                               // TWDR => data or address register
    TWCR = (1<<TWINT)|(1<<TWEN);            // TWINT => writing one clears it
    while ((TWCR & (1<<TWINT)) == 0);
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

