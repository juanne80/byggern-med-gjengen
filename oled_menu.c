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

//SUBMENU 1

const char* submenu1_items[] = { "oh 1", "oh 2", "oh 3", "oh 4" };
Menu submenu1 = {
    .title = "Submenu 1",
    .items = submenu1_items,
    .num_items = 4,
    .submenus = NULL,   // ingen av disse fire har egne undermenyer
    .parent = NULL      // settes i menu_init(), se under
};

//MAIN MENU

const char* main_items[] = {"Alternativ 1","Alternativ 2","Alternativ 3","Alternativ 4", "Alternativ 5","Alternativ 6","Alternativ 7","Alternativ 8"};
Menu* main_submenus[8] = { &submenu1, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

Menu MainMenu = {
    .title = "Main menu",
    .items = main_items,
    .num_items = 8,
    .submenus = main_submenus,
    .parent = NULL
};

void menu_init(void) {
    submenu1.parent = &MainMenu;  // gjøres i en funksjon fordi structene refererer til hverandre
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


void menu_handle_input(JoystickDirection joystickdirection) {

	switch (joystickdirection) {
		case UP:
			if (selected > 0) selected --;
			break;
		case DOWN:
			if (selected < current_menu->num_items - 1) selected++;
			break;
		case RIGHT:
			if (current_menu->submenus != NULL && current_menu->submenus[selected] != NULL) {
                current_menu = current_menu->submenus[selected];  // gå til "neste"
                selected = 0;
            } else {
                // Menyen er "valgt"
            }
			break;
		case LEFT:
            if (current_menu->parent != NULL) {
                current_menu = current_menu->parent;              // gå til "forrige"
                selected = 0;
            }
		break;
		deafult:
			break;
		}
    menu_draw();
}

