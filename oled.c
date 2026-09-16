#include "oled.h"
#include "spi.h"
#include <avr/io.h>


void oled_command(uint8_t command)
{
    PORTB &= ~(1 << PB1);          // D/!C lav = kommando
    spi_select_slave(DISP);
    spi_write_byte(command);
    spi_deselect_all();
}

void oled_data(uint8_t data)
{
    PORTB |= (1 << PB1);           // D/!C høy = data
    spi_select_slave(DISP);
    spi_write_byte(data);
    spi_deselect_all();
}

void oled_init() {
	oled_command(0x81); //Set Contrast Control
	oled_command(0xA6); //Set Normal diplay (not inverse)

	oled_command(0x00); //Lower column start adress
	

	oled_command(0xA4); //Entire Display ON - output follows RAM content
	oled_command(0xAF); // Set Display ON

}

void oled_reset() {

}

void oled_home() {

}

void oled_goto_line(line) {

}

void oled_goto_column(column) {

}

void oled_clear_line(line) {

}

void oled_pos(row, column) {

}

void oled_print(char*) {

}