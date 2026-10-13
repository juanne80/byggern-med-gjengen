#include "io_driver.h"
#include "spi_driver.h"
#include <avr/io.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

static uint8_t center_x;
static uint8_t center_y;

volatile char *adc = (char*)0x1000;

#define CH_JOYSTICK_X 2
#define CH_JOYSTICK_Y 3
#define CH_TOUCH_X 1
#define CH_TOUCH_Y 0



void adc_start_conversion() {
	*adc = 0x00;
}

void adc_read_all_channels(uint8_t *results) {
	adc_start_conversion();
	while (PIND & (1 << PD4)) { }
	while (!(PIND & (1 << PD4))) {
		//Wait
	}
	for (uint8_t i = 0; i < 4; i++) {
		results[i] = *adc;
	}

}



void joystick_calibrate() {
	//joy x = 2,56
	//joy y = 2.54
	uint8_t results[4];
	adc_read_all_channels(results);
	center_x = results[CH_JOYSTICK_X];
	center_y = results[CH_JOYSTICK_Y];
	printf("center x: %d\n", center_x);
		printf("center y: %d\n", center_y);
}



JoystickPosition joystick_get_position() {
	uint8_t results[4];
	adc_read_all_channels(results);
	JoystickPosition pos;
	pos.x = joystickpos_to_percentage(results[CH_JOYSTICK_X], center_x);
	pos.y = joystickpos_to_percentage(results[CH_JOYSTICK_Y], center_y);
	return pos;
}

JoystickDirection joystick_get_direction() {
	JoystickPosition pos = joystick_get_position();
	const int8_t threshold = 30;
	if (pos.x > threshold) return UP;
	if (pos.x < -threshold) return DOWN;
	if (pos.y > threshold) return RIGHT;
	if (pos.y < -threshold) return LEFT;
	return NEUTRAL;
} 


int joystickpos_to_percentage(uint8_t raw, uint8_t center)
{
	int result;
	if (raw >= center) {
		result = (int16_t)(raw-center)*100/(246-center);
	}

	else result = -(int16_t)(center-raw)*100/(center-69);
	if (result > 100)  result = 100;
    if (result < -100) result = -100;
    return result;
}

int touchpadpos_to_percentage(uint8_t voltage)
{
	return round(200/5 * (voltage - 2.5));
}

const char* joystick_direction_to_string(JoystickDirection dir)
{
    switch (dir) {
        case LEFT:    return "LEFT";
        case RIGHT:   return "RIGHT";
        case UP:      return "UP";
        case DOWN:    return "DOWN";
        case NEUTRAL: return "NEUTRAL";
        default:      return "UKJENT";
    }
}

uint8_t buttons_right, buttons_left, buttons_nav;

void io_read_buttons() {
    spi_select_slave(IO);
    spi_transfer(0x04);                    // kommando: les knapper
    buttons_right = spi_transfer(0x00);    // dummy-byte for å klokke ut svaret
    buttons_left  = spi_transfer(0x00);
    buttons_nav   = spi_transfer(0x00);
    spi_deselect();
}

uint8_t sr6_pressed() {
    return (buttons_right & (1 << 5)) != 0;   // R6 er bit 5
}

void io_led_set(uint8_t led_n, uint8_t on) {
    spi_select_slave(IO);
    spi_transfer(0x05);   // kommando: LED on/off
    spi_transfer(led_n);  // hvilken LED (0-5)
    spi_transfer(on);     // 0 = av, alt annet = på
    spi_deselect();
}

void light_correct_lights() {
	io_read_buttons();
	if ((buttons_right & (1 << 5)) != 0) {
		io_led_set(0, 1);
	}
	io_led_set(1, buttons_left);
}

void button_init() {
	for (uint8_t i = 0; i < 6; i++) {
		io_led_set(i, 0);
	}

}

//Knapp SR3 fungerer ikke ("holdt inne" hele tiden)