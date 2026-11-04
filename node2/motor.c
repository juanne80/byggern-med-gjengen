#include <stdint.h>
#include <stdio.h>
#include "motor.h"
#include "sam.h"
#include "encoder.h"

// ---- Regulator ----
static float kp       = 0.35f;
static float ki       = 0.1f;
static float kd       = 0.01f;
static float joy_gain = 0.4f;
static uint8_t open_loop = 0;     // 1 = ingen regulering (extreme)

#define T      0.01f    // 10 ms, samme som timeren
#define U_MAX  100
 
// ---- Joystick -> referanse ----
#define DEADZONE  15
 
// ---- Bane ----
#define POS_MIN   0
#define POS_MAX   2645

// ---- PWM ----
#define MOTOR_PWM_PERIOD 4200   // 84 MHz / 4200 = 20 kHz
 
 
// ================== Tilstand (delt mellom funksjonene) ================== 
static float   integral = 0;           // I-ledd
static int32_t last_pos = 0;           // D-ledd: forrige posisjon
static float   d_filter = 0;           // D-ledd: glattet fart
 
static float   reference = POS_MIN;    // onsket posisjon
static volatile int8_t  joy = 0;       // skrives fra main, leses i avbruddet
static volatile uint8_t regulator_enabled = 1;   // 0 under motor_home()



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


void motor_set_difficulty(uint8_t level)
{
    switch (level) {
        case 0:  // Easy
            joy_gain = 0.4f;  kp = 0.35f; ki = 0.1f; kd = 0.01f; open_loop = 0;
            break;
        case 1:  // Medium
            joy_gain = 0.25f; kp = 0.35f; ki = 0.1f; kd = 0.01f; open_loop = 0;
            break;
        case 2:  // Hard: treg og litt svampete
            joy_gain = 0.15f; kp = 0.15f; ki = 0.1f; kd = 0.0f;  open_loop = 0;
            break;
        case 3:  // Extreme: joystick styrer motoren direkte
        default:
            open_loop = 1;
            break;
    }
    integral = 0;     // ikke ta med gammelt integral inn i nye parametre
    d_filter = 0;
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

int8_t pid_regulator(int32_t pos, int32_t ref)
{
    int32_t e = pos - ref;
 
    // P
    float u = kp * e;
 
    // I (hoppes over hvis KI = 0, ellers deles det pa null)
    if (ki > 0.0f) {
        integral += e * T;
        float i_max = U_MAX / ki;            // anti-windup
        if (integral >  i_max) integral =  i_max;
        if (integral < -i_max) integral = -i_max;
        u += ki * integral;
    }
 
    // D: fart i skritt per sekund, lavpassfiltrert
    float speed = (pos - last_pos) / T;
    last_pos = pos;
    d_filter = 0.7f * d_filter + 0.3f * speed;
    u += kd * d_filter;
 
    // Metning
    if (u >  U_MAX) u =  U_MAX;
    if (u < -U_MAX) u = -U_MAX;
 
    return (int8_t)u;
}


void motor_on_tick(void)                       // hvert 10. ms (timer_driver.c)
{
	if (!regulator_enabled) return;          // motor_home() styrer motoren selv

	if (open_loop) {
    int8_t u = -joy * 0.6;          // maks 60 %, så den ikke smeller i endene
    if (joy > -DEADZONE && joy < DEADZONE) u = 0;
    motor_set(u);
    return;
	}

    if (joy > DEADZONE || joy < -DEADZONE) {
        reference += joy * joy_gain;
    }

    if (reference < POS_MIN) reference = POS_MIN;
    if (reference > POS_MAX) reference = POS_MAX;


    int8_t u = pid_regulator(encoder_read(), (int32_t)reference);
    motor_set(u);
}


void motor_update_joystick(int8_t new_joystick) {
	static uint8_t count = 0;
	joy = new_joystick;
	if (++count >= 25) {                     // print ca. 2 ganger i sekundet
        count = 0;
        printf("joy=%d ref=%d pos=%d\r\n", new_joystick, (int)reference, (int)encoder_read());
    }

}


void motor_home(void)
{
    regulator_enabled = 0;               // regulatoren holder seg unna

    motor_set(40);                       // litt roligere mot kanten
    int32_t last = encoder_read();
    uint8_t still = 0;

    for (uint8_t n = 0; n < 100 && still < 2; n++) {    // maks ca. 10 s
        for (volatile uint32_t i = 0; i < 1200000; i++) { }   // ca. 100 ms
        int32_t now = encoder_read();
        still = (now == last) ? still + 1 : 0;
        last = now;
    }

    motor_set(0);
    encoder_reset();

    reference = POS_MIN;                       // nullstill regulatoren
    integral  = 0;
    last_pos  = 0;
    d_filter    = 0;

    regulator_enabled = 1;               // regulatoren tar over igjen
}