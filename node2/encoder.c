#include "sam.h"
#include "../node1/io_driver.h"
#include "can.h"
#include "encoder.h"
#include "ir_driver.h"
#include "timer_driver.h"

void encoder_init(void){
	// Aktiverer en timer i kanal 0, TC2. (TC6 har ID 33, D5/PC25 -> TIOA6, her 6 kommer fra?)
    PMC->PMC_PCER1 = (1 << (ID_TC6-32));

	// Gi PC25 (TIOA6) og PC26 (TIOB6) til TC-en, periferi B
    PIOC->PIO_PDR   = PIO_PC25 | PIO_PC26;   // PIO slipper pinnene
    PIOC->PIO_ABSR |= PIO_PC25 | PIO_PC26;   // velg periferi B

	TC2 -> TC_BMR = TC_BMR_QDEN | TC_BMR_POSEN; // aktiverer quadrature decoder og posisjonsmåling

    // Velger instillinger for timer-kanalen. 
    TC2->TC_CHANNEL[0].TC_CMR = TC_CMR_TCCLKS_XC0; 			// Velger XC0 som klokke

    TC2->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG; // slår på klokken til telleren og nullstiller telleren til 0 før den starter å telle
}

int32_t encoder_read(void){
	return (int32_t)TC2->TC_CHANNEL[0].TC_CV;
}

void encoder_reset(void){
    TC2->TC_CHANNEL[0].TC_CCR = TC_CCR_SWTRG;   // setter telleren til 0
}
