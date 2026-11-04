#ifndef F_CPU
#define F_CPU 4915200UL
#endif


#include "can_driver.h"
#include <stdint.h>
#include "can_controller_driver.h"
#include <util/delay.h>
#include "mcp2515.h"



// Resets and configures the MCP2515 (receive everything, bit timing), then switches to the given mode. Returns 0 on success, 1 on failure.
uint8_t can_init(uint8_t mode) { 
    
	can_controller_reset();
    _delay_ms(1);

    uint8_t value = can_controller_read(MCP_CANSTAT);   // Fetches the value in CANSTAT
    if ((value & MODE_MASK) != MODE_CONFIG) {  //Checks that OPMOD2 = 1 and OPMOD1 = OPMOD0 = 0 (configuration mode)
        return 1;
    }


    can_controller_write(MCP_RXB0CTRL, MCP_RXM_ANY); // RXB0CTR: RXM = 11, turn off filters/masks and accept all messages (when reseting the filter is turned on)

    can_controller_write(MCP_CANINTE, MCP_RX0IE); // CANINTE: RX0IE = 1, interrupt pin goes low when a message arrives in RXB0

    // Bit timing (values defined in mcp2515.h)
	can_controller_write(MCP_CNF1, CAN_CNF1);
	can_controller_write(MCP_CNF2, CAN_CNF2);
	can_controller_write(MCP_CNF3, CAN_CNF3);

	// Switch to the requested mode and check that it took effect
    can_controller_bit_modify(MCP_CANCTRL, MODE_MASK, mode);
    value = can_controller_read(MCP_CANSTAT);
    if ((value & MODE_MASK) != mode) {
        return 1;
    }

    return 0;
}


// Puts a message in TXB0 and requests transmission. Returns 0 on success, 1 if TXB0 is still busy with the previous message.
uint8_t can_send(can_message *msg) {

	// Wait until the previous message in TXB0 has been sent
    uint16_t timeout = 1000;
	while (can_controller_read_status() & MCP_STAT_TX0REQ) {
		if (--timeout == 0) {
			return 1;
		}
		
	}

	// 11-bit ID: upper 8 bits in SIDH, lower 3 bits in bit 7-5 of SIDL
    can_controller_write(MCP_TXB0SIDH, msg->id >> 3);         
    can_controller_write(MCP_TXB0SIDL, (msg->id & 0x07) << 5);

    // Data length (0-8) and data bytes
    can_controller_write(MCP_TXB0DLC, msg->length); 
    for (uint8_t i = 0; i < msg->length; i++) {
        can_controller_write(MCP_TXB0D0 + i, msg->data[i]);
    }

    can_controller_request_to_send(0);
    return 0;
}


// Reads a message from RXB0 if one has arrived. Returns 1 if a message was read, 0 if RXB0 was empty.
uint8_t can_receive(can_message *msg) { 
    
	//Checks if there is a message in RXB=, returns 0 if not
	if (!(can_controller_read_status() & (MCP_STAT_RX0IF))) {
        return 0;  
    }

	// Rebuild the 11-bit ID from SIDH and bit 7-5 of SIDL
    uint8_t sidh = can_controller_read(MCP_RXB0SIDH);   
    uint8_t sidl = can_controller_read(MCP_RXB0SIDL);  
    msg->id = (sidh << 3) | (sidl >> 5);

	// Data length, limited to 8 bytes
    msg->length = can_controller_read(MCP_RXB0DLC) & MCP_DLC_MASK;
    if (msg->length > CAN_MAX_DATA_LEN) {
		msg->length = CAN_MAX_DATA_LEN;
	} 

	//Read data bytes into message
    for (uint8_t i = 0; i < msg->length; i++) {
        msg->data[i] = can_controller_read(MCP_RXB0D0 + i); 
    }

	// Clear the flag so RXB0 can receive the next message
    can_controller_bit_modify(MCP_CANINTF, MCP_RX0IF, 0x00); 

    return 1; 
}

