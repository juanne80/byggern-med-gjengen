#include <stdio.h>
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

#define CAN_ID_JOYSTICK 0x10
#define CAN_ID_SETTINGS 0x20
#define CAN_ID_STATUS 0x30
 


int main()
{
    SystemInit();
	WDT->WDT_MR = WDT_MR_WDDIS; //Disable Watchdog Timer

	
    uart_init(F_CPU, 9600); //freuency + baudrate

	//CAN init
	
    // CAN bit timing, must match node 1 (125 kbit/s):
    // 1 sync + 2 prop + 7 phase1 + 6 phase2 = 16 TQ = 8 us per bit
	const CanInit can_in = {
		.brp    = 41, // TQ = (BRP+1)/MCK = 42/84 MHz = 500 ns
		.sjw    =  0, //re-synhronization jump width = 1 tq (also -1)
		.propag = 1, //PROP - 1
		.phase1 = 6, //PS1 - 1
		.phase2 = 5, //PS2 - 1
		.smp    = 0, //sample once
	};

	can_init(can_in, 0); 
   
	PMC->PMC_PCER0 = (1 << ID_PIOB);     // clock to PIOB
	pwm_init();			// servo
	adc_init();		 // IR receiver
	encoder_init();
	motor_init();
	solenoid_init();

    motor_home();  // slider to its start position before the controller starts
	timer_init(); //pid controller and solenoid timer

	
	CanMsg msg;
	uint8_t last_solenoid_button = 0;
	uint32_t last_status = 0;


 
    while (1) {
		//Messages from node 1
        if (can_rx(&msg)) {
			if (msg.id == CAN_ID_JOYSTICK) {
				int8_t xpercentage = (int8_t)msg.byte[0];
				int8_t ypercentage = (int8_t)msg.byte[1];
				uint8_t solenoid_button_pressed = msg.byte[2];

				joystick_to_PWM(xpercentage);		// servo
				motor_update_joystick(ypercentage);		// motor reference

				// Fire only when the button goes from released to pressed
				if (solenoid_button_pressed && !last_solenoid_button) {
					solenoid_fire();
				}
				last_solenoid_button = solenoid_button_pressed;
			} 
			else if (msg.id == CAN_ID_SETTINGS) {
                // byte[0] is the command (1 = start), byte[1-4] are the settings
                game_start(msg.byte[1], msg.byte[2], msg.byte[3], msg.byte[4]);
            }

        }
		
		check_ir_beam(adc_read()); //goal detection

		// Game status to node 1 every 200 ms
		if (timer_ticks - last_status >= 20) {      
			last_status = timer_ticks;

			uint8_t lives;
			if (setting_lives == 5) {
				lives = 255;                          // 255 = infinite lives
			} else {
				lives = get_lives_left();
			}

			uint16_t time = get_game_time();
			
			CanMsg status;
			status.id      = CAN_ID_STATUS;
			status.length  = 4;
			status.byte[0] = game_is_running();       
			status.byte[1] = lives;
			status.byte[2] = time >> 8;        // high byte
			status.byte[3] = time & 0xFF;      // low byte

			can_tx(status);
		}
		
	}
}


// sudo picocom -b 9600 /dev/ttyS4
// EXIT: ctrl X


