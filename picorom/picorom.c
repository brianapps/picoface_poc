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


// void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq) {
//     blink_program_init(pio, sm, offset, pin);
//     pio_sm_set_enabled(pio, sm, true);

//     printf("Blinking pin %d at %d Hz\n", pin, freq);

//     // PIO counter program takes 3 more cycles in total than we pass as
//     // input (wait for n + 1; mov; jmp)
//     pio->txf[sm] = (125000000 / (2 * freq)) - 3;
// }


/*

void do_my_pio_test() {
    const uint MY_SM = 0;
    PIO pio = pio0;
    pio_sm_claim(pio, MY_SM);
    
    pio_gpio_init(pio, PIN_XE);
    pio_gpio_init(pio, PIN_CSROM);

    for (uint i = 0; i < 7; ++i) {
        pio_gpio_init(pio, PIN_D0 + i);
    }

    pio_sm_set_consecutive_pindirs(pio, MY_SM, PIN_XE, 1, true);
    pio_sm_set_consecutive_pindirs(pio, MY_SM, PIN_CSROM, 1, true);

    
    uint offset = pio_add_program(pio, &fetchaddr_program);
    pio_sm_config c = fetchaddr_program_get_default_config(offset);
    sm_config_set_in_pins(&c, 2);
    sm_config_set_in_shift(&c, true, false, 0);
    sm_config_set_sideset_pins(&c, PIN_XE);
    pio_sm_init(pio, MY_SM, offset, &c);
    pio_sm_set_enabled(pio, MY_SM, true);





    while (true) {
        uint32_t addr = pio_sm_get_blocking(pio, MY_SM);

        for (int i = 0; i < 14; i++) {
            if (addr & (1 << (13 - i))) {
                putchar('1');
            }
            else
                putchar('0');
        }

        printf("\nAddress = %X\n", addr);
        sleep_ms(100);
    }
}

*/


const uint MY_SM = 0;
const uint SM_OUTDATA = 1;
const uint SM_CSROM = 2;


void do_my_pio() {
    gpio_init(PIN_RESET);
    gpio_put(PIN_RESET, false);
    gpio_set_dir(PIN_RESET, true);

    
    PIO pio = pio0;
    pio_sm_claim(pio, MY_SM);
    pio_sm_claim(pio, SM_OUTDATA);
    pio_sm_claim(pio, SM_CSROM);
    
    pio_gpio_init(pio, PIN_XE);
    pio_gpio_init(pio, PIN_CSROM);

    for (uint i = 0; i < 8; ++i) {
        pio_gpio_init(pio, PIN_D0 + i);
    }

    pio_sm_set_consecutive_pindirs(pio, MY_SM, PIN_XE, 1, true);
    pio_sm_set_consecutive_pindirs(pio, SM_CSROM, PIN_CSROM, 1, true);
    pio_sm_set_consecutive_pindirs(pio, SM_OUTDATA, PIN_D0, 8, false);

    {
        uint offset = pio_add_program(pio, &fetchaddr_program);
        pio_sm_config c = fetchaddr_program_get_default_config(offset);
        sm_config_set_in_pins(&c, 2);
        sm_config_set_in_shift(&c, true, false, 0);
        sm_config_set_sideset_pins(&c, PIN_XE);
        pio_sm_init(pio, MY_SM, offset, &c);
        pio_sm_set_enabled(pio, MY_SM, true);
    }

    {
        uint offset = pio_add_program(pio, &putdata_program);
        pio_sm_config c = putdata_program_get_default_config(offset);
        sm_config_set_set_pins(&c, PIN_CSROM, 1);
        sm_config_set_out_pins(&c, PIN_D0, 8);

        pio_sm_init(pio, SM_OUTDATA, offset, &c);
        pio_sm_set_enabled(pio, SM_OUTDATA, true);    
    }


    {
        uint offset = pio_add_program(pio, &disablerom_program);
        pio_sm_config c = disablerom_program_get_default_config(offset);
        sm_config_set_set_pins(&c, PIN_CSROM, 1);
        pio_sm_init(pio, SM_CSROM, offset, &c);
        pio_sm_set_enabled(pio, SM_CSROM, true);
    }



    uint32_t lastm1 = 0;
    uint32_t c = 13;

    while (true) {
        uint32_t addr = pio_sm_get_blocking(pio, MY_SM);

        uint32_t romaddr = addr & 0x3FFF;


        // Access is 
        // 1 Write
        // 2 M1 READ
        // 3 Normal read        
        //
        uint8_t access = (addr >> 14) & 3;

        if (access == 3 && romaddr == 66) {
            //pio_sm_put(pio, SM_CSROM, 0);
            pio_sm_put(pio, SM_OUTDATA, c);
            c++;
        } else if (access == 1 && romaddr == 66) {
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

void do_test_csrom_pin() {
    PIO pio = pio0;
    pio_sm_claim(pio, SM_CSROM);
    pio_gpio_init(pio, PIN_CSROM);
    pio_sm_set_consecutive_pindirs(pio, SM_CSROM, PIN_CSROM, 1, true);

    uint offset = pio_add_program(pio, &disablerom_program);
    pio_sm_config c = disablerom_program_get_default_config(offset);
    sm_config_set_set_pins(&c, PIN_CSROM, 1);
    pio_sm_init(pio, SM_CSROM, offset, &c);
    pio_sm_set_enabled(pio, SM_CSROM, true);

    pio_sm_put_blocking(pio, SM_CSROM, 0);
    pio_sm_get_blocking(pio, SM_CSROM);


    while (true) {
        sleep_ms(1000);
        printf("tick\n");
    }

}


void do_data_pins_test() {
    // gpio_init(PIN_D0);
    // gpio_set_dir(PIN_D0, true);

    PIO pio = pio0;
    pio_sm_claim(pio, MY_SM);
    pio_sm_claim(pio, SM_OUTDATA);
    pio_sm_claim(pio, SM_CSROM);
    
    pio_gpio_init(pio, PIN_XE);
    pio_gpio_init(pio, PIN_CSROM);

    for (uint i = 0; i < 8; ++i) {
        pio_gpio_init(pio, PIN_D0 + i);
    }

    pio_sm_set_consecutive_pindirs(pio, SM_OUTDATA, PIN_D0, 8, false);


    {
        uint offset = pio_add_program(pio, &flash_program);
        pio_sm_config c = flash_program_get_default_config(offset);
        sm_config_set_out_pins(&c, PIN_D0, 8);

        pio_sm_init(pio, SM_OUTDATA, offset, &c);
        pio_sm_set_enabled(pio, SM_OUTDATA, true);    
    }


    while (true) {
        pio_sm_put_blocking(pio, SM_OUTDATA, 0xFFFF);
        sleep_ms(500);
        pio_sm_put_blocking(pio, SM_OUTDATA, 0);
        sleep_ms(500);
        printf("tick\n");
    }

}


int main() {
    stdio_init_all();
    sleep_ms(1000);
    printf("Started\n");
    do_my_pio();
    //do_test_csrom_pin();
    //do_data_pins_test();
}


/*
int maina()
{
    stdio_init_all();

    gpio_init(PIN_XE);
    //gpio_disable_pulls(PIN_XE);
    gpio_set_dir(PIN_XE, true);

    // // PIO Blinking example
    // PIO pio = pio0;
    // uint offset = pio_add_program(pio, &blink_program);
    // printf("Loaded program at %d\n", offset);
    
    // #ifdef PICO_DEFAULT_LED_PIN
    // blink_pin_forever(pio, 0, offset, PICO_DEFAULT_LED_PIN, 3);
    // #else
    // blink_pin_forever(pio, 0, offset, 6, 3);
    // #endif
    // // For more pio examples see https://github.com/raspberrypi/pico-examples/tree/master/pio

    bool xe = true;

    while (true) {
        printf("Hello, world %d!\n", xe);
        gpio_put(PIN_XE, xe);

        sleep_ms(1000);
        xe = !xe;
    }
}
*/

