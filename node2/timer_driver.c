#include "timer_driver.h"
#include "sam.h"
#include "motor.h"
#include "solenoid.h"

volatile uint32_t timer_ticks = 0;   

void timer_init(void)
{
	// Aktiverer en timer i kanal 0, TC0
    PMC->PMC_PCER0 = (1 << ID_TC0);

    // Velger instillinger for timer-kanalen. 
    TC0->TC_CHANNEL[0].TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK1  // MCK/2 = 42 MHz
                              | TC_CMR_WAVE					// setter timer til waveform-modus
                              | TC_CMR_WAVSEL_UP_RC;		// teller fra 0 til verdien til RC

    TC0->TC_CHANNEL[0].TC_RC = 420000; //Setter verdien til RC slik at telleren øker hvert 10. ms


    TC0->TC_CHANNEL[0].TC_IER = TC_IER_CPCS; //Aktiverer RC Compare interrupt

    NVIC_EnableIRQ(TC0_IRQn); // Aktiverer IRQn, prosessoren lytter til denne kanalen

    TC0->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG; // slår på klokken til telleren og nullstiller telleren til 0 før den starter å telle
}


void TC0_Handler(void)
{
    
    TC0->TC_CHANNEL[0].TC_SR; //Leser statusregisteret

	// skrive hva vi vil skal skje hvert 10. ms
    timer_ticks++;
	motor_on_tick();
	solenoid_tick();
}

// pwm
void tc_pwm_init(void)
{
	// Aktiverer en timer i kanal 1, TC0
    PMC->PMC_PCER0 = (1 << ID_TC1);

    // Gir pinne PA2 (A7) til timeren. Periferi A = TIOA1
    PIOA->PIO_PDR   = PIO_PDR_P2; // TIOA1 er utgangen til kanel 1 og er koblet til PA2 (A7). Deaktiverer at PIO styrer PA2 
    PIOA->PIO_ABSR &= ~PIO_ABSR_P2;   // bit = 0 betyr periferi A, altså styrer periferi A PA2 (timeren)

	// Velger instillinger for timer-kanalen.
    TC0->TC_CHANNEL[1].TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK1  // 42 MHz
                              | TC_CMR_WAVE                 // waveform-modus
                              | TC_CMR_WAVSEL_UP_RC         // tell 0 -> RC
                              | TC_CMR_ACPC_SET             // pinne høy ved RC
                              | TC_CMR_ACPA_CLEAR;          // pinne lav ved RA


    TC0->TC_CHANNEL[1].TC_RC = 840000;  // periode 20 ms
    TC0->TC_CHANNEL[1].TC_RA = 63000;   // puls 1,5 ms

    TC0->TC_CHANNEL[1].TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG; // slår på klokken til telleren og nullstiller telleren til 0 før den starter å telle
}

void tc_pwm_set_pulse_us(uint32_t pulse_us)
{
    if (pulse_us < 900)  pulse_us = 900;    // aldri under 0,9 ms
    if (pulse_us > 2100) pulse_us = 2100;   // aldri over 2,1 ms

    TC0->TC_CHANNEL[1].TC_RA = pulse_us * 42;
}