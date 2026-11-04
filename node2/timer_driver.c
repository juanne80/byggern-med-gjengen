#include "timer_driver.h"
#include "sam.h"
#include "motor.h"
#include "solenoid.h"

volatile uint32_t timer_ticks = 0;    // +1 every 10 ms, 100 ticks = 1 s

// Starts TC0 channel 0 with an interrupt every 10 ms.
void timer_init(void)
{
    // Clock to TC0 channel 0 (ID_TC0 = 27 -> PCER0)
    PMC->PMC_PCER0 = (1 << ID_TC0);
 
    TC0->TC_CHANNEL[0].TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK1    // MCK/2 = 42 MHz
                              | TC_CMR_WAVE                   // waveform mode
                              | TC_CMR_WAVSEL_UP_RC;          // count from 0 up to RC, then restart


    TC0->TC_CHANNEL[0].TC_RC = 420000; //42 MHz * 10 ms = 420 000 steps per period


    TC0->TC_CHANNEL[0].TC_IER = TC_IER_CPCS; // Interrupt when the counter reaches RC
    NVIC_EnableIRQ(TC0_IRQn); // Activate IRQn, the processor listens to this channel

    TC0->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG; // Enable the clock and reset the counter
}


// Interrupt handler, runs every 10 ms
void TC0_Handler(void){
    TC0->TC_CHANNEL[0].TC_SR;   // reading the status register clears the interrupt flag
 
    timer_ticks++;
    motor_on_tick();            // PID controller
    solenoid_tick();            // ends the solenoid shot after its time
}


// NOT USED IN FINAL GAME
// Servo PWM with TC0 channel 1 on PA2 (A7, TIOA1): 20 ms period, starts at 1.5 ms.
void tc_pwm_init(void)
{
    // Clock to TC0 channel 1 (ID_TC1 = 28 -> PCER0)
    PMC->PMC_PCER0 = (1 << ID_TC1);
 
    // PA2 is controlled by peripheral A (TIOA1) instead of the PIO
    PIOA->PIO_PDR   = PIO_PDR_P2;
    PIOA->PIO_ABSR &= ~PIO_ABSR_P2;     // bit = 0 selects peripheral A
 
    TC0->TC_CHANNEL[1].TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK1    // 42 MHz
                              | TC_CMR_WAVE                   // waveform mode
                              | TC_CMR_WAVSEL_UP_RC           // count from 0 up to RC
                              | TC_CMR_ACPC_SET               // pin high at RC (start of the period)
                              | TC_CMR_ACPA_CLEAR;            // pin low at RA (end of the pulse)
 
    TC0->TC_CHANNEL[1].TC_RC = 840000;  // period 20 ms (42 MHz * 20 ms)
    TC0->TC_CHANNEL[1].TC_RA = 63000;   // pulse 1.5 ms (42 MHz * 1.5 ms)
 
    TC0->TC_CHANNEL[1].TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG;
}


// Sets the servo pulse width for tc_pwm_init() in microseconds, limited to 900-2100 us.
void tc_pwm_set_pulse_us(uint32_t pulse_us)
{
    if (pulse_us < 900)  pulse_us = 900;
    if (pulse_us > 2100) pulse_us = 2100;
 
    TC0->TC_CHANNEL[1].TC_RA = pulse_us * 42;   // 42 timer steps per us
}
