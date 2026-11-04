#include "sam.h"
#include "encoder.h"

// Gives PC25/PC26 to the timer counter and starts the quadrature decoder.
void encoder_init(void){
	// Clock to TC2 channel 0. ID 33 is above 31, so it is in PCER1, bit 33 - 32 = 1
    PMC->PMC_PCER1 = (1 << (ID_TC6-32));

	// Let peripheral B (TIOA6/TIOB6) control PC25 and PC26 instead of the PIO
    PIOC->PIO_PDR   = PIO_PC25 | PIO_PC26;   
    PIOC->PIO_ABSR |= PIO_PC25 | PIO_PC26;   

	TC2 -> TC_BMR = TC_BMR_QDEN | TC_BMR_POSEN; // activate quadrature decoder and position measurement

    // Channel 0 counts on XC0, which is the decoded encoder signal
	TC2->TC_CHANNEL[0].TC_CMR = TC_CMR_TCCLKS_XC0;

	// Enable the clock and reset the counter to 0
    TC2->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG; 
}

// Returns the current position in encoder steps (negative to the left of 0).
int32_t encoder_read(void){
	return (int32_t)TC2->TC_CHANNEL[0].TC_CV;
}

// Sets the current position to 0.
void encoder_reset(void){
    TC2->TC_CHANNEL[0].TC_CCR = TC_CCR_SWTRG;
}
