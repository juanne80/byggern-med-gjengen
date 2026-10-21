#include <stdint.h>
#include <stdio.h>


void pwm_init(void);
void pwm_set_pulse_us(uint32_t pulse_us);
void joystick_to_PWM(int8_t xpercentage);
