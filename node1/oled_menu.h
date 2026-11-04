#ifndef OLED_MENU_H
#define OLED_MENU_H

#include <stdint.h>
#include "io_driver.h"

// Settings chosen in the menu
extern uint8_t setting_lives;        
extern uint8_t setting_player;       
extern uint8_t setting_controller;   
extern uint8_t setting_difficulty;  
extern uint8_t menu_start_game_requested;
 
void menu_init(void);
void menu_draw(void);
void menu_handle_input(MenuJoystickDirection menujoystickdirection);
 
void menu_update_time(uint16_t seconds);
void menu_show_game_screen(uint8_t lives, uint16_t seconds);
void menu_show_game_over(uint16_t seconds);
 
#endif
 
