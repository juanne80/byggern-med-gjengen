#include "sam.h"
#include "ir_driver.h"
 
 
// Turns on the ADC in free running mode on channel 7, so it measures continuously.
void adc_init(void)
{
    // Clock to the ADC. ID_ADC is 37, above 31, so it is in PCER1
    PMC->PMC_PCER1 = (1 << (ID_ADC - 32));
 
    ADC->ADC_MR = ADC_MR_FREERUN_ON          // new conversion as soon as one is done
                | ADC_MR_PRESCAL(2);         // ADC clock = 84 MHz / ((2+1)*2) = 14 MHz (max 22 MHz) p. 1403 in arduino Due datasheet
 
    ADC->ADC_CHER = ADC_CHER_CH7;            // enable channel 7 (A0 / PA16) p. 40 in arduino Due datasheet
    ADC->ADC_CR   = ADC_CR_START;            // start converting
}
 
 
// Returns the latest measurement from the IR receiver, 0-4095 (0-3.3 V). Because of our choice of recistance, we get values in the range of 0-1300 ish
uint16_t adc_read(void)
{
    return ADC->ADC_CDR[7];
}
