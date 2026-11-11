#ifndef IO_DRIVER_H
#define IO_DRIVER_H

#include <stdint.h>

typedef struct {
    int8_t x;       
    int8_t y;
} JoystickPosition;

typedef struct {
	int16_t x;       
	int16_t y;
} TouchPosition;
 
// Joystick direction (exercise 3, not used in the final game)
typedef enum {
    LEFT,
    RIGHT,
    UP,
    DOWN,
    NEUTRAL
} JoystickDirection;
 
// Direction from the navigation buttons, used by the menu
typedef enum {
    MENU_LEFT,
    MENU_RIGHT,
    MENU_UP,
    MENU_DOWN,
    MENU_BUTTON,
    MENU_NEUTRAL
} MenuJoystickDirection;


void adc_read_all_channels(uint8_t *results);
void joystick_calibrate(void);
int joystickpos_to_percentage(uint8_t raw, uint8_t center);
JoystickPosition joystick_get_position(void);
int16_t touchpadpos_to_percentage(uint8_t voltage);
TouchPosition touchpad_get_position(void);
 
void io_read_buttons(void);
void io_led_set(uint8_t led_n, uint8_t on);
void button_init(void);
uint8_t solenoid_button_pressed(void);
MenuJoystickDirection get_menu_joystick_direction(void);
 
JoystickDirection joystick_get_direction(void);
void light_correct_lights(void);
 
#endif
