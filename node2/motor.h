#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

void motor_init(void);
void motor_set_difficulty(uint8_t level);
void motor_set(int8_t u);                       
int8_t pid_regulator(int32_t pos, int32_t ref);
void motor_on_tick(void);                       
void motor_update_joystick(int8_t new_joystick);
void motor_update_touch(int8_t percent);
void motor_home(void);                         

#endif