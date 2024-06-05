#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "blink.pio.h"


const uint8_t PIN_D0 = 11;
const uint8_t PIN_RESET = 19;

// set high to disable rom.
// When low /CSROM is floating but when high this pulls /CSROM to GND and
// therefore gives the pico control of the data lines when the Z80 is 
// accessing memory in the lower 16KB.
const uint8_t PIN_CSROM = 20; 
const uint8_t PIN_PICOREQ = 21;
const uint8_t PIN_XE = 22;


const uint MY_SM = 0;
const uint SM_OUTDATA = 1;


void __time_critical_func(do_my_pio)() {
    gpio_init(PIN_RESET);
    gpio_put(PIN_RESET, false);
    gpio_set_dir(PIN_RESET, true);

    
    PIO pio = pio0;
    pio_sm_claim(pio, MY_SM);
    pio_sm_claim(pio, SM_OUTDATA);
    
    pio_gpio_init(pio, PIN_XE);
    pio_gpio_init(pio, PIN_CSROM);

    for (uint i = 0; i < 8; ++i) {
        pio_gpio_init(pio, PIN_D0 + i);
    }

    pio_sm_set_consecutive_pindirs(pio, MY_SM, PIN_XE, 1, true);
    pio_sm_set_consecutive_pindirs(pio, SM_OUTDATA, PIN_CSROM, 1, true);
    pio_sm_set_consecutive_pindirs(pio, SM_OUTDATA, PIN_D0, 8, false);

    uint offset = pio_add_program(pio, &fetchaddr_program);
    pio_sm_config sm_config = fetchaddr_program_get_default_config(offset);
    sm_config_set_in_pins(&sm_config, 2);
    sm_config_set_in_shift(&sm_config, true, false, 0);
    sm_config_set_sideset_pins(&sm_config, PIN_XE);
    pio_sm_init(pio, MY_SM, offset, &sm_config);
    pio_sm_set_enabled(pio, MY_SM, true);

    offset = pio_add_program(pio, &putdata_program);
    sm_config = putdata_program_get_default_config(offset);
    sm_config_set_set_pins(&sm_config, PIN_CSROM, 1);
    sm_config_set_out_pins(&sm_config, PIN_D0, 8);
    pio_sm_init(pio, SM_OUTDATA, offset, &sm_config);
    pio_sm_set_enabled(pio, SM_OUTDATA, true);    


    uint32_t lastm1 = 0;
    uint32_t c = 'x';

    const uint32_t myaddr = 5433;

    while (true) {
        // Address read from PIO is 32 bits are follow
        // Bits 0-13 = ROM address - mask 0x3FFF
        // Bit 14 = /M1 - mask 0x4000
        // Bit 15 = /WR - mask 0x8000
        // Bit 16-23 = Data
        // Bits 24-31 = Random/Ignored 
        // The external logic circuit ensures addresses are only passed 
        // if /WR is low and /RD is high or vice versa. Therefore is /WR
        // is high then /RD must be low so Bit 14 is 1 for a read and 0 a write.
        uint32_t addr = pio_sm_get_blocking(pio, MY_SM);

        uint32_t romaddr = addr & 0x3FFF;

        // Access is 
        // 1 Write
        // 2 M1 READ
        // 3 Normal read        
        uint8_t access = (addr >> 14) & 3;

        if ((access == 3 || access == 2) && romaddr == myaddr) {
            //pio_sm_put(pio, SM_CSROM, 0);
            pio_sm_put(pio, SM_OUTDATA, c);
            //c++;
        } else if (access == 1 && romaddr == myaddr) {
            c = (addr >> 16) & 0xFF;
        }

        if (access == 2) {
            lastm1 = romaddr;
        } else if (access == 1 && romaddr > 16) // ignore writes to lower access because these happen normally
        {
            // uint8_t writeval = (addr >> 16) & 0xFF;
            // printf("Write at = %d to %d, 0x%X, lastm1 = %X\n", 
            //     romaddr, writeval, addr, lastm1);
            if (romaddr == 1234) {
                printf("Resetting\n");
                gpio_put(PIN_RESET, true);
                sleep_ms(3);
                gpio_put(PIN_RESET, false);
            }

        }

        

        // if (romaddr == 8191) {
        //     printf("Access %X, (%d), lastm1 = %X\n", addr, access, lastm1);
        // }            
    }
}


int main() {
    stdio_init_all();
    sleep_ms(100);
    printf("Started\n");
    do_my_pio();
}

