#include <stdint.h>
#include <avr/io.h>
#include <stdio.h>
#include <avr/pgmspace.h>


#include "oled.h"
#include "spi_driver.h"
#include "fonts.h"

uint8_t joystick_line;

void oled_command(uint8_t command)
{
    PORTB &= ~(1 << PB2);          // D/!C lav = kommando
    spi_select_slave(DISP);
    spi_transfer(command);
    spi_deselect();
}

void oled_data(uint8_t data)
{
    PORTB |= (1 << PB2);           // D/!C høy = data
    spi_select_slave(DISP);
    spi_transfer(data);
    spi_deselect();
}

void oled_init(void)
{

	//oled_command(0xFD);
	oled_reset();   

	oled_command(0xA4);

	oled_command(0xA1);
	oled_command(0xC8);

    oled_home();  

    oled_command(0xAF); 
	joystick_line = 0;

}

void oled_reset() {
	for (uint8_t line = 0; line < 8; line++) {
		oled_clear_line(line);
	}
 }

void oled_home() {
	oled_pos(0,0);
 }

void oled_goto_line(uint8_t line)
{
    oled_command(0xB0 | (line & 0x07)); //The last 3 bits of line are masked out
}

void oled_goto_column(uint8_t column)
{
    oled_command(0x00 | (column & 0x0F));         // Lower nibble
    oled_command(0x10 | ((column >> 4) & 0x0F));  // Upper nibble
}

void oled_clear_line(uint8_t line) {
	oled_pos(line,0);
	for (uint8_t i = 0; i < 128; i++) {
		oled_data(0x00);
	}
 }

void oled_pos(uint8_t line, uint8_t column) {
	oled_goto_line(line);
	oled_goto_column(column);
}

void oled_print(const char *str)
{
    while (*str)
    {
        uint8_t index = *str - ' ';        // ASCII-value relative to space (0x20)
        for (uint8_t col = 0; col < 5; col++)
        {
            oled_data(pgm_read_byte(&font5[index][col]));
        }
        oled_data(0x00);                    // Empty colum between
        str++;
    }
}


