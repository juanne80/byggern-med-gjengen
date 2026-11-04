#include "pwm_driver.h"
#include "sam.h"
#include "../node1/io_driver.h"
#include "can.h"
#include "timer_driver.h"


void pwm_init(void)
{
    PMC->PMC_PCER1 = (1 << (ID_PWM - 32)); // ID starter på 32, må derfor skiftes (ID-32), aktiverer klokken til ID

    PIOB->PIO_PDR   = PIO_PDR_P13; // Fjerner at PIO kontrollerer PB13
    PIOB->PIO_ABSR |= PIO_ABSR_P13;   // PB13 skal styres av PWM (periferi B)

    PWM->PWM_CLK = PWM_CLK_PREA(1) | PWM_CLK_DIVA(42); //Setter CLKA til 1 MHz

    PWM->PWM_CH_NUM[1].PWM_CMR  = PWM_CMR_CPRE_CLKA | PWM_CMR_CPOL;  // kanal 1 bruker 1 MHz-klokken, og vi pulsen starter som høy
    PWM->PWM_CH_NUM[1].PWM_CPRD = 20000;              // periode 20 ms
    PWM->PWM_CH_NUM[1].PWM_CDTY = 1500;               // start i midten

    PWM->PWM_ENA = PWM_ENA_CHID1;                     // aktiver kanal 1
}

void pwm_set_pulse_us(uint32_t pulse_us)
{
    if (pulse_us < 900)  pulse_us = 900;    // aldri under 0,9 ms
    if (pulse_us > 2100) pulse_us = 2100;   // aldri over 2,1 ms

    PWM->PWM_CH_NUM[1].PWM_CDTYUPD = pulse_us;   // byttes ved slutten av perioden
}


void joystick_to_PWM(int8_t xpercentage){
	uint32_t dutyCycle = (uint32_t)(xpercentage*6 + 1500);
	if (xpercentage > -10 && xpercentage < 10) dutyCycle = 1500;
    

	pwm_set_pulse_us(dutyCycle);
	
}
