#ifndef SPI_DRIVER_H
#define SPI_DRIVER_H

#include <stdint.h>

#define DISP 0
#define IO 1
#define CAN 2

void spi_init(void);
void spi_deselect(void);
void spi_select_slave(int n);
uint8_t spi_transfer(uint8_t data);
void spi_write_n_bytes(const uint8_t *data, uint16_t n);
void spi_read_n_bytes(uint8_t *buffer, uint16_t n);

#endif
