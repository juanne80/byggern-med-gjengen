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
