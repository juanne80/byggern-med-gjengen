#include "uart_driver.h"
#include <stdio.h>
#include <avr/io.h>


// Sets the baud rate and enables the transmitter and receiver (8 data bits, 1 stop bit). Function copied from ATmega162 datasheet, p. 172. 
void USART_Init(unsigned int ubrr){
    /* Set baud rate */
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;

    /* Enable receiver and transmitter */
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);

    /* 8 data bits, 1 stop bits */
    UCSR0C = (1 << URSEL0) |
         (1 << UCSZ00) |
         (1 << UCSZ01);

}

// Sends one byte, waiting until the transmit buffer is free. Function copied from ATmega162 datasheet, p. 173. 
void USART_Transmit(unsigned char data){
    /* Wait for empty transmit buffer */
    while (!(UCSR0A & (1 << UDRE0)));

    /* Put data into buffer */
    UDR0 = data;

}

// Waits until a byte has been received and returns it. Function copied from ATmega162 datasheet, p. 176. 
unsigned char USART_Receive(void){
    /* Wait for data to be received */
    while (!(UCSR0A & (1 << RXC0)));

    return UDR0;

}

// Sends one character. Used by stdio so printf() writes to the UART. Needed for fdevopen() in main. 
int uart_putchar(char c, FILE *stream){
    USART_Transmit(c);
    return 0;
}

// Recievies one character. Used by stdio so printf() writes to the UART. Needed for fdevopen() in main. 
int uart_getchar(FILE *stream){
    return USART_Receive();
}
