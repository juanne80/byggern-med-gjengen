#include "sam.h"
#include "solenoid.h"

static volatile uint8_t shoot_ticks = 0;		 // ticks of 10 ms left of the current shot

// Sets up PA24 as an output, starting high so the solenoid does not fire at start-up.
void solenoid_init(void){
    // Clock to PIOA (ID 11 -> PCER0)
    PMC->PMC_PCER0 = (1 << ID_PIOA);
 
    PIOA->PIO_SODR = PIO_PA24;      // set high (off) before it becomes an output
    PIOA->PIO_PER  = PIO_PA24;
    PIOA->PIO_OER  = PIO_PA24;
}

void solenoid_on(void) {
	PIOA->PIO_CODR = PIO_PA24;
}

void solenoid_off(void) {
	PIOA->PIO_SODR = PIO_PA24;
}



// Starts one shot of 200 ms. Called when the button goes from released to pressed. Ignored if a shot is already in progress
void solenoid_fire(void){
    if (shoot_ticks == 0) {
        solenoid_on();
        shoot_ticks = 20;           // 20 x 10 ms = 200 ms
    }
}
 

// Counts down the shot and turns the solenoid off when it is over. Called from TC0_Handler every 10 ms.
void solenoid_tick(void){
    if (shoot_ticks > 0) {
        shoot_ticks--;
        if (shoot_ticks == 0) {
            solenoid_off();
        }
    }
}
// Returns 1 while a shot is in progress, otherwise 0.
uint8_t solenoid_active(void)
{
    return shoot_ticks > 0;
}