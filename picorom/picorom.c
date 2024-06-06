#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"
#include "hardware/pio.h"

#include "blink.pio.h"

extern const uint8_t FGH_ROM[];
extern const uint32_t FGH_ROM_SIZE;

// multiplexed address pins
const uint8_t PIN_MA0 = 2;
const uint8_t PIN_MA1 = 3;
const uint8_t PIN_MA2 = 4;
const uint8_t PIN_MA3 = 5;
const uint8_t PIN_MA4 = 6;
const uint8_t PIN_MA5 = 7;
const uint8_t PIN_MA6 = 8;
const uint8_t PIN_NOT_M1 = 9;
const uint8_t PIN_NOT_WR = 10;
const uint8_t PIN_D0 = 11;
const uint8_t PIN_D1 = 12;
const uint8_t PIN_D2 = 13;
const uint8_t PIN_D3 = 14;
const uint8_t PIN_D4 = 15;
const uint8_t PIN_D5 = 16;
const uint8_t PIN_D6 = 17;
const uint8_t PIN_D7 = 18;

const uint8_t PIN_RESET = 19;

// set high to disable rom.
// When low /CSROM is floating but when high this pulls /CSROM to GND and
// therefore gives the pico control of the data lines when the Z80 is 
// accessing memory in the lower 16KB.
const uint8_t PIN_CSROM = 20; 
const uint8_t PIN_PICOREQ = 21;
const uint8_t PIN_XE = 22;

const uint8_t PIN_USER_SWITCH = 26;

const uint MY_SM = 0;
const uint SM_OUTDATA = 1;

uint8_t romdata[32768];
volatile bool romenabled = false;

static queue_t eventqueue;


void __time_critical_func(do_my_pio)() {
    // gpio_init(PIN_RESET);
    // gpio_put(PIN_RESET, false);
    // gpio_set_dir(PIN_RESET, true);

    
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

        if (romenabled) {
            if (access == 3 || access == 2) {
                pio_sm_put(pio, SM_OUTDATA, romdata[romaddr]);
            }
        }
        else {

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
                    // gpio_put(PIN_RESET, true);
                    // sleep_ms(3);
                    // gpio_put(PIN_RESET, false);
                }

            }
        }

    }
}

int64_t alarm_switch_debounce(alarm_id_t id, void* user_data) {
    bool isup = gpio_get(PIN_USER_SWITCH);
    if (!isup) {
        printf("Switching the rom now\n");
        uint8_t data = 1;
        queue_add_blocking(&eventqueue, &data);
        // memcpy(romdata, FGH_ROM, FGH_ROM_SIZE);
        // //romenabled = true;
        // printf("Resetting\n");
        // gpio_put(PIN_RESET, true);
        // sleep_ms(3);
        // // gpio_put(PIN_RESET, false);
    }
    
    return 0;
}

void user_switch_interrupt() {
    uint8_t data = 1;
    queue_add_blocking(&eventqueue, &data);
}


int main() {
    stdio_init_all();

    queue_init(&eventqueue, 1, 16);




    multicore_launch_core1(do_my_pio);

    gpio_init(PIN_USER_SWITCH);
    gpio_set_dir(PIN_USER_SWITCH, false);
    gpio_pull_up(PIN_USER_SWITCH);
    gpio_set_irq_enabled_with_callback(PIN_USER_SWITCH, GPIO_IRQ_EDGE_FALL, true, user_switch_interrupt);

    gpio_init(PIN_RESET);
    gpio_put(PIN_RESET, false);
    gpio_set_dir(PIN_RESET, true);    

    sleep_ms(1000);
    printf("Started %d\n", FGH_ROM_SIZE);

    uint32_t c = 0;


    while (true) {
        uint8_t event;
        queue_remove_blocking(&eventqueue, &event);
        printf("Queue popped %d\n", event);
        sleep_ms(2);

        bool isup = gpio_get(PIN_USER_SWITCH);
        if (!isup) {
            printf("Switching ROM now..\n");
            memcpy(romdata, FGH_ROM, FGH_ROM_SIZE);
            romdata[5433] = 'a' + c % 32;
            c++;
            romenabled = !romenabled;
            printf("Resetting %d\n", romenabled);
            gpio_put(PIN_RESET, true);
            sleep_ms(3);
            gpio_put(PIN_RESET, false);        
        }
    }
    
}

