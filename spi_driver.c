#include "spi_driver.h"
#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>

void spi_init(void) {
  // PB1 = button
  // PB2 = D/!C, PB3 = CS IO, PB4 = CS DISP
  // PB5 = MOSI, PB6 = MISO, PB7 = SCK

  DDRB = (1 << DDB2) | (1 << DDB3) | (1 << DDB4) | (1 << DDB5) |
         (1 << DDB7); // Set outputs
	DDRB &= ~ (1 << DDB6);
	DDRB &= ~ (1 << DDB1);

  spi_deselect();

  SPCR =
      (1 << SPE) | (1 << MSTR) | (1 << SPR0); // Enable SPI, master, clockrate =
                                              // fck/16 (må kanskje endres på)
                                              // DORD=0 -> MSB sendes først
}

void spi_deselect() {
  PORTB |=
      (1 << PB2) | (1 << PB3) |
      (1 << PB4); // Set io and disp CS and D/!C to 1 before SPI is activated
  // Set can cs high
}

void spi_select_slave(int n) {
  //spi_deselect();

  switch (n) {
  case 0: // Display
    PORTB &= ~(1 << PB4);
    break;
  case 1: // IO
    PORTB &= ~(1 << PB3);
    break;
  case 2: // CAN
    // PORTD &= ~(1 << PD4); //MÅÅÅ settes til rett register
    break;
  default:
    // ugyldig slavenummer – ingen slave velges
    break;
  }
}

uint8_t spi_transfer(uint8_t data) {
  SPDR = data; // Start transmission
  while (!(SPSR & (1 << SPIF))) {
  }; // Wait for transmission complete
  return SPDR;
}

void spi_write_n_bytes(const uint8_t *data, uint16_t n) {
  for (uint16_t i = 0; i < n; i++) {
    spi_transfer(data[i]);
  }
}

void spi_read_n_bytes(uint8_t *buffer, uint16_t n) {
  for (uint16_t i = 0; i < n; i++) {
    buffer[i] = spi_transfer(0xFF);
  }
}
