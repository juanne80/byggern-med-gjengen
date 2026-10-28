#include "driver.h"
#include "io_driver.h"
#include "oled.h"
#include "spi_driver.h"
#include "sram_test.h"
#include <avr/io.h>
#include <util/delay.h>

#include <stdio.h>
#include "oled_menu.h"
#include "can_driver.h"
#include "can_controller_driver.h"
#include "mcp2515.h"

#define FOSC 4915200UL
#define BAUD 9600
#define MYUBRR (FOSC / 16 / BAUD - 1)

int main(void) {
  USART_Init(MYUBRR);

  fdevopen(uart_putchar, uart_getchar);

  // Enable external memory interface
  MCUCR |= (1 << SRE);
  SFIOR &= ~(1 << XMM0);
  SFIOR &= ~(1 << XMM1);
  SFIOR |= (1 << XMM2);

  // Set up clock signal generation to ADC
  DDRD |= (1 << PD5);                   // Set PDX as output
  TCCR1A |= (1 << COM1A0);              // Toggle OC1A on Compare Match
  TCCR1B |= (1 << WGM12) | (1 << CS10); // No prescaling (table 57)
  OCR1A = 0;

  // Reading from ADC

  spi_init();
  oled_init();
  menu_init();
  joystick_calibrate();
  button_init();


    //initialisering
    if (can_init(MODE_NORMAL) != 0) {
        printf("can_init feilet \r\n");
        while (1);   // ikke gjør mer...
    }
    printf("can_init gikk bra \r\n");

	
	//Sjekk at bit timing ble rett
    printf("CNF1=0x%02X CNF2=0x%02X CNF3=0x%02X\r\n",can_controller_read(MCP_CNF1), can_controller_read(MCP_CNF2), can_controller_read(MCP_CNF3));
 
    
	//Sjekk modus. OPMOD er bit 7-5 i CANSTAT: 000 = normal, 010 = loopback, 100 = configuration.
    uint8_t opmod = can_controller_read(MCP_CANSTAT) >> 5;
    printf("OPMOD=%d (%s)\r\n", opmod, opmod == 0 ? "normal" : "IKKE normal mode!");
	

    can_message tx = {
        .id = 0x123,
        .length = 3,
        .data = {1, 2, 3}
    };
    can_message rx;

	MenuJoystickDirection menujoystickdirection;
  	MenuJoystickDirection lastmenujoystickdirection;
  	lastmenujoystickdirection = MENU_NEUTRAL;
 
    while (1) {
        // Sende til node 2
		JoystickPosition pos = joystick_get_position();
		
		tx.data[0] = pos.x;
		tx.data[1] = pos.y;
		tx.data[2] = solenoid_button_pressed();

		can_send(&tx);

		lastmenujoystickdirection = menujoystickdirection;
		menujoystickdirection = get_menu_joystick_direction();
		if (lastmenujoystickdirection != menujoystickdirection) {
			menu_handle_input(menujoystickdirection);
		}
 
        _delay_ms(20); 
 
 
    }
	
}

// sudo picocom -b 9600 /dev/ttyS4
// EXIT: ctrl X
