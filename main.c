#include "driver.h"
#include "io_driver.h"
#include "oled.h"
#include "spi_driver.h"
#include "sram_test.h"
#include <avr/io.h>
#include <util/delay.h>

#include <stdio.h>
#include "oled_menu.h"

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

  JoystickDirection joystickdirection;
  JoystickDirection lastjoystickdirection;
  lastjoystickdirection = NEUTRAL;

  while(1) {

	light_correct_lights();

	lastjoystickdirection = joystickdirection;
	joystickdirection = joystick_get_direction();
	printf(joystick_direction_to_string(joystickdirection));
	if (lastjoystickdirection != joystickdirection) {
		menu_handle_input(joystickdirection);
	}
	_delay_ms(100);

  }

  // while (1)
  // {
  // 	JoystickPosition pos = joystick_get_position();
  // 	printf("X: %d, Y: %d\n", pos.x, pos.y);
  // 	_delay_ms(5000);

  // }

  //
  // while (1)
  // {
  //    *(volatile uint8_t *)0x1800 = 0x0;
  // }
  // SRAM_test();
  //
}

// sudo picocom -b 9600 /dev/ttyS4
// EXIT: ctrl X
