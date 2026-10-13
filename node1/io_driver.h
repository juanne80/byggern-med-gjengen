#ifndef IO_DRIVER_H
#define IO_DRIVER_H

#include <stdint.h>



typedef struct {int8_t x; int8_t y;} JoystickPosition;
typedef enum {LEFT, RIGHT, UP, DOWN, NEUTRAL} JoystickDirection;


int joystickpos_to_percentage(uint8_t raw, uint8_t center);
int touchpadpos_to_percentage(uint8_t voltage);
void adc_start_conversion();
void adc_read_all_channels(uint8_t *results);
void joystick_calibrate();
JoystickDirection joystick_get_direction();
JoystickPosition joystick_get_position();
const char* joystick_direction_to_string(JoystickDirection dir);
void io_read_buttons() ;
uint8_t sr6_pressed();
void io_led_set(uint8_t led_n, uint8_t on);
void light_correct_lights();
void button_init();

#endif