#include "sam.h"
#include "pwm_driver.h"


void pwm_init(void)
{
    PMC->PMC_PCER1 = (1 << (ID_PWM - 32)); //ID starts at 32, so it must be shifted (ID-32), activates the clock to ID

    // PB13 is controlled by peripheral B (PWMH1) instead of the PIO
    PIOB->PIO_PDR   = PIO_PDR_P13;
    PIOB->PIO_ABSR |= PIO_ABSR_P13;

	// CLKA = 84 MHz / 2 / 42 = 1 MHz,
    PWM->PWM_CLK = PWM_CLK_PREA(1) | PWM_CLK_DIVA(42); 

    PWM->PWM_CH_NUM[1].PWM_CMR  = PWM_CMR_CPRE_CLKA | PWM_CMR_CPOL;   // CLKA, pulse starts high
    PWM->PWM_CH_NUM[1].PWM_CPRD = 20000;                              // period 20 ms
    PWM->PWM_CH_NUM[1].PWM_CDTY = 1500;                               // start in the middle


    PWM->PWM_ENA = PWM_ENA_CHID1;    
}

// Sets the servo pulse width in microseconds, limited to 900-2100 us.
void pwm_set_pulse_us(uint32_t pulse_us)
{
    // Never outside 0.9-2.1 ms, to protect the servo
    if (pulse_us < 900) {
		pulse_us = 900;
    }
    if (pulse_us > 2100) {
        pulse_us = 2100;
    }

    // CDTYUPD is applied at the start of the next period, so no pulse is cut short
    PWM->PWM_CH_NUM[1].PWM_CDTYUPD = pulse_us;

}

// Converts a joystick percentage to a servo pulse width.
// -100 % = 900 us, 0 = 1500 us, 100 % = 2100 us. Small values are ignored so the servo stays still in the middle
void joystick_to_PWM(int8_t xpercentage){
	uint32_t dutyCycle = (uint32_t)(xpercentage*6 + 1500);
	if (xpercentage > -10 && xpercentage < 10){
		dutyCycle = 1500;
	}
	pwm_set_pulse_us(dutyCycle);
	
}
