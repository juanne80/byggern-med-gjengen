#include <stdint.h>
#include "spi_driver.h"
#include "can_controller_driver.h"
#include "mcp2515.h"

// Reads one register from the MCP2515 and returns its value
uint8_t can_controller_read(uint8_t address) {
	spi_select_slave(CAN);
	spi_transfer(MCP_READ);
	spi_transfer(address);
	uint8_t data = spi_transfer(0x00);
	spi_deselect();
	return data;
}

// Writes one byte to a register in the MCP2515
void can_controller_write(uint8_t address, uint8_t data) {
	spi_select_slave(CAN);
	spi_transfer(MCP_WRITE);
	spi_transfer(address);
	spi_transfer(data);
	spi_deselect();
}

// Requests transmission of the message in transmit buffer 0, 1 or 2
void can_controller_request_to_send(uint8_t buffer_number) { 
	spi_select_slave(CAN);
	spi_transfer(MCP_RTS | (1 << buffer_number));
	spi_deselect();
}


// Reads the quick status byte of the MCP2515
// Bit 0: RX0IF - message received in RXB0
// Bit 1: RX1IF - message received in RXB1
// Bit 2: TX0REQ - TXB0 is waiting to be sent
// Bit 3: TX0IF - TXB0 has been sent
// Bit 4: TX1REQ - TXB1 is waiting to be sent
// Bit 5: TX1IF - TXB1 has been sent
// Bit 6: TX2REQ - TXB2 is waiting to be sent
// Bit 7: TX2IF - TXB2 has been sent
uint8_t can_controller_read_status(void) {
	spi_select_slave(CAN);
	spi_transfer(MCP_READ_STATUS);
	uint8_t status = spi_transfer(0x00);
	spi_deselect();
	return status;
}

// Changes only the bits selected by mask in a register, leaving the others untouched
void can_controller_bit_modify(uint8_t address, uint8_t mask, uint8_t data) {
	spi_select_slave(CAN);
	spi_transfer(MCP_BITMOD);
	spi_transfer(address);
	spi_transfer(mask);
	spi_transfer(data);
	spi_deselect();
}

// Resets the MCP2515, afterwards it is in configuration mode
void can_controller_reset(void) {
	spi_select_slave(CAN);
	spi_transfer(MCP_RESET);
	spi_deselect();
}

