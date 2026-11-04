#include "sam.h"
#include "gamer_driver.h"
#include "timer_driver.h"
#include "motor.h"
#include "solenoid.h"



// Settings received from node 1. See oled_menu.c for details
uint8_t setting_lives      = 2;
uint8_t setting_player     = 7;
uint8_t setting_controller = 0;
uint8_t setting_difficulty = 0;

// Game state
static uint16_t ir_counter     = 0;    // number of goals (times the beam was broken)
static uint8_t  previous_state = 1;    // 1 = beam free, 0 = beam broken
static uint32_t last_goal_tick = 0;    // timer_ticks at the last goal
static uint8_t  lives_left     = 0;
static uint8_t  game_running   = 0;
 
static uint32_t start_tick = 0;        // timer_ticks when the game started
static uint16_t final_time = 0;        // game time in seconds, set at game over



// Starts a new game with the settings from node 1. Called when the start message arrives

void game_start(uint8_t lives, uint8_t player, uint8_t controller, uint8_t difficulty)
{
    setting_lives      = lives;
    setting_player     = player;
    setting_controller = controller;
    setting_difficulty = difficulty;
 
    ir_counter     = 0;
    previous_state = 1;
    last_goal_tick = timer_ticks;
    lives_left     = lives + 1;           
 
	motor_set_difficulty(difficulty);
    motor_home();			// carriage to start, controller reset

	start_tick   = timer_ticks;
    final_time   = 0;                         
    game_running = 1;
}

// Checks the IR beam and counts a goal when it is broken. Called continuously from main with the latest ADC value
void check_ir_beam(uint16_t ir_value){
	if (ir_value < 100 && previous_state==1){ 		// 100 = threshold for beam broken, found by testing
		previous_state = 0;

		if (timer_ticks-last_goal_tick >= 200){ //200 ticks is 2 s
			ir_counter++;
			last_goal_tick = timer_ticks;

			if (game_running && setting_lives != 5) { // 5 = infinite
                if (lives_left > 0 && !solenoid_active() ) { // No life is lost while the solenoid is out, since the relay can cause interference
					lives_left--;
				}
                if (lives_left == 0) {
					game_running = 0;   // game over
					final_time = (timer_ticks - start_tick) / 100; 
				}
            }
		}
	}
	else if (ir_value >= 1000){
		previous_state = 1;
	}
}

// Returns the number of goals since the game started
uint16_t get_ir_counter(void){
	return ir_counter;
}

uint8_t  get_lives_left(void){ 
	return lives_left; 
}

uint8_t  game_is_running(void){ 
	return game_running; 
}

// Returns the game time in seconds: counting while playing, frozen after game over
uint16_t get_game_time(void)
{
    if (game_running) {
        return (timer_ticks - start_tick) / 100;
    }
    return final_time;
}
