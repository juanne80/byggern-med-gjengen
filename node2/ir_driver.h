#ifndef IR_DRIVER_H
#define IR_DRIVER_H

#include <stdint.h>

void adc_init(void);
uint16_t adc_read(void);

#endif