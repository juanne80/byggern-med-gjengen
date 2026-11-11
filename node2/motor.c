#include <stdint.h>
#include <stdio.h>
#include "motor.h"
#include "sam.h"
#include "encoder.h"

// Controller parameters (changed by motor_set_difficulty)
static float kp       = 0.35f;
static float ki       = 0.1f;
static float kd       = 0.01f;
static float joy_gain = 0.4f;		// encoder steps per tick per joystick %
static uint8_t open_loop = 0;     	// 1 = no regulation (extreme!!!!!!!!)

#define T      0.01f    // 10 ms sample time, same as the timer
#define U_MAX  100		 // max motor output, %
 
#define DEADZONE  15	// joystick values closer to 0 than this are ignored
 
// Limits for the reference, in encoder steps from the home position
#define POS_MIN   0
#define POS_MAX   2645

#define MOTOR_PWM_PERIOD 4200   // 84 MHz / 4200 = 20 kHz (the driver needs < 50 kHz)
 
 
// State shared between the functions
static float   integral  = 0;                    // I term
static int32_t last_pos  = 0;                    // D term: position at the previous tick
static float   d_filter  = 0;                    // D term: filtered speed
 
static float   reference = POS_MIN;              // wanted position
static volatile int8_t  joy = 0;                 // written from main, read in the interrupt
static volatile uint8_t regulator_enabled = 1;   // 0 while motor_home() runs



// Sets up the direction pin (PC23) and PWM channel 0 on PB12, with the motor stopped.
void motor_init(void) {
    // Clock to PIOC (ID 13 -> PCER0) and PWM (ID 36 -> PCER1)
    PMC->PMC_PCER0 = (1 << ID_PIOC);
    PMC->PMC_PCER1 = (1 << (ID_PWM - 32));
 
    // PC23 = direction, normal output
    PIOC->PIO_PER = PIO_PC23;
    PIOC->PIO_OER = PIO_PC23;
 
    // PB12 is controlled by peripheral B (PWMH0) instead of the PIO
    PIOB->PIO_PDR   = PIO_PB12;
    PIOB->PIO_ABSR |= PIO_PB12;


    PWM->PWM_CH_NUM[0].PWM_CMR  = PWM_CMR_CPRE_MCK | PWM_CMR_CPOL; //uses main clock, set clock polarity to start high
    PWM->PWM_CH_NUM[0].PWM_CPRD = 4200; // sets the period to 4200 steps = 20 kHz (must be between 10-50kHz)
    PWM->PWM_CH_NUM[0].PWM_CDTY = 0; // start with 0% duty cycle (motor off)
    PWM->PWM_ENA = PWM_ENA_CHID0; //turn on PWM from channel 0

}


void motor_set_difficulty(uint8_t level)
{
    switch (level) {
        case 0:  // Easy
            joy_gain = 0.4f;  kp = 0.35f; ki = 0.1f; kd = 0.01f; open_loop = 0;
            break;
        case 1:  // Medium: slow
            joy_gain = 0.25f; kp = 0.35f; ki = 0.1f; kd = 0.01f; open_loop = 0;
            break;
        case 2:  // Hard: more unstable
            joy_gain = 0.25f; kp = 0.15f; ki = 0.2f; kd = 0.0f;  open_loop = 0;
            break;
        case 3:  // Extreme: joystick control only, no regulation
			open_loop = 1;
			break;
        default:
            open_loop = 1;
            break;
    }
    integral = 0;     // don't use old integral in new parameters
    d_filter = 0;
}

// Drives the motor with u from -100 to 100 %. The sign gives the direction
void motor_set(int8_t u)
{
    // Checks direction
    if (u >= 0) {
        PIOC->PIO_SODR = PIO_PC23; // PC23 high = one way
    } else {
        PIOC->PIO_CODR = PIO_PC23; // PC23 low = opposite way
        u = -u;                    
    }

    if (u > 100) {
        u = 100;
    }

    PWM->PWM_CH_NUM[0].PWM_CDTYUPD = (4200*u)/100; //set duty cycle to 4200*percent
}

// PID controller. Returns the motor output (-100 to 100 %) that moves pos towards ref.
int8_t pid_regulator(int32_t pos, int32_t ref)
{
    int32_t e = pos - ref;
 
    // P
    float u = kp * e;
 
    // I (skips if KI = 0, otherwise it divides by zero)
    if (ki > 0.0f) {
        integral += e * T;
        float i_max = U_MAX / ki;            // anti-windup
        if (integral >  i_max) integral =  i_max;
        if (integral < -i_max) integral = -i_max;
        u += ki * integral;
    }
 
    //  D on the measured speed (steps per second), low-pass filtered because the encoder only counts whole steps
    float speed = (pos - last_pos) / T;
    last_pos = pos;
    d_filter = 0.7f * d_filter + 0.3f * speed;
    u += kd * d_filter;
 
    // Saturation
    if (u >  U_MAX) {
		u =  U_MAX;
    }
    if (u < -U_MAX) {
		u = -U_MAX;
	}
    return (int8_t)u;
}

// Runs every 10 ms from the timer interrupt. The joystick moves the reference, and the PID controller drives the slider towards it.
void motor_on_tick(void)                       
{
	if (!regulator_enabled) {
        return;                         // motor_home() controls the motor itself
    }    

	// Extreme: no controller, joystick -> motor output directly
	if (open_loop) {
		int8_t u = -joy * 0.6;           // max 60 %, so it does not hit the ends too hard
		if (joy > -DEADZONE && joy < DEADZONE) {
			u = 0;
		}
		motor_set(u);
		return;
	}

    if (joy > DEADZONE || joy < -DEADZONE) {
        reference += joy * joy_gain;
    }

	// Keep the reference on the track
    if (reference < POS_MIN) {
		reference = POS_MIN;
    }
    if (reference > POS_MAX) {
		reference = POS_MAX;
    }

    int8_t u = pid_regulator(encoder_read(), (int32_t)reference);
    motor_set(u);
}


// Stores the latest joystick value from node 2's CAN message.
// Prints joystick, reference and position about twice a second for debugging.
void motor_update_joystick(int8_t new_joystick) {
	static uint8_t count = 0;
	joy = new_joystick;
	if (++count >= 25) {                     
        count = 0;
        printf("joy=%d ref=%d pos=%d\r\n", new_joystick, (int)reference, (int)encoder_read());
    }

}

// Uses the touch pad as an absolute position: -100 % to 100 %. In Extreme mode the pad works like the joystick instead.
void motor_update_touch(int8_t percent) {
    if (open_loop) {
        joy = percent;
        return;
    }
    joy = 0;    // the joystick must not move the reference as well
    reference = POS_MIN + (float)(percent + 100) * (POS_MAX - POS_MIN) / 200.0f;
}

// Drives the slider to the end of the track, sets the encoder to 0 there and resets the controller. Can be called both before and after timer_init().
void motor_home(void)
{
    regulator_enabled = 0;              // the controller stays out of the way
 
    motor_set(40);
    int32_t last = encoder_read();
    uint8_t still = 0;
 
    // Stop when the encoder has not moved for two checks in a row (max about 10 s)
    for (uint8_t n = 0; n < 100 && still < 2; n++) {
        for (volatile uint32_t i = 0; i < 1200000; i++) { }    // about 100 ms
        int32_t now = encoder_read();
        if (now == last) {
            still++;
        } else {
            still = 0;
        }
        last = now;
    }


    motor_set(0);
    encoder_reset();

	// Start the controller from a clean state
    reference = POS_MIN;                       
    integral  = 0;
    last_pos  = 0;
    d_filter    = 0;

    regulator_enabled = 1;               
}