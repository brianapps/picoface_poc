#include "pico/stdlib.h"

const uint8_t NMI_ROM[] = {
    0x00, 0xed, 0x73, 0x7a, 0x00, 0x31, 0x00, 0x10, 0xf5, 0x3e, 0x04, 0xd3, 0xfe, 0xf1, 0xed, 0x7b, 
    0x7a, 0x00, 0xed, 0x45, 0x00, 0x00, 
};

const uint32_t NMI_ROM_SIZE = sizeof(NMI_ROM);
const uint32_t EXITNMI = 0x79;
