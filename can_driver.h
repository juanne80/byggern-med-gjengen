#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint16_t id;       // 11-bit standard-ID (0x000–0x7FF)
    uint8_t  length;   // antall databyter, 0–8
    uint8_t  data[8];
} can_message;


uint8_t can_init(void);
uint8_t can_send(can_message *msg);
uint8_t can_receive(can_message *msg);

