#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>
#include "spi_driver.h"
#include "can_controller_driver.h"
#include "mcp2515.h"

uint8_t can_controller_read(uint8_t address) {
	spi_select_slave(CAN);
	spi_transfer(MCP_READ);
	spi_transfer(address);
	uint8_t data = spi_transfer(0x00);
	spi_deselect();
	return data;
}


void can_controller_write(uint8_t address, uint8_t data) {
	spi_select_slave(CAN);
	spi_transfer(MCP_WRITE);
	spi_transfer(address);
	spi_transfer(data);
	spi_deselect();
}


void can_controller_request_to_send(uint8_t buffer_number) { //register 0, 1 or 2
	spi_select_slave(CAN);
	spi_transfer(MCP_RTS | (1 << buffer_number));
	spi_deselect();
}

// STATUS BITS:
// 0	RX0IF: melding mottatt i RXB0
// 1	RX1IF: melding mottatt i RXB1
// 2	TXREQ for TXB0: TXB0 venter/sender
// 3	TX0IF: TXB0 ferdig sendt
// 4	TXREQ for TXB1
// 5	TX1IF
// 6	TXREQ for TXB2
// 7	TX2IF

uint8_t can_controller_read_status(void) {
	spi_select_slave(CAN);
	spi_transfer(MCP_READ_STATUS);
	uint8_t status = spi_transfer(0x00);
	spi_deselect();
	return status;
}


void can_controller_bit_modify(uint8_t address, uint8_t mask, uint8_t data) {
	spi_select_slave(CAN);
	spi_transfer(MCP_BITMOD);
	spi_transfer(address);
	spi_transfer(mask);
	spi_transfer(data);
	spi_deselect();
}

void can_controller_reset(void) {
	spi_select_slave(CAN);
	spi_transfer(MCP_RESET);
	spi_deselect();
}

