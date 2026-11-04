#ifndef TIMER_DRIVER_H
#define TIMER_DRIVER_H
 
#include <stdint.h>
 
extern volatile uint32_t timer_ticks;    
 
void timer_init(void);
void TC0_Handler(void);
 
void tc_pwm_init(void);
void tc_pwm_set_pulse_us(uint32_t pulse_us);
 
#endif
