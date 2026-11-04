#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
 
void    encoder_init(void);
int32_t encoder_read(void);
void    encoder_reset(void);
 
#endif
