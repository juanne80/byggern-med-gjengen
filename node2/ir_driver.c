#include "sam.h"
#include "../node1/io_driver.h"
#include "can.h"
#include "ir_driver.h"

void adc_init(void){
	PMC->PMC_PCER1 = (1 << (ID_ADC - 32)); // ID starter på 32, må derfor skiftes (ID-32), aktiverer klokken til ID

	ADC->ADC_MR = ADC_MR_FREERUN_ON          // måler kontinuerlig
                | ADC_MR_PRESCAL(2);         // ADC-klokke = 84 MHz / ((2+1)*2) = 14 MHz (Kan maks være 22 MHz, s.1403)

	ADC->ADC_CHER = ADC_CHER_CH7; //Aktiverer kanal 7 (PA16/ADC[0], s.40)
	ADC->ADC_CR = ADC_CR_START; //starter adc conversion

}

uint16_t adc_read(void)
{
    return ADC->ADC_CDR[7];                  // siste måling fra kanal 7, går fra 0 til 4095 (3,3 V)
}
