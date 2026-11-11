#ifndef F_CPU
#define F_CPU 4915200UL
#endif


#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
 
#include "uart_driver.h"
#include "io_driver.h"
#include "oled.h"
#include "oled_menu.h"
#include "spi_driver.h"
#include "sram_test.h"
#include "can_driver.h"
#include "mcp2515.h"


#define BAUD 9600
#define MYUBRR (F_CPU / 16 / BAUD - 1)

#define CAN_ID_JOYSTICK 0x10
#define CAN_ID_SETTINGS 0x20
#define CAN_ID_STATUS 0x30

typedef enum { STATE_MENU, STATE_PLAYING, STATE_GAME_OVER } GameState;


int main(void) {

  USART_Init(MYUBRR);
  fdevopen(uart_putchar, uart_getchar);

  // Enable external memory interface (SRAM and ADC)
  MCUCR |= (1 << SRE);
  SFIOR &= ~(1 << XMM0);
  SFIOR &= ~(1 << XMM1);
  SFIOR |= (1 << XMM2);  // Release PC4-PC7 for JTAG

  // Clock for the external ADC
  DDRD |= (1 << PD5);                   // Set PDX as output
  TCCR1A |= (1 << COM1A0);              // Toggle OC1A on Compare Match
  TCCR1B |= (1 << WGM12) | (1 << CS10); // No prescaling (table 57)
  OCR1A = 0;


  spi_init();
  oled_init();
  menu_init();
  joystick_calibrate();
  button_init();


    if (can_init(MODE_NORMAL) != 0) {
    	printf_P(PSTR("can_init feilet\r\n"));        
		while (1);   // Dont do anything more
    }
	printf_P(PSTR("Node 1 klar\r\n"));
 
    
    can_message joystick_msg = {
        .id = CAN_ID_JOYSTICK,
        .length = 3,
        .data = {1, 2, 3}
    };
	can_message rx;


	GameState state = STATE_MENU;
	MenuJoystickDirection menujoystickdirection = MENU_NEUTRAL;
  	MenuJoystickDirection lastmenujoystickdirection = MENU_NEUTRAL;

	// What the game screen shows now, so it is only redrawn when something changes
	uint8_t  last_lives = 0xFF;
	uint16_t last_time = 0xFFFF;
	
	menu_draw();

	uint8_t active_controller = 0;   // controller node 2 is using (sent at the last game start)
 
    while (1) {

		//Send touchpad or joystick position to node 2 (dependent on chosen settings)
		if (active_controller == 1) {
			TouchPosition touch = touchpad_get_position();
			joystick_msg.data[0] = touch.y;        
			joystick_msg.data[1] = touch.x;        
		} else {
			JoystickPosition pos = joystick_get_position();
			joystick_msg.data[0] = pos.x;          
			joystick_msg.data[1] = pos.y;          
		}


        // Send solenoid button to node 2
		joystick_msg.data[2] = solenoid_button_pressed();
		can_send(&joystick_msg);

		// Read the navigation buttons, react only when they change
		lastmenujoystickdirection = menujoystickdirection;
		menujoystickdirection = get_menu_joystick_direction();
		uint8_t new_input = (lastmenujoystickdirection != menujoystickdirection);


		switch (state) {
			
		case STATE_MENU:
			while (can_receive(&rx)) { }  	
			if (new_input) {
					menu_handle_input(menujoystickdirection);
				}

			if (menu_start_game_requested) {

				menu_start_game_requested = 0;

                can_message settings_msg = {
                    .id     = CAN_ID_SETTINGS,
                    .length = 5,
                    .data   = {
                        1,                     // command: 1 = start game
                        setting_lives,         // 0-4 = 1-5 lives, 5 = infinite
                        setting_player,        // player
                        setting_controller,    // 0 = joystick, 1 = touch slider
                        setting_difficulty     // 0 = easy ... 3 = extreme
                    }
				};

				can_send(&settings_msg);
				active_controller = setting_controller;

				// Node 2 drives the slider to its start position before the game starts
				oled_reset();
				oled_pos(3, 0);
				oled_print("Starting...");
				_delay_ms(4000);                      // Waits for node 2 to slide the slider to start position

				while (can_receive(&rx)) { }          // Throw away old status messages

				// Draw the game screen with the chosen number of lives
				uint8_t start_lives;
				if (setting_lives == 5) {
					start_lives = 255;                // Infinite
				} else {
					start_lives = setting_lives + 1;  // Index 0 = 1 life
				}
				menu_show_game_screen(start_lives, 0);
				last_lives = start_lives;
				last_time  = 0;

				state = STATE_PLAYING;
			}
			break;

		case STATE_PLAYING:
			//Recieves updates from node 2
			while (can_receive(&rx)) {
				if (rx.id == CAN_ID_STATUS) {
					uint8_t  running = rx.data[0];
					uint8_t  lives   = rx.data[1];
					uint16_t time    = ((uint16_t)rx.data[2] << 8) | rx.data[3];

					if (!running) {
						menu_show_game_over(time);
						state = STATE_GAME_OVER;
					} else if (lives != last_lives) {
						menu_show_game_screen(lives, time);     // Whole screen when a life is lost
						last_lives = lives;
						last_time = time;
					} else if (time != last_time) {
						menu_update_time(time);   				// Only the time line every second
						last_time = time;
					}
				}
			}
			break;

		case STATE_GAME_OVER:
			while (can_receive(&rx)) {} // Empty buffer

			if (new_input && menujoystickdirection == MENU_BUTTON) {

				// Wait until the button is released and has stopped bouncing, so the same press is not seen as a new one in the menu
				while (get_menu_joystick_direction() == MENU_BUTTON) {}
				_delay_ms(200);

				menujoystickdirection     = MENU_NEUTRAL;
				lastmenujoystickdirection = MENU_NEUTRAL;

				state = STATE_MENU;
				menu_draw();
			}
			break;
		}

		_delay_ms(20);
	}
}

// sudo picocom -b 9600 /dev/ttyS4

