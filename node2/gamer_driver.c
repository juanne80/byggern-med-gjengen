#include "sam.h"
#include "../node1/io_driver.h"
#include "can.h"
#include "gamer_driver.h"
#include "ir_driver.h"
#include "timer_driver.h"
#include "motor.h"
#include "encoder.h"
#include "solenoid.h"


// Innstillinger mottatt fra node 1
uint8_t setting_lives      = 2;
uint8_t setting_player     = 7;
uint8_t setting_controller = 0;
uint8_t setting_difficulty = 0;

// spillstatus
static uint16_t ir_counter     = 0;   // antall mal (ganger strålen er brutt)
static uint8_t  previous_state = 1;   // 1 = ikke blokkert
static uint32_t last_goal_tick = 0;
 
static uint8_t  lives_left     = 0;
static uint8_t  game_running   = 0;

static uint32_t start_tick = 0;    // timer_ticks da spillet startet
static uint16_t final_time = 0;    // sluttid i sekunder, settes ved game over


// Kalles nar start-meldingen kommer fra node 1
void game_start(uint8_t lives, uint8_t player, uint8_t controller, uint8_t difficulty)
{
    setting_lives      = lives;
    setting_player     = player;
    setting_controller = controller;
    setting_difficulty = difficulty;
 
    ir_counter     = 0;
    previous_state = 1;
    last_goal_tick = timer_ticks;
    lives_left     = lives + 1;           // indeks 0 betyr 1 liv
 
	motor_set_difficulty(difficulty);
    motor_home();
	start_tick   = timer_ticks;
    final_time   = 0;                         // vognen til start, regulatoren nullstilt
    game_running = 1;
}


void check_ir_beam(uint16_t ir_value){
	if (ir_value < 100 && previous_state==1){
		previous_state = 0;

		if (timer_ticks-last_goal_tick >= 200){ //100 ticks er 1 s
			ir_counter++;
			last_goal_tick = timer_ticks;

			if (game_running && setting_lives != 5) { // 5 = uendelig
                if (lives_left > 0 && !solenoid_active() ) {
					lives_left--;
				}
                if (lives_left == 0) {
					game_running = 0;   // game over
					final_time = (timer_ticks - start_tick) / 100;   // 100 ticks = 1 s
				}
            }
		}
	}
	else if (ir_value >= 1000){
		previous_state = 1;
	}
}

uint16_t get_ir_counter(void){
	return ir_counter;
}

uint8_t  get_lives_left(void){ 
	return lives_left; 
}

uint8_t  game_is_running(void){ 
	return game_running; 
}

uint16_t get_game_time(void)
{
    if (game_running) {
        return (timer_ticks - start_tick) / 100;
    }
    return final_time;
}
