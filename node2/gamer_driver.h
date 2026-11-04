#ifndef GAMER_DRIVER_H
#define GAMER_DRIVER_H

#include <stdint.h>

// Innstillinger fra node 1
extern uint8_t setting_lives;
extern uint8_t setting_player;
extern uint8_t setting_controller;
extern uint8_t setting_difficulty;

void     game_start(uint8_t lives, uint8_t player, uint8_t controller, uint8_t difficulty);
void     check_ir_beam(uint16_t ir_value);

uint16_t get_ir_counter(void);
uint8_t  get_lives_left(void);
uint8_t  game_is_running(void);
uint16_t get_game_time(void);

#endif