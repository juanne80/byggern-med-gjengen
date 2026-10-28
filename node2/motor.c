#include <stdint.h>
#include <stdio.h>
#include "motor.h"
#include "sam.h"
#include "encoder.h"

#define KP     0.35f    // startverdi, juster
#define KI     0.1f    // start med 0, legg til senere
#define KD     0.01f
#define T      0.01f    // 10 ms, samme som timeren
#define U_MAX  100

#define JOY_GAIN  0.4f 
#define DEADZONE  5


void motor_init(void) {
	// Strøm til PIOC (ID 13 → PCER0) og PWM (ID 36 → PCER1)
    PMC->PMC_PCER0 = (1 << ID_PIOC);
    PMC->PMC_PCER1 = (1 << (ID_PWM - 32));

    // PC23 = retning, vanlig utgang
    PIOC->PIO_PER = PIO_PC23;
    PIOC->PIO_OER = PIO_PC23;

    PIOB->PIO_PDR   = PIO_PB12; //Deaktiverer at pb12 styres av pio
    PIOB->PIO_ABSR |= PIO_PB12; //Lar periferi B (PMWH0) styre

    PWM->PWM_CH_NUM[0].PWM_CMR  = PWM_CMR_CPRE_MCK | PWM_CMR_CPOL; //bruker hovedklokken, setter klokkepolaritet til å starte høyt
    PWM->PWM_CH_NUM[0].PWM_CPRD = 4200; //Setter perioden til 4200 skritt = 20 kHz (må være mellom 10-50kHz)
    PWM->PWM_CH_NUM[0].PWM_CDTY = 0; // start stille
    PWM->PWM_ENA = PWM_ENA_CHID0; //Skrur på pwm output fra kanal 0

}



void motor_set(int8_t u)
{
    // Sjekker retning
    if (u >= 0) {
        PIOC->PIO_SODR = PIO_PC23; // PC23 høy = en vei
    } else {
        PIOC->PIO_CODR = PIO_PC23; // PC23 lav = motsatt vei
        u = -u;                      // gjør u positiv
    }

    if (u > 100) {
        u = 100;
    }

    PWM->PWM_CH_NUM[0].PWM_CDTYUPD = (4200*u)/100; //Setter dutycycle til 4200*prosent
}

int8_t pi_regulator(int32_t pos, int32_t ref)
{
    static float integral = 0;
    static int32_t last_pos = 0;        
    static float d_filter = 0;           

    int32_t e = pos - ref;
    integral += e * T;

    // Anti-windup
    float i_max = U_MAX / KI;
    if (integral >  i_max) integral =  i_max;
    if (integral < -i_max) integral = -i_max;

    float speed = (pos - last_pos) / T;   // ny: fart i skritt per sekund
    last_pos = pos;                       // ny
    d_filter = 0.7f * d_filter + 0.3f * speed; // ny: glatter ut farten

    float u = KP*e + KI*integral + KD*d_filter;   // endret: + KD*d_filt

    if (u >  U_MAX) u =  U_MAX;
    if (u < -U_MAX) u = -U_MAX;

    return (int8_t)u;
}

static volatile int8_t joy = 0;

static float reference = 0; //Begynner til siden, på 0

void motor_on_tick(void)                       // hvert 10. ms (timer_driver.c)
{
    if (joy > DEADZONE || joy < -DEADZONE) {
        reference += joy * JOY_GAIN;
    }

    if (reference < 0) reference = 0;
    if (reference > 2645) reference = 2645;

    int8_t u = pi_regulator(encoder_read(), reference);
    motor_set(u);
}


void motor_update_joystick(int8_t new_joystick) {
	joy = new_joystick;
    printf("joy=%d ref=%d pos=%d\r\n", new_joystick, (int)reference, (int)encoder_read());
}
