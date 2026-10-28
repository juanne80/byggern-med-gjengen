//Menu stuff
#include <stdint.h>
#include <avr/io.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include "oled_menu.h"
#include "io_driver.h"
#include "oled.h"


typedef struct Menu {
    const char* title;
    const char** items;        // tekstene som vises, f.eks. "Alternativ 1"
    uint8_t num_items;
    struct Menu** submenus;    // samme lengde som items. NULL = ingen undermeny for den linja
    struct Menu* parent;       // "forrige" meny (dit du går ved back)
} Menu;


// ======================= Settings -> undermenyer =======================
 
// Settings -> Number of lives
const char* num_of_lives_items[] = { "1", "2", "3", "4", "5", "Infinite" };
Menu num_of_lives = {
    .title = "Number of lives",
    .items = num_of_lives_items,
    .num_items = 6,
    .submenus = NULL,          // ingen undermenyer
    .parent = NULL
};
 
// Settings -> Choose player
const char* choose_player_items[] = { "Hanne", "Helene", "Ulrikke", "Studass",
                                      "Sensor", "Looser", "VIP Guest", "Guest" };
Menu choose_player = {
    .title = "Choose player",
    .items = choose_player_items,
    .num_items = 8,
    .submenus = NULL,
    .parent = NULL
};
 
// Settings -> Controller
const char* controller_items[] = { "Joystick", "Touch slider" };
Menu controller = {
    .title = "Controller",
    .items = controller_items,
    .num_items = 2,
    .submenus = NULL,
    .parent = NULL
};
 
// Settings -> Difficulty
const char* difficulty_items[] = { "Easy", "Medium", "Hard", "Extreme" };
Menu difficulty = {
    .title = "Difficulty",
    .items = difficulty_items,
    .num_items = 4,
    .submenus = NULL,
    .parent = NULL
};
 
 
// ============================== Settings ==============================
 
const char* settings_items[] = { "Number of lives", "Choose player", "Controller", "Difficulty" };
Menu* settings_submenus[4]   = { &num_of_lives,     &choose_player,  &controller,  &difficulty };
 
Menu settings = {
    .title = "Settings",
    .items = settings_items,
    .num_items = 4,
    .submenus = settings_submenus,
    .parent = NULL
};
 
 
// ========================= Previous highscores =========================
 
const char* prev_highscores_items[] = { "1. Ulrikke", "2. Hanne", "3. Helene", "4. Sensor" };
Menu prev_highscores = {
    .title = "Previous highscores",
    .items = prev_highscores_items,
    .num_items = 4,
    .submenus = NULL,
    .parent = NULL
};
 
 
// ============================== Main menu ==============================
 
const char* main_items[] = { "Start game", "Settings", "Previous highscores", "Calibrate" };
Menu* main_submenus[4]   = { NULL,         &settings,  &prev_highscores,      NULL };
 
Menu MainMenu = {
    .title = "Main menu",
    .items = main_items,
    .num_items = 4,
    .submenus = main_submenus,
    .parent = NULL             // toppmenyen har ingen forelder
};
 
 
// Kobler hver meny til menyen over seg (brukes av LEFT)
void menu_init(void) {
    settings.parent        = &MainMenu;
    prev_highscores.parent = &MainMenu;
 
    num_of_lives.parent    = &settings;
    choose_player.parent   = &settings;
    controller.parent      = &settings;
    difficulty.parent      = &settings;
}
 


Menu* current_menu = &MainMenu;
uint8_t selected = 0;


void menu_draw(void) {
    for (uint8_t i = 0; i < 8; i++) {
        oled_clear_line(i);
        if (i < current_menu->num_items) {
            oled_pos(i, 0);
            oled_print(i == selected ? ">" : " ");
            oled_print(current_menu->items[i]);
        }
    }
}


void menu_handle_input(MenuJoystickDirection menujoystickdirection) {

	switch (menujoystickdirection) {
		case MENU_UP:
			if (selected > 0) selected --;
			break;
		case MENU_DOWN:
			if (selected < current_menu->num_items - 1) selected++;
			break;
		case MENU_RIGHT:
			if (current_menu->submenus != NULL && current_menu->submenus[selected] != NULL) {
                current_menu = current_menu->submenus[selected];  // gå til "neste"
                selected = 0;
            }
			break;
		case MENU_LEFT:
            if (current_menu->parent != NULL) {
                current_menu = current_menu->parent;              // gå til "forrige"
                selected = 0;
            }
		//case MENU_BUTTON:
		break;
		deafult:
			break;
		}
    menu_draw();
}

