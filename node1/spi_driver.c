#include "spi_driver.h"
#include <avr/io.h>
#include <stdint.h>

// Sets up the pins, deselects all devices and enables SPI as master (clock = F_CPU/16)
void spi_init(void) {
	// PB1 = button
	// PB2 = D/!C
	// PD3 = CS CAN, PB3 = CS IO, PB4 = CS DISP
	// PB5 = MOSI, PB6 = MISO, PB7 = SCK

	// Outputs: D/C, the two chip selects on port B, MOSI and SCK
	DDRB = (1 << DDB2) | (1 << DDB3) | (1 << DDB4) | (1 << DDB5) | (1 << DDB7);
	
	// Inputs: MISO and the joytick button
	DDRB &= ~ (1 << DDB6);
	DDRB &= ~ (1 << DDB1);

	// Chip select for the CAN controller
	DDRD |= (1 << DDD3);

  	spi_deselect();

  	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0); // Enable SPI, master mode, clockrate = fck/16. DORD = 0 -> MSB is sent first
}

// Sets all chip selects high, so no device is selected.
void spi_deselect() {

  PORTD |= (1 << PD3);  // CAN
  PORTB |= (1 << PB2) | (1 << PB3) | (1 << PB4); // IO board, OLED
}


// Selects one device by pulling its chip select low (DISP, IO or CAN)
void spi_select_slave(int n) {
  switch (n) {
	case DISP: 				
		PORTB &= ~(1 << PB4);
		break;
	case IO: 				
		PORTB &= ~(1 << PB3);
		break;
	case CAN: 				
		PORTD &= ~(1 << PD3);    
		break;
	default: 				// unknown device: select nothing
		break;
  }
}

// Sends one byte and returns the byte received at the same time.
uint8_t spi_transfer(uint8_t data) {

  SPDR = data; 							// Start transmission
  while (!(SPSR & (1 << SPIF))) {}; 	// Wait for transmission complete
  return SPDR;
}


// Sends n bytes from a buffer. Not used in the final game.
void spi_write_n_bytes(const uint8_t *data, uint16_t n) {
  for (uint16_t i = 0; i < n; i++) {
    spi_transfer(data[i]);
  }
}

// Reads n bytes into a buffer by sending dummy bytes. Not used in the final game.
void spi_read_n_bytes(uint8_t *buffer, uint16_t n) {
  for (uint16_t i = 0; i < n; i++) {
    buffer[i] = spi_transfer(0xFF);
  }
}
