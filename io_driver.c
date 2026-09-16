#include "io_driver.h"
#include <avr/io.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>


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

uint8_t center_x, center_y;

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
	const int8_t threshold = 10;
	if (pos.x > threshold) return RIGHT;
	if (pos.x < -threshold) return LEFT;
	if (pos.y > threshold) return UP;
	if (pos.y < -threshold) return DOWN;
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
