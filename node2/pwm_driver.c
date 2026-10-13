#include "pwm_driver.h"
#include "sam.h"
#include "../node1/io_driver.h"
#include "can.h"
#include "timer_driver.h"


void joystick_to_PWM(int8_t xpercentage){
	uint32_t dutyCycle = (uint32_t)(xpercentage*6 + 1500);

	tc_pwm_set_pulse_us(dutyCycle);
	
}
