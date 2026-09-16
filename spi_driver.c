#include "spi_driver.h"
#include <avr/io.h>


#define DISP 1
#define IO 1
#define CAN 1

void spi_init(void)
{
	DDRB = (1 << DDB1)|(1 << DDB2)|(1 << DDB3)|(1 << DDB4)| (1<<DDB5)|(1<<DDB7); //Set MOSI and SCK ouput, use PB2-PB4 as chip select, PBX as D/!C
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0); //Enable SPI, Master, set clock rate fck/16 (må kanskje endres på)
	// nå er MSB of the data transmitted first
	spi_deselect();
}

void spi_deselect() {
	PORTB |= (1 << PB4) | (1 << PB0);
    PORTD |= (1 << PD4);
}

void spi_select_slave(int n) {
	spi_deselect();	

    switch (n) {
        case 0:  // Display
            PORTB &= ~(1 << PB4);
            break;
        case 1:  // IO
            PORTB &= ~(1 << PB0);
            break;
        case 2:  // CAN
            PORTD &= ~(1 << PD4);
            break;
        default:
            // ugyldig slavenummer – ingen slave velges
            break;
    }
}


uint8_t spi_transfer(uint8_t data)
{
	SPDR = data; // Start transmission 
	while(!(SPSR & (1<<SPIF))) {}; // Wait for transmission complete
	return SPDR;
}

void spi_write_n_bytes(const uint8_t *data, uint16_t n)
{
    for (uint16_t i = 0; i < n; i++) {
        spi_transfer(data[i]);
    }
}

void spi_read_n_bytes(uint8_t *buffer, uint16_t n)
{
    for (uint16_t i = 0; i < n; i++) {
        buffer[i] = spi_transfer(0xFF);
    }
}
