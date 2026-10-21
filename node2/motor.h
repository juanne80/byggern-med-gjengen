#include <stdint.h>
#include <stdio.h>

void motor_init(void);
void motor_set(int8_t u);
int8_t pi_regulator(int32_t pos, int32_t ref);
void motor_on_tick(void);
void motor_update_joystick(int8_t new_joystick);