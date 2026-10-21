#include "sam.h"
#include "../node1/io_driver.h"
#include "can.h"
#include "gamer_driver.h"
#include "ir_driver.h"
#include "timer_driver.h"
#include "motor.h"
#include "encoder.h"


uint16_t ir_counter = 0;
uint8_t previous_state = 1; //1 = ikke blokkert
uint32_t last_goal_tick = 0;


void check_ir_beam(uint16_t ir_value){
	if (ir_value < 1000 && previous_state==1){
		previous_state = 0;
		if (timer_ticks-last_goal_tick >= 100){ //100 ticks er 1 s
			ir_counter++;
			last_goal_tick = timer_ticks;
		}
	}
	else if (ir_value >= 1000){
		previous_state = 1;
	}
}

uint16_t get_ir_counter(void){
	return ir_counter;
}

void start_game(void) {
	int32_t last_encoder_value = 1000;
	int32_t encoder_value = encoder_read();
	motor_set(50);
	while (last_encoder_value != encoder_value) {
		last_encoder_value = encoder_value;
		encoder_value = encoder_read();
		for (volatile uint32_t i = 0; i < 1200000; i++) { }
	}
	motor_set(0);
	encoder_reset();
}

