#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "oled.h"
#include "spi_driver.h"
#include "fonts.h"


// Sends one command byte to the display (D/C low).
void oled_command(uint8_t command)
{
    PORTB &= ~(1 << PB2);     // D/C low = command
    spi_select_slave(DISP);
    spi_transfer(command);
    spi_deselect();
}

// Sends one data byte to the display (D/C high), f.ex. one column of 8 pixels.
void oled_data(uint8_t data)
{
    PORTB |= (1 << PB2);           // D/C high = data
    spi_select_slave(DISP);
    spi_transfer(data);
    spi_deselect();
}

// Clears the screen, sets the orientation and turns the display on.
void oled_init(void)
{
	oled_reset();  

	oled_command(0xA4);     // Show what is in the display RAM
    oled_command(0xA1);     // Mirror horizontally
    oled_command(0xC8);     // Mirror vertically, the two mirrors turn the image the right way for how the display is mounted on the board
    
	oled_home();
    
	oled_command(0xAF);     // Display on

}

// Clears the whole screen
void oled_reset() {
	for (uint8_t line = 0; line < 8; line++) {
		oled_clear_line(line);
	}
}


// Moves the write position to the top left corner
void oled_home() {
	oled_pos(0,0);
}

// Moves the write position to the start of line (page) 0-7.
void oled_goto_line(uint8_t line)
{
    oled_command(0xB0 | (line & 0x07)); // The last 3 bits of line are masked out
}

// Moves the write position to column 0-127 on the current line. The column is sent as two commands, 4 bits in each.
void oled_goto_column(uint8_t column)
{
    oled_command(0x00 | (column & 0x0F));         // Lower nibble
    oled_command(0x10 | ((column >> 4) & 0x0F));  // Upper nibble
}

// Clears one line by writing 128 empty columns
void oled_clear_line(uint8_t line) {
	oled_pos(line,0);
	for (uint8_t i = 0; i < 128; i++) {
		oled_data(0x00);
	}
}

// Moves the write position to the given line and column
void oled_pos(uint8_t line, uint8_t column) {
	oled_goto_line(line);
	oled_goto_column(column);
}

// Prints a text string at the current position using the font given (font.h)
void oled_print(const char *str)
{
    while (*str)
    {
        uint8_t index = *str - ' ';        // ASCII-value relative to space (0x20)
        for (uint8_t col = 0; col < 5; col++)
        {
            oled_data(pgm_read_byte(&font5[index][col]));
        }
        oled_data(0x00);        		// Empty colum between character
        str++;
    }
}


