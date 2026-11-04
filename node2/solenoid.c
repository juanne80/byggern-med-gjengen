#include "sam.h"
#include "solenoid.h"


void solenoid_init(void)
{
    PMC->PMC_PCER0 = (1 << ID_PIOA);   // klokke til PIOA (ID 11 → PCER0)

    PIOA->PIO_SODR = PIO_PA24;      // først høy = av (aktiv lav)
    PIOA->PIO_PER  = PIO_PA24;
    PIOA->PIO_OER  = PIO_PA24;
}

void solenoid_on(void) {
	PIOA->PIO_CODR = PIO_PA24;
}

void solenoid_off(void) {
	PIOA->PIO_SODR = PIO_PA24;
}

static volatile uint8_t shoot_ticks = 0;

void solenoid_fire(void)              // kalles når knappen går 0 → 1
{
    if (shoot_ticks == 0) {           // ikke skyt mens den allerede er ute
        solenoid_on();
        shoot_ticks = 20;             // 10 × 10 ms = 100 ms
    }
}

void solenoid_tick(void)              // kalles fra TC0_Handler hvert 10. ms
{
    if (shoot_ticks > 0) {
        shoot_ticks--;
        if (shoot_ticks == 0) {
            solenoid_off();
        }
    }
}

uint8_t solenoid_active(void)
{
    return shoot_ticks > 0;
}