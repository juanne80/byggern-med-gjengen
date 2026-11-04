#ifndef SOLENOID_H
#define SOLENOID_H

#include <stdint.h>
 
void solenoid_init(void);
void solenoid_on(void);
void solenoid_off(void);
void solenoid_fire(void);        
void solenoid_tick(void);         
uint8_t solenoid_active(void);     
 
#endif
