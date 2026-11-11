#ifndef F_CPU
#define F_CPU 4915200UL
#endif

#include "io_driver.h"
#include "spi_driver.h"
#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/pgmspace.h>     
#include <util/delay.h>
#include <math.h>


#define ADC_ADDRESS 0x1000
#define CH_JOYSTICK_X 2
#define CH_JOYSTICK_Y 3
#define CH_TOUCH_X 1
#define CH_TOUCH_Y 0

// Joystick limits measured on the ADC
#define JOY_RAW_MIN     69
#define JOY_RAW_MAX     246


// IO board SPI commands
#define IO_CMD_BUTTONS  0x04
#define IO_CMD_LED      0x05

volatile char *adc = (char*)ADC_ADDRESS;

static uint8_t center_x;
static uint8_t center_y;

// Button states from the last call to io_read_buttons()
static uint8_t buttons_right;   // [0 0 R6 R5 R4 R3 R2 R1]
static uint8_t buttons_left;    // [0 L7 L6 L5 L4 L3 L2 L1]
static uint8_t buttons_nav;     // [0 0 0 Up Down Left Right Btn]


// JOYSTICK AND TOUCHPAD SECTION

// Starts a conversion, waits for BUSY (PD4) and reads all four ADC channels.
void adc_read_all_channels(uint8_t *results) {
	
	*adc = 0x00;                   	// Any write starts a conversion

	while (PIND & (1 << PD4)) {}	// Wait for BUSY to go low

	while (!(PIND & (1 << PD4))) {} // And high again: conversion done!
	
	for (uint8_t i = 0; i < 4; i++) {
		results[i] = *adc;    		 // each read returns the next channel
	}

}

// Measures the joystick rest position. The stick must not be touched while this runs.
void joystick_calibrate(void) {

	//Reads values from adc
	uint8_t results[4];
	adc_read_all_channels(results);

	//Fetches joystick values from adc values
	center_x = results[CH_JOYSTICK_X];
	center_y = results[CH_JOYSTICK_Y];

	// printf_P(PSTR("center x=%d y=%d\r\n"), center_x, center_y);
}


// Converts a raw ADC value to -100% to 100% relative to the calibrated center
int joystickpos_to_percentage(uint8_t raw, uint8_t center)
{
	int result;
	if (raw >= center) {
		result = (int16_t)(raw-center)*100/(JOY_RAW_MAX-center);
	}

	else result = -(int16_t)(center-raw)*100/(center-JOY_RAW_MIN);
	
	//Check if valid percentage
	if (result > 100) {
		result = 100;
	}

    if (result < -100) {
		result = -100;
	}

    return result;
}

// Returns the joystick position as -100..100 %
JoystickPosition joystick_get_position(void) {

	uint8_t results[4];
	adc_read_all_channels(results);

	JoystickPosition pos;
	pos.x = joystickpos_to_percentage(results[CH_JOYSTICK_X], center_x);
	pos.y = joystickpos_to_percentage(results[CH_JOYSTICK_Y], center_y);
	return pos;
}



// Converts a raw touch pad ADC value (0-255) to -100..100 %
int16_t touchpadpos_to_percentage(uint8_t raw)
{
    return (int16_t)raw * 40 / 51 - 100;   // 200/255 = 40/51
}

// Returns the touchpad position as -100..100 % in x and y.
TouchPosition touchpad_get_position(void) {

	uint8_t results[4];
	adc_read_all_channels(results);

	TouchPosition pos;
	pos.x = touchpadpos_to_percentage(results[CH_TOUCH_Y]);
	pos.y = touchpadpos_to_percentage(results[CH_TOUCH_X]);
	return pos;
}




// BUTTONS SECTION
//Note: Button SR3 does not work (always registered as pressed)


// Reads all buttons from the IO board and stores them in buttons_right/left/nav
void io_read_buttons(void) {
    spi_select_slave(IO);
    spi_transfer(0x04);    // Command for reading buttons
    _delay_us(40);
    buttons_right = spi_transfer(0x00);  // Dummy bytes clock out the answer
    _delay_us(2);
    buttons_left  = spi_transfer(0x00);
    _delay_us(2);
    buttons_nav   = spi_transfer(0x00);
    spi_deselect();
}



// Returns 1 if the solenoid button (R6) is pressed, otherwise 0.
uint8_t solenoid_button_pressed(void)
{
    io_read_buttons();
    return (buttons_right >> 5) & 1;    // bit 5 = R6
}


// Reads the navigation buttons and returns the direction used by the menu. If several are pressed, the one checked first wins.
MenuJoystickDirection get_menu_joystick_direction(void) {
	io_read_buttons();

	if ((buttons_nav >> 4) & 1) {
		return MENU_UP;
	}

    if ((buttons_nav >> 3) & 1) {
		return MENU_LEFT;
	}
    if ((buttons_nav >> 2) & 1) {
		return MENU_DOWN;

	}
    if ((buttons_nav >> 1) & 1) {
		return MENU_RIGHT;
	}

    if ((buttons_nav >> 0) & 1) {
		return MENU_BUTTON;

	}

    return MENU_NEUTRAL;
}


// NOT USED IN FINAL GAME, MADE FOR EXCERCISES

// Returns the direction the joystick is pushed (or NEUTRAL near the center).
// Made for exercise 3 (joystick direction) and used by the first version of the OLED menu in exercise 4.
JoystickDirection joystick_get_direction(void) {
	JoystickPosition pos = joystick_get_position();
	const int8_t threshold = 30;
	if (pos.x > threshold) {
		return UP;
	}
	if (pos.x < -threshold) {
		return DOWN;
	}
	if (pos.y > threshold) {
		return RIGHT;
	}
	if (pos.y < -threshold) {
		return LEFT;
	}

	return NEUTRAL;
} 


void light_correct_lights(void) {
	io_read_buttons();
	if ((buttons_right & (1 << 5)) != 0) {
		io_led_set(0, 1);
	}
	io_led_set(1, buttons_left);
}

// Turns LED number 0-5 on the IO board on (on != 0) or off.
void io_led_set(uint8_t led_n, uint8_t on) {
    spi_select_slave(IO);
    spi_transfer(0x05);   // Command: LED on/off
    spi_transfer(led_n);  // Which LED (0-5)
    spi_transfer(on);     // 0 = off, else = on
    spi_deselect();
}


// Turns off all LEDs
void button_init(void) {
	for (uint8_t i = 0; i < 6; i++) {
		io_led_set(i, 0);
	}

}
