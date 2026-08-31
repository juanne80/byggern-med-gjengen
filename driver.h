#include <stdio.h>

void USART_Transmit( unsigned char data);
unsigned char USART_Receive( void );
void USART_Init( unsigned int ubrr);
int uart_putchar(char c, FILE *stream);
int uart_getchar(FILE *stream);