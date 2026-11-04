#include <stdio.h>
#include <stdarg.h>
#include "sam.h"
#include "uart.h"
#include "can.h"
#include "pwm_driver.h"
#include "timer_driver.h"
#include "ir_driver.h"
#include "gamer_driver.h"
#include "encoder.h"
#include "motor.h"
#include "solenoid.h"

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
	
	//Vi velger synch = 1, prop = 2, ps1 = 7, ps2 = 6 -> 16 tq, der tq = 500 ns. Får 8 mikro s per bit = 125 kbit/s
	const CanInit can_in = {
		.brp    = 41, // TQ = (BRP+1)/MCK = 42/84 MHz = 500 ns
		.sjw    =  0, //re-synhronization jump width = 1 tq (også -1)
		.propag = 1, //PROP - 1
		.phase1 = 6, //PS1 - 1
		.phase2 = 5, //PS2 - 1
		.smp    = 0, // 1 sampling
	};

	can_init(can_in, 0); 
	printf("CAN_BR = 0x%08lX\r\n", CAN0->CAN_BR);

	// Skriv ut og dekod CAN_BR, så dere kan sammenligne med CNF1-3 på node 1.
    // Feltene lagres som "verdi - 1", derfor +1 for å få antall TQ.
    uint32_t br = CAN0->CAN_BR;
    printf("CAN_BR = 0x%08lX\r\n", br);
    printf("  BRP=%lu  -> TQ = %lu/MCK\r\n", (br >> 16) & 0x7F, ((br >> 16) & 0x7F) + 1);
    printf("  PROPAG=%lu TQ, PHASE1=%lu TQ, PHASE2=%lu TQ, SJW=%lu TQ, SMP=%lu\r\n",
           ((br >> 8)  & 0x7) + 1,
           ((br >> 4)  & 0x7) + 1,
           ((br >> 0)  & 0x7) + 1,
           ((br >> 12) & 0x3) + 1,
           (br >> 24) & 0x1);
	
	

	PMC->PMC_PCER0 = (1 << ID_PIOB);     // klokke til PIOB
	
	pwm_init();
	adc_init();
	encoder_init();
	motor_init();
	solenoid_init();

    motor_home();
	timer_init();

	
	CanMsg msg;
	uint8_t last_solenoid_button = 0;
	uint32_t last_print = 0;


 
    while (1) {
        if (can_rx(&msg)) {
			if (msg.id == 0x10) {
				int8_t xpercentage = (int8_t)msg.byte[0];
				int8_t ypercentage = (int8_t)msg.byte[1];
				uint8_t solenoid_button_pressed = msg.byte[2];
				joystick_to_PWM(xpercentage);
				motor_update_joystick(ypercentage);

				if (solenoid_button_pressed && !last_solenoid_button) {
					solenoid_fire();
				}
			last_solenoid_button = solenoid_button_pressed;
			}
			else if(msg.id == 0x20){
				setting_lives      = msg.byte[1];
				setting_player     = msg.byte[2];
				setting_controller = msg.byte[3];
				setting_difficulty = msg.byte[4];
				game_start(setting_lives, setting_player, setting_controller, setting_difficulty);
			}
        }
		
		check_ir_beam(adc_read());

		static uint32_t last_status = 0;
		if (timer_ticks - last_status >= 20) {        // hvert 200 ms
			last_status = timer_ticks;

			uint8_t lives;
			if (setting_lives == 5) {
				lives = 255;                          // 255 = uendelig
			} else {
				lives = get_lives_left();
			}

			uint16_t score = get_ir_counter();
			uint16_t time = get_game_time();
			
			CanMsg status;
			status.id      = 0x30;
			status.length  = 4;
			status.byte[0] = game_is_running();       // 1 = spillet går
			status.byte[1] = lives;
			status.byte[2] = time >> 8;        // høy byte
			status.byte[3] = time & 0xFF;      // lav byte

			can_tx(status);
		}
		
		
		if (timer_ticks - last_print >= 50) {         // hvert halve sekund
			last_print = timer_ticks;
			printf("Counter = %u  Lives = %u  ADC = %u\r\n",
				get_ir_counter(), get_lives_left(), adc_read());
		}
	}
}


// sudo picocom -b 9600 /dev/ttyS4
// EXIT: ctrl X


