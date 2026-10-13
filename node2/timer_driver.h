#include <stdint.h>
#include <stdio.h>


void timer_init();
void TC0_Handler(void);
void tc_pwm_init(void);
void tc_pwm_set_pulse_us(uint32_t pulse_us);