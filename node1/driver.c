#include "driver.h"
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>


void USART_Init(unsigned int ubrr)
{
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

void USART_Transmit(unsigned char data)
{
    /* Wait for empty transmit buffer */
    while (!(UCSR0A & (1 << UDRE0)));

    /* Put data into buffer */
    UDR0 = data;

}

unsigned char USART_Receive(void)
{
    /* Wait for data to be received */
    while (!(UCSR0A & (1 << RXC0)));

    return UDR0;

}

int uart_putchar(char c, FILE *stream)
{
    USART_Transmit(c);
    return 0;
}

int uart_getchar(FILE *stream)
{
    return USART_Receive();
}