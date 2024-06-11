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
const uint8_t PIN_NMI = 27;

const uint MY_SM = 0;
const uint SM_OUTDATA = 1;


static queue_t eventqueue;


struct MYSTATE {
    uint32_t flags;
    uint32_t nmi_active;
    uint32_t writable;
};

volatile struct MYSTATE rom_state = {0};
uint8_t rom_data[32768];

// The RETN instruction is two bytes (ED 45) so the exit address is the 
// for the second byte of the instruction.
extern const uint32_t EXITNMI;
extern const uint8_t NMI_ROM[];
extern const uint32_t NMI_ROM_SIZE;

void __time_critical_func(serverom)() {
    //register unsigned nmi_exit __asm("r10") = (1 << 14 | 0x80) << 17;
    //register unsigned nmi_entry __asm("r11") = (1 << 14 | 0x66) << 17;
    // r9 holds previous romstate
    // r8 holds nmiexit address shifted, or 1 if disabled
    asm(
    "   mov r0, #1\n"
    "   mov r8, r0\n"
    "   mov r4, %[rom_data]\n"
    "main_loop:\n"
    "   ldr r1,=%[addr_mask]\n"
    "check_fifo_empty:\n"
    "   ldr r0, [ %[base], %[fstat_offset]]\n"
    "   lsr r0, r0, %[rx0empty_shift]\n"
    "   bcs check_fifo_empty\n"
    "   ldr r0, [ %[base], %[sm0_rx_offset]]\n"   // r0 now holds data and address
    "   and r1, r0, r1\n"               // r1 now holds just address
    "   lsl r2, r0, #17\n"                        // carry flag is set for a read, anc clear for a write
    "   bcc write_op\n"
    "   ldr r3, [ %[rom_state], #0]\n"            // r3 holds rom_sate
    "   lsr r3, r3, #1\n"                         // CC is set if rom enable, zero flag is NMI check is enabled
    "   bcc read_rom_disabled\n"
    "   beq send_data\n"
    "   cmp r2, %[nmiaddr]\n"
    "   beq activate_nmi\n"
    "send_data:\n"
    "   ldrb r0, [r4, r1]\n"
    "   str r0, [ %[base], %[sm1_tx_offset] ]\n"
    // Check if have reached the exit point of the nmi routine
    "   cmp r2, r8\n"
    "   bne main_loop\n"

    // leaving nmi rom now
    "   mov r0, r9\n"
    "   str r0, [%[rom_state], #0]\n"      // restore the rom_state to what is was prior to entering the nmi routine
    "   mov r0, #1\n"                      // r8 is set to an invalid shifted address so we have effectively disabled
    "   mov r8, r1\n"                      // the nmi exit check from now on
    "   mov r4, %[rom_data]\n"             // Restore the rom address to the normal rom, regardless of whether the rom is enabled or not
    // And need to set a variable to say we aren't in the nmi anymore
    "   mov r0, #0\n"
    "   str r0, [%[rom_state], #4]\n"    // no longer in nmi
    "   b main_loop\n"


    "activate_nmi:\n"
    "   mov r0, #0\n"                              // always serve a NOP as first NMI instruction, this saves a read instruction
    "   str r0, [ %[base], %[sm1_tx_offset] ]\n"  //
    "   mov r0, #1\n"
    "   mov r9, r0\n"                            // previous romstate was enabled
    "   str r0, [%[rom_state], #4]\n"            // nmi routine is now active
    "   str r0, [%[rom_state], #0]\n"            // rom is enabled, because it is the nmi rom
    "   mov r8, %[nmiexit]\n"                    // and set r8 to nmi exit address
    "   ldr r4, =%[rom_data_nmi]\n"
    "   b main_loop\n"

    "read_rom_disabled:\n"
    "   beq main_loop\n"   // zero if nmi check is not required
    "   cmp r2, %[nmiaddr]\n"
    "   bne main_loop\n"
    "   mov r0, #0\n"                              // always serve a NOP as first NMI instruction, this saves a read instruction
    "   str r0, [ %[base], %[sm1_tx_offset] ]\n"  //
    "   mov r9, r0\n"                            // previous romstate was disabled
    "   mov r1, #1\n"
    "   str r1, [%[rom_state], #4]\n"            // we are now in nmi routine
    "   str r1, [%[rom_state], #0]\n"            // and the rom is enabled
    "   mov r8, %[nmiexit]\n"                    // and set r8 to nmi exit address
    "   mov r4, %[rom_data]\n"
    "   mov r0, #1\n"
    "   lsl r0, #14\n"
    "   add r4, r4, r0\n"
    "   b main_loop\n"
    "write_op:\n"
    // r0 is data and address
    // writes are allowed if we using the nmi rom ..
    "   cmp r8, %[nmiexit]\n"
    "   bne main_loop\n"
    // of if configured to allow it ..
    // "   ldr r2, [%[rom_state], #8 ]\n"
    // "   cmp r2, #0\n"
    // "   beq main_loop\n"
    // // and address above 16 (because the normal spectrum rom writes 
    // // to some lower locations and we want to ignore them)
    // "   cmp r1, #16\n"
    // "   blt main_loop\n"
    // "perform_write:"
    "   lsr r0, r0, #16\n"
    "   strb r0, [r4, r1]\n"
    "   b main_loop\n"
    :
    : [base] "r" (PIO0_BASE), 
      [nmiaddr] "h" ((0 << 14 | 0x66) << 17),
      [nmiexit] "h" ((0 << 14 | EXITNMI) << 17),
      [fstat_offset] "I" (PIO_FSTAT_OFFSET),
      [sm0_rx_offset] "I" (PIO_RXF0_OFFSET),
      [sm0_tx_offset] "I" (PIO_TXF0_OFFSET),
      [sm1_tx_offset] "I" (PIO_TXF0_OFFSET + 4),
      [rx0empty_shift] "I" (PIO_FSTAT_RXEMPTY_LSB + 1),
      [addr_mask] "i" (0x3FFF),
      [rom_state] "r" (&rom_state),
      [rom_data]  "h" (rom_data),
      [rom_data_nmi]  "i" (rom_data + 16384)
      
    : "r0", "r2", "r1", "r3", "r4", "r8", "r9"
    );

    return;
}



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
    sm_config_set_in_shift(&sm_config, true, true, 32);
    sm_config_set_sideset_pins(&sm_config, PIN_XE);


    pio_sm_init(pio, MY_SM, offset, &sm_config);
    pio_sm_set_enabled(pio, MY_SM, true);

    #if 0
    offset = pio_add_program(pio, &putdata_auto_program);
    sm_config = putdata_auto_program_get_default_config(offset);
    sm_config_set_sideset_pins(&sm_config, PIN_CSROM);
    sm_config_set_out_pins(&sm_config, PIN_D0, 8);
    sm_config_set_out_shift(&c, true, true, 8);
    pio_sm_init(pio, SM_OUTDATA, offset, &sm_config);
    pio_sm_set_enabled(pio, SM_OUTDATA, true);

    #else
    offset = pio_add_program(pio, &putdata_program);
    sm_config = putdata_program_get_default_config(offset);
    sm_config_set_sideset_pins(&sm_config, PIN_CSROM);
    sm_config_set_out_pins(&sm_config, PIN_D0, 8);
    pio_sm_init(pio, SM_OUTDATA, offset, &sm_config);
    pio_sm_set_enabled(pio, SM_OUTDATA, true);
    #endif

    serverom();
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


    memcpy(rom_data + 16384 + 0x66, NMI_ROM, NMI_ROM_SIZE);

    rom_state.flags = 0;

    multicore_launch_core1(do_my_pio);

    gpio_init(PIN_USER_SWITCH);
    gpio_set_dir(PIN_USER_SWITCH, false);
    gpio_pull_up(PIN_USER_SWITCH);
    //gpio_set_irq_enabled_with_callback(PIN_USER_SWITCH, GPIO_IRQ_EDGE_FALL, true, user_switch_interrupt);

    gpio_init(PIN_RESET);
    gpio_put(PIN_RESET, false);
    gpio_set_dir(PIN_RESET, true);    

     gpio_init(PIN_NMI);
     gpio_put(PIN_NMI, true);
     gpio_set_dir(PIN_NMI, true);    


    sleep_ms(1000);
    printf("Started %d\n", FGH_ROM_SIZE);

    uint32_t c = 0;

    uint8_t* nmi_rom_data = rom_data + 16384;


    while (true) {
        //uint8_t event;
        //queue_remove_blocking(&eventqueue, &event);
        //printf("Queue popped %d\n", event);
        sleep_ms(2);

        bool isup = gpio_get(PIN_USER_SWITCH);
        if (!isup) {

            uint32_t count = 0;

            for (count = 0; count < 1000 && !gpio_get(PIN_USER_SWITCH); count++) {
                sleep_ms(1);
            }

            if (count < 1000) {
                printf("Try to send nmi: %d\n", count);

                if (((rom_state.flags & 0x2) != 0) || rom_state.nmi_active) {
                    printf("Not issuing NMI because one is already active\n");
                }
                else {
                    rom_state.flags |= 2;
                    gpio_put(PIN_NMI, false);
                    sleep_ms(3);
                    gpio_put(PIN_NMI, true);
                    // sleep_ms(2000);

                    // uint8_t* scr = rom_data + 16384 + 0x2000;

                    // for (uint l = 0; l < 108; l++) {
                    //     for (uint c = 0; c < 64; c++) {
                    //         printf("%02X", *scr);
                    //         scr++;
                    //     }
                    //     printf("\n");
                    // }

                }
            }
            else {
                printf("Switching ROM now..\n");
                memcpy(rom_data, FGH_ROM, FGH_ROM_SIZE);
                //rom_data[5433] = 'a' + c % 32;
                c++;
                rom_state.flags = (rom_state.flags & 1) ? 0 : 1;
                rom_state.writable = 0;
                printf("Resetting %d\n", rom_state.flags);

                gpio_put(PIN_RESET, true);
                sleep_ms(3);
                gpio_put(PIN_RESET, false);    

               while (!gpio_get(PIN_USER_SWITCH))
                    sleep_ms(1);
            }
        }
    }
    
}

