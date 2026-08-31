#include <avr/io.h>
#include <util/delay.h>
#include "driver.h"
#include <stdio.h>

#define FOSC 4915200UL
#define BAUD 9600
#define MYUBRR (FOSC/16/BAUD-1)

int main(void)
{
	USART_Init(MYUBRR);

	fdevopen(uart_putchar, uart_getchar);

    while (1)
    {
		//USART_Transmit('A');
        //printf("Hello world!\r\n");
		int data;
		data = USART_Receive();
		USART_Transmit(data);
        _delay_ms(1000);
    }

}

