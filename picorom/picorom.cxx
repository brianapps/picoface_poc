#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/pio.h"
#include "pico/cyw43_arch.h"
#include "hardware/structs/bus_ctrl.h"
#include "usb_command_handler.h"


#include "http_server.h"
#include "littlefs-lib/pico_hal.h"


#include "blink.pio.h"

#define LZ4_STATIC_LINKING_ONLY
#include "lz4.h"

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
   // "   str r0, [ %[base], %[sm1_tx_offset] ]\n"
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
    //"   str r0, [ %[base], %[sm1_tx_offset] ]\n"
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
    //sm_config_set_out_shift(&sm_config, true, true, 8);
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

#define SNA_SIZE 49179
#define SNA_LOAD_SIZE (LZ4_DECOMPRESS_INPLACE_BUFFER_SIZE(SNA_SIZE))

static char sna_buffer[49179];
static char sna_load_buffer[SNA_LOAD_SIZE + 1024];

// extern const uint8_t MANIC_DATA[];
// extern const uint32_t MANIC_SIZE;

// extern const uint8_t KNIGHT_DATA[];
// extern const uint32_t KNIGHT_SIZE;

#define ACTION_BEGIN_SNA_READ 0x01
#define ACTION_SNA_READ_NEXT 0x02
#define ACTION_SNA_BEGIN_WRITE 0x03
#define ACTION_SNA_NEXT_WRITE 0x04
#define ACTION_SNA_LIST 5


const char* current_sna_data = NULL;
uint32_t current_sna_offset =  0;
uint32_t current_sna_size = 0;

uint32_t current_sna_write_offset = 0;


void load_snapshot_file(const char* fileAndExt) {
    char filename[128];
    int filePartLen = strlen(fileAndExt);
    memcpy(filename, fileAndExt, filePartLen);
    filename[filePartLen] = '.';
    strcpy(filename + filePartLen + 1, fileAndExt + filePartLen + 1);
    printf("File name and extension: %s\n", filename);
    int file = pico_open(filename, LFS_O_RDONLY);
    size_t file_size = pico_size(file);
    pico_read(file, sna_load_buffer + SNA_LOAD_SIZE - file_size, file_size);
    pico_close(file);
    int res = LZ4_decompress_safe_partial(sna_load_buffer + SNA_LOAD_SIZE - file_size, sna_load_buffer, file_size, SNA_SIZE, SNA_SIZE);
    printf("Decompress result: %d, original size: %d\n", res, file_size);

    current_sna_data = sna_load_buffer;
    current_sna_size = SNA_SIZE;            
}

bool stringendswith(const char* str, const char* suffix) {
    int suffixlen = strlen(suffix);
    int len = strlen(str);
    if (suffixlen > len) {
        return false;
    }
    return strcasecmp(str + len - suffixlen, suffix) == 0;
}

void nmi_action_list_sna() {
    uint8_t* nmi_rom_data = rom_data + 16384;
    uint16_t start =  (nmi_rom_data[3] << 8) |  nmi_rom_data[2];
    uint16_t destoffset =  (nmi_rom_data[5] << 8) |  nmi_rom_data[4];
    uint16_t stringoffset = destoffset + 22;

    printf("List snaps starting at %d and storing at %X\n", start, destoffset);

    int dir = pico_dir_open("/");
    int16_t foundsofar = 0;
    struct lfs_info info;

    nmi_rom_data[destoffset] = 0;
    nmi_rom_data[destoffset + 1] = 0;

    while (pico_dir_read(dir, &info) > 0) {
        if (info.type == LFS_TYPE_REG) {
            int filenamelen = strlen(info.name);
            if (filenamelen > 5 && strcasecmp(info.name + filenamelen - 5, ".snaz") == 0) {
                if (foundsofar >= start) {
                    int index = foundsofar - start;
                    if (index >= 10) {
                        nmi_rom_data[destoffset + 1] = 1;
                        break;
                    }
                    nmi_rom_data[destoffset] = index + 1;
                    nmi_rom_data[destoffset + 2 + 2 * index] = stringoffset & 0xFF;
                    nmi_rom_data[destoffset + 2 + 2 * index + 1] = (stringoffset >> 8) & 0xFF;

                    printf("Listing %s\n", info.name);
                    strcpy(reinterpret_cast<char*>(nmi_rom_data) + stringoffset, info.name);
                    nmi_rom_data[stringoffset + filenamelen - 5] = '\0';
                    stringoffset += filenamelen + 1;
                }

                foundsofar++;
            }
        }
    }

    



    pico_dir_close(dir);


}


void process_nmi_request() {
    uint8_t* nmi_rom_data = rom_data + 16384;
    uint8_t action = nmi_rom_data[1];
    printf("Action to process: %d\n", action);

    for (int i = 0; i < 16; ++i) {
        printf("%02X ", nmi_rom_data[i]);
    }
    printf("\n");

    if (action == ACTION_BEGIN_SNA_READ) {
        uint32_t nameoffset = (nmi_rom_data[3] << 8) |  nmi_rom_data[2];
        uint32_t headeroffset =  (nmi_rom_data[5] << 8) |  nmi_rom_data[4];

        printf("Loading %s\n", nmi_rom_data + nameoffset);
        load_snapshot_file(reinterpret_cast<const char*>(nmi_rom_data) + nameoffset);

        printf("Start SNA, head destination: %X\n", headeroffset);
        // for (int i = 0; i < 27; i++)
        //     nmi_rom_data[headeroffset + i] = current_sna_data[i];
        memcpy(nmi_rom_data + headeroffset, current_sna_data, 27);
        //memcpy(nmi_rom_data + headeroffset, current_sna_data, 27);
        current_sna_offset = 27;
    }
    else if (action == ACTION_SNA_READ_NEXT) {
        uint16_t offset =  (nmi_rom_data[3] << 8) |  nmi_rom_data[2];
        uint16_t count =  (nmi_rom_data[5] << 8) |  nmi_rom_data[4];

        uint32_t tocopy = MIN(count, current_sna_size - current_sna_offset);

        printf("Continue SNA, destination: %X, count: %X\n", offset, tocopy);

        memcpy(nmi_rom_data + offset, current_sna_data + current_sna_offset, tocopy);

        current_sna_offset += tocopy;

        nmi_rom_data[4] = tocopy & 0xFF;
        nmi_rom_data[5] = (tocopy >> 8) & 0xFF;

    }  else if (action == ACTION_SNA_BEGIN_WRITE) {
        uint16_t headeroffset =  (nmi_rom_data[3] << 8) |  nmi_rom_data[2];
        printf("Start SNA save, head destination: %X\n", headeroffset);
        memcpy(sna_buffer, nmi_rom_data + headeroffset, 27);
        current_sna_write_offset = 27;
    }  else if (action == ACTION_SNA_NEXT_WRITE) {
        uint16_t offset =  (nmi_rom_data[3] << 8) |  nmi_rom_data[2];
        uint16_t count =  (nmi_rom_data[5] << 8) |  nmi_rom_data[4];
        uint32_t tocopy = MIN(count, sizeof(sna_buffer) - current_sna_write_offset);
        printf("Continue SNA save, offset: %X, length = %X; tocopy = %X\n", offset, count, tocopy);
        memcpy(sna_buffer + current_sna_write_offset, nmi_rom_data + offset, tocopy);
        current_sna_write_offset += tocopy;
    } else if (action == ACTION_SNA_LIST) {
        nmi_action_list_sna();
    }
    nmi_rom_data[0] = 0;
}

void init_file_system() {
       if (pico_mount(false) != LFS_ERR_OK) {
        printf("Mount failed, try formatting\n");
        if (pico_mount(true) != LFS_ERR_OK) {
            printf("Mount failed, after formatting\n");
        }
    }
}


//#define ENABLE_WIFI

int main() {



    stdio_init_all();

    bus_ctrl_hw->priority = BUSCTRL_BUS_PRIORITY_PROC1_BITS;


    memcpy(rom_data + 16384, NMI_ROM, NMI_ROM_SIZE);

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


    sleep_ms(200);
    init_file_system();

    #ifdef ENABLE_WIFI
    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi...\n");

    if (cyw43_arch_wifi_connect_async(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK) != 0) {
        printf("failed to connect.\n");
        return 1;
    }

    #endif

    printf("\xAB""Busctrl->priority %X\n", bus_ctrl_hw->priority);

    uint32_t c = 0;

    uint8_t* nmi_rom_data = rom_data + 16384;

    bool areConnected = false;


    while (true) {
        #ifdef ENABLE_WIFI
        #if PICO_CYW43_ARCH_POLL
                // if you are using pico_cyw43_arch_poll, then you must poll periodically from your
                // main loop (not from a timer) to check for Wi-Fi driver or lwIP work that needs to be done.
                cyw43_arch_poll();
                // you can poll as often as you like, however if you have nothing else to do you can
                // choose to sleep until either a specified time, or cyw43_arch_poll() has work to do:
                //cyw43_arch_wait_for_work_until(make_timeout_time_ms(2));
        #else
                // if you are not using pico_cyw43_arch_poll, then WiFI driver and lwIP work
                // is done via interrupt in the background. This sleep is just an example of some (blocking)
                // work you might be doing.
        #endif
        #endif
        sleep_ms(1);
        pollUsbCommandHandler();

        #ifdef ENABLE_WIFI
        bool linkup = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP;
        if (linkup != areConnected) {
            printf("WIFI connected: %d\n", linkup);
            if (linkup) {
                start_http_server();
            }
            areConnected = linkup;
        }
        #endif

        if (nmi_rom_data[0] == 255) {
            process_nmi_request();
        }

        bool isup = gpio_get(PIN_USER_SWITCH);
        if (!isup) {

            uint32_t count = 0;

            for (count = 0; count < 1000 && !gpio_get(PIN_USER_SWITCH); count++) {
                sleep_ms(1);
            }

            if (count < 1000) {
                printf("Try to send nmi: %d\n", count);
                memcpy(rom_data + 16384, NMI_ROM, NMI_ROM_SIZE);

                if (((rom_state.flags & 0x2) != 0) || rom_state.nmi_active) {
                    printf("Not issuing NMI because one is already active\n");
                }
                else {
                    nmi_rom_data[0] = 0;
                    rom_state.flags |= 2;
                    gpio_put(PIN_NMI, false);
                    sleep_ms(3);
                    gpio_put(PIN_NMI, true);
                }
            }
            else {
                printf("Switching ROM now..\n");
                memcpy(rom_data, FGH_ROM, FGH_ROM_SIZE);
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

