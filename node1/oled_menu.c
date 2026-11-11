//Menu stuff
#include <stdint.h>
#include <avr/io.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include "oled_menu.h"
#include "io_driver.h"
#include "oled.h"


typedef struct Menu {
    const char** items;   // Menu option text strings
    uint8_t num_items;
    struct Menu** submenus;   // Same length as items, NULL = no submenu for that line
    struct Menu* parent;      // Menu to go back to with LEFT, NULL = top menu
} Menu;


// SETTINGS:

// The values are indexes into the item lists below. main.c reads them and sends them to node 2 when a game starts.
 
uint8_t setting_lives      = 2;   // "3 lives"
uint8_t setting_player     = 7;   // "Guest"
uint8_t setting_controller = 0;   // "Joystick"
uint8_t setting_difficulty = 0;   // "Easy"
 
uint8_t menu_start_game_requested = 0;   // set to 1 when "Start game" is selected


// SETTINGS -> SUBMENUS:
 
const char* num_of_lives_items[] = { "1", "2", "3", "4", "5", "Infinite" };
Menu num_of_lives = {
    .items = num_of_lives_items,
    .num_items = 6,
    .submenus = NULL,          // ingen undermenyer
    .parent = NULL
};
 
const char* choose_player_items[] = { "Hanne", "Helene", "Ulrikke", "Studass", "Sensor", "Looser", "VIP Guest", "Guest" }; // Has no effect yet, supposed o be used when registrering highscores
Menu choose_player = {
    .items = choose_player_items,
    .num_items = 8,
    .submenus = NULL,
    .parent = NULL
};
 
const char* controller_items[] = { "Joystick", "Touchpad" }; 
Menu controller = {
    .items = controller_items,
    .num_items = 2,
    .submenus = NULL,
    .parent  = NULL
};
 
const char* difficulty_items[] = { "Easy", "Medium", "Hard", "Extreme" };
Menu difficulty = {
    .items = difficulty_items,
    .num_items = 4,
    .submenus = NULL,
    .parent = NULL// motor
};
 
 
// SETTINGS -> PARENT MENU:
 
const char* settings_items[] = { "Number of lives", "Choose player", "Controller", "Difficulty" };
Menu* settings_submenus[4]   = { &num_of_lives,  &choose_player, &controller, &difficulty };
Menu settings = {
    .items = settings_items,
    .num_items = 4,
    .submenus = settings_submenus,
    .parent = NULL //Set to main later
};

 
// HIGHSCORES:
 
const char* prev_highscores_items[] = { "1. Ulrikke", "2. Hanne", "3. Helene", "4. Sensor" }; // Not implemented yet
Menu prev_highscores = {
    .items = prev_highscores_items,
    .num_items = 4,
    .submenus = NULL,
    .parent = NULL
};


// MAIN MENU:
 
const char* main_items[] = { "Start game", "Settings", "Previous highscores", "Calibrate" };
Menu* main_submenus[4]   = { NULL,  &settings, &prev_highscores, NULL };
Menu MainMenu = {
    .items = main_items,
    .num_items = 4,
    .submenus = main_submenus,
    .parent = NULL             // Main menu has no parent
};
 

// MENU FUNCTIONS

Menu* current_menu = &MainMenu; // Menu shown on screen
uint8_t selected = 0; // Line the cursor is on


// Links every submenu to the menu above it, so LEFT knows where to go back. Done at runtime because the structs refer to each other.
void menu_init(void) {

    settings.parent = &MainMenu;
    prev_highscores.parent = &MainMenu;
 
    num_of_lives.parent = &settings;
    choose_player.parent = &settings;
    controller.parent = &settings;
    difficulty.parent = &settings;
}
 

// Returns the stored choice for the current menu, or -1 if it is not a settings menu.
static int8_t current_choice(void) {
    if (current_menu == &num_of_lives) {
		return setting_lives;
	}
    if (current_menu == &choose_player) {
		return setting_player;
	}
    if (current_menu == &controller) {
		return setting_controller;
	}
    if (current_menu == &difficulty) {
		return setting_difficulty;
	}
    return -1;
}
 
// Draws the current menu: > marks the cursor, * marks the stored choice
void menu_draw(void)
{
    int8_t choice = current_choice();

    for (uint8_t i = 0; i < 8; i++) {
        oled_clear_line(i);

        if (i < current_menu->num_items) {
            oled_pos(i, 0);

            if (i == selected) {
                oled_print(">");        // cursor on this line
            } else {
                oled_print(" ");        // keep the text aligned
            }

            oled_print(current_menu->items[i]);

            if (i == choice) {
                oled_print(" *");
            }
        }
    }
}

// Goes up one level and puts the cursor on the line we came from
static void go_to_parent(void) {

    Menu* child = current_menu;
    current_menu = current_menu->parent;
    selected = 0;

    for (uint8_t i = 0; i < current_menu->num_items; i++) {
        if (current_menu->submenus != NULL && current_menu->submenus[i] == child) {
            selected = i;
            break;
        }
    }
}


// Handles "select" on a line that has no submenu, depending on which menu we are in
static void handle_button(void) {

    if (current_menu == &MainMenu) {
        switch (selected) {
            case 0: 					// Start game: main.c sees the flag and starts the game
                menu_start_game_requested = 1;
                break;
            case 3:     				// Calibrate
                joystick_calibrate();
                break;
            default:   // Settings and highscores: open the submenu. Equals RIGHT
                if (current_menu->submenus[selected] != NULL) {
                    current_menu = current_menu->submenus[selected];
                    selected = 0;
                }
                break;
        }
    }
 
	// Settings: open the chosen submenu with the cursor on the stored choice
    else if (current_menu == &settings) {
        current_menu = current_menu->submenus[selected];
        selected = current_choice();          // start pa det lagrede valget
		
    }
 
	// Settings submenus: store the choice
    else if (current_menu == &num_of_lives) {
        setting_lives = selected;
    }

    else if (current_menu == &choose_player) {
        setting_player = selected;
    }

    else if (current_menu == &controller) {
        setting_controller = selected;
    }

    else if (current_menu == &difficulty) {
        setting_difficulty = selected;
    }

}
 

// Moves around in the menu based on the navigation buttons, then redraws it.
void menu_handle_input(MenuJoystickDirection menujoystickdirection) {

	switch (menujoystickdirection) {
		case MENU_UP:
			if (selected > 0) {
				selected --;
			}
			break;

		case MENU_DOWN:
			if (selected < current_menu->num_items - 1) {
				selected++;
			}
			break;

		case MENU_RIGHT: // open submenu, or select the line if it has none
			if (current_menu->submenus != NULL && current_menu->submenus[selected] != NULL) {
                current_menu = current_menu->submenus[selected];  // gå til "neste"
                selected = 0;
            }
			else  {
				handle_button(); 
			}
			break;

		case MENU_LEFT:  // back to the parent menu
            if (current_menu->parent != NULL) {
                current_menu = current_menu->parent;
                selected = 0;
            }
			break;
		
		case MENU_BUTTON: // ignored: the button sometimes registers presses by itself
            break;

		default:
			break;
		}
    menu_draw();
}


// GAME SCREENS

// Redraws only the time line on the game screen (used every second)
void menu_update_time(uint16_t seconds)
{
    char buf[22];
    oled_clear_line(5);
    oled_pos(5, 0);
    sprintf_P(buf, PSTR("Time: %u:%02u"), seconds / 60, seconds % 60);
    oled_print(buf);
}

// Draws the screen shown while playing: lives left and time.
void menu_show_game_screen(uint8_t lives, uint16_t seconds)
{
    char buf[22];
    oled_reset();

    oled_pos(1, 0);
    oled_print("GAME ON");

    oled_pos(3, 0);
    if (lives == 255) {       		// 255 = infinite lives
        oled_print("Lives: infinite");
    } else {
        sprintf_P(buf, PSTR("Lives: %u"), lives);
        oled_print(buf);
    }

    menu_update_time(seconds);
}



// Skull - given to us by Claude
static const uint8_t PROGMEM skull[5][48] = {
    { 0x00, 0xC0, 0xE0, 0xF0, 0xFC, 0xFE, 0xFC, 0xDC, 0x80, 0x80, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xC0,
      0xE0, 0xE0, 0xE0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xE0, 0xE0, 0xE0,
      0xC0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x80, 0x80, 0xDC, 0xFC, 0xFE, 0xFC, 0xF0, 0xE0, 0xC0, 0x00 },
    { 0x00, 0x01, 0x01, 0x03, 0x01, 0x00, 0x01, 0x03, 0x07, 0x03, 0xF8, 0xFE, 0xFF, 0xFF, 0x3F, 0x1F,
      0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x3F, 0xFF, 0xFF, 0x3F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,
      0x1F, 0x3F, 0xFF, 0xFF, 0xFE, 0xF8, 0x03, 0x07, 0x03, 0x01, 0x00, 0x01, 0x03, 0x01, 0x01, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x07, 0x0F, 0x1F, 0x3E, 0x7C,
      0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xE2, 0xF3, 0xF3, 0xE2, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC,
      0x7C, 0x3E, 0x1F, 0x0F, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xC0, 0xE0, 0xF0, 0x78, 0x78, 0x3C, 0x00,
      0x1F, 0x37, 0x7D, 0x7F, 0x77, 0x7D, 0x7F, 0x77, 0x7D, 0x7F, 0x77, 0x7D, 0x7F, 0x77, 0x3D, 0x1F,
      0x00, 0x3C, 0x78, 0x78, 0xF0, 0xE0, 0xC0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00 },
    { 0x00, 0x07, 0x0F, 0x1F, 0x7F, 0xFE, 0x7F, 0x77, 0x03, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x03, 0x77, 0x7F, 0xFE, 0x7F, 0x1F, 0x0F, 0x07, 0x00 },
};

// Draws the skull with its top left corner at the given line and column
static void draw_skull(uint8_t line, uint8_t column)
{
    for (uint8_t page = 0; page < 5; page++) {
        oled_pos(line + page, column);
        for (uint8_t col = 0; col < 48; col++) {
            oled_data(pgm_read_byte(&skull[page][col]));
        }
    }
}

// Draws the game over screen: skull, "GAME OVER" and the final time
void menu_show_game_over(uint16_t seconds)
{
    char buf[22];
    oled_reset();

    draw_skull(0, 40);

    oled_pos(5, 37);
    oled_print("GAME OVER");

    sprintf(buf, "Time: %u:%02u", seconds / 60, seconds % 60);
    oled_pos(6, 0);
    oled_print(buf);

    oled_pos(7, 0);
    oled_print("Press to continue");
}

