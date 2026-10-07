#include <stdint.h>
#include <stdio.h>

#define TXB0 0x81
#define TXB1 0x82
#define TXB2 0x84


uint8_t can_controller_read(uint8_t address);
void can_controller_write(uint8_t address, uint8_t data);
void can_controller_request_to_send(uint8_t buffer_number);
uint8_t can_controller_read_status(void);
void can_controller_bit_modify(uint8_t address, uint8_t mask, uint8_t data);
void can_controller_reset(void);

