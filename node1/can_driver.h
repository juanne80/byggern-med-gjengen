#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#include <stdint.h>

typedef struct {
    uint16_t id;        // 11-bit standard ID (0x000-0x7FF)
    uint8_t  length;    // number of data bytes, 0-8
    uint8_t  data[8];
} can_message;

uint8_t can_init(uint8_t mode);
uint8_t can_send(can_message *msg);
uint8_t can_receive(can_message *msg);

#endif