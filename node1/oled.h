#include <stdint.h>
#include <stdio.h>

void oled_command(uint8_t command);
void oled_data(uint8_t data);
void oled_init();
void oled_reset();
void oled_home();
void oled_goto_line(uint8_t line);
void oled_goto_line(uint8_t line);
void oled_clear_line(uint8_t line);
void oled_pos(uint8_t line, uint8_t column);
void oled_print(const char *str);
