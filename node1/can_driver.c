#include "can_driver.h"
#include <stdint.h>
#include <stdio.h>
#include "can_controller_driver.h"
#include <util/delay.h>
#include "mcp2515.h"

uint8_t can_init(uint8_t mode) { //Returner 0 hvis alt gikk bra, 1 hvis noe gikk galt
    can_controller_reset();
    _delay_ms(1);

    uint8_t value = can_controller_read(MCP_CANSTAT);   // Henter ut verdi i CANSTAT
    if ((value & MODE_MASK) != MODE_CONFIG) {  //Sjekker at OPMOD2 = 1, mens OPMOD1 og OPMOD0 = 0 (Tilsvarer configuration mode)
        return 1;
    }


    can_controller_write(MCP_RXB0CTRL, MCP_RXM_ANY); // RXB0CTR: RXM = 11, ved reset skrus filteret på, men vi ønsker å motta alle meldinger

    can_controller_write(MCP_CANINTE, MCP_RX0IE); // CANINTE: RX0IE = 1, gir interrupt på interrupt-pinnen når RXB0 får en melding 

	//Bit timing, sjekk mcp2515.h
	can_controller_write(MCP_CNF1, CAN_CNF1);
	can_controller_write(MCP_CNF2, CAN_CNF2);
	can_controller_write(MCP_CNF3, CAN_CNF3);

    can_controller_bit_modify(MCP_CANCTRL, MODE_MASK, mode); // setter modus

    // Sjekker at modusbyttet faktisk skjedde (samme greie som i starten)
    value = can_controller_read(MCP_CANSTAT);
    if ((value & MODE_MASK) != mode) {
        return 1;
    }

    return 0;
}



uint8_t can_send(can_message *msg) {

    uint16_t timeout = 1000;
	while (can_controller_read_status() & MCP_STAT_TX0REQ) { // Sjekker TXREQ for TXB0, har her valgt å kun sende til TXB0 - MULIG FEILKILDE SENERE
		if (--timeout == 0) return 1;
	}

    can_controller_write(MCP_TXB0SIDH, msg->id >> 3);          // TXB0SIDH settes til de 8 mest signifikante bitene av ID-en
    can_controller_write(MCP_TXB0SIDL, (msg->id & 0x07) << 5); // TXB0SIDL settes til de 3 minst signifikante bitene av ID-en, plassert i de 3 mest signifikante bitene av registeret

    can_controller_write(MCP_TXB0DLC, msg->length); // TXB0DLC settes til lengden på meldingen (0-8)


    for (uint8_t i = 0; i < msg->length; i++) {
        can_controller_write(MCP_TXB0D0 + i, msg->data[i]);  // TXB0D0..D7
    }

    can_controller_request_to_send(0);
    return 0;
}


uint8_t can_receive(can_message *msg) { //Returnerer 0 om ingen meldinger, 1 når mottatt
    if (!(can_controller_read_status() & (MCP_STAT_RX0IF))) { //Sjekker om det er en melding i RXB0, hvis ikke returneres 0
        return 0;  
    }

    uint8_t sidh = can_controller_read(MCP_RXB0SIDH);   // RXB0SIDH - øverste del av id
    uint8_t sidl = can_controller_read(MCP_RXB0SIDL);   // RXB0SIDL - nederste del av id
    msg->id = (sidh << 3) | (sidl >> 5);

    msg->length = can_controller_read(MCP_RXB0DLC) & MCP_DLC_MASK;   // RXB0DLC
    if (msg->length > CAN_MAX_DATA_LEN) {
		msg->length = CAN_MAX_DATA_LEN;
	} 

    for (uint8_t i = 0; i < msg->length; i++) {
        msg->data[i] = can_controller_read(MCP_RXB0D0 + i);  // RXB0D0..D7
    }

    can_controller_bit_modify(MCP_CANINTF, MCP_RX0IF, 0x00); // Si fra at RXB0 er lest og ledig igjen

    return 1; 
}

