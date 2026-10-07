#include <stdio.h>
#include <stdarg.h>
#include "sam.h"
#include "uart.h"
#include "can.h"

/*
 * Remember to update the Makefile with the (relative) path to the uart.c file.
 * This starter code will not compile until the UART file has been included in the Makefile. 
 * If you get somewhat cryptic errors referencing functions such as _sbrk, 
 * _close_r, _write_r, _fstat etc, you have most likely not done that correctly.

 * If you get errors such as "arm-none-eabi-gcc: no such file", you may need to reinstall the arm gcc packages using
 * apt or your favorite package manager.
 */
//#include "../path_to/uart.h"

int main()
{
    SystemInit();
	WDT->WDT_MR = WDT_MR_WDDIS; //Disable Watchdog Timer


	//Uncomment after including uart above
    uart_init(F_CPU, 9600); //freuency + baudrate, må muligens endres til høyere baudrate
    printf("Hello World\n\r");

	//CAN init

	can_init(can_in, 0); 
	printf("CAN_BR = 0x%08lX\r\n", CAN0->CAN_BR);

	PMC->PMC_PCER0 = (1 << ID_PIOB);     // klokke til PIOB
    PIOB->PIO_PER  = PIO_PB13;           // PIO styrer pinnen
    PIOB->PIO_OER  = PIO_PB13;           // utgang
    PIOB->PIO_SODR = PIO_PB13;           // sett høy

    while (1) { }
    
}


// sudo picocom -b 9600 /dev/ttyS4
// EXIT: ctrl X
