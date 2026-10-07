#include <stdint.h>
#include <stdio.h>



// CNF1: SJW (bit 7-6) = 1-1, BRP (bit 5-0) = 3
#define CAN_CNF1 ((0 << 6) | 3)                       // 0x03
// CNF2: BTLMODE (bit 7) = 1, SAM (bit 6) = 0,
//       PHSEG1 (bit 5-3) = 7-1, PRSEG (bit 2-0) = 2-1
#define CAN_CNF2 ((1 << 7) | (0 << 6) | (6 << 3) | 1) // 0xB1
// CNF3: PHSEG2 (bit 2-0) = 6-1
#define CAN_CNF3 (5)                                  // 0x05

typedef struct {
    uint16_t id;       // 11-bit standard-ID (0x000–0x7FF)
    uint8_t  length;   // antall databyter, 0–8
    uint8_t  data[8];
} can_message;


uint8_t can_init(void);
uint8_t can_send(can_message *msg);
uint8_t can_receive(can_message *msg);

