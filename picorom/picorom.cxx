#include <stdio.h>
#include <string.h>
#include <stdarg.h>
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

#include "nmi.h"

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
    // bit 0 is set if we are serving a rom from the pico
    // bit 1 is set if we should serve the nmi rom when
    uint32_t flags;
    // non-zero if the nmi rom is active.
    uint32_t nmi_active;
    // The addres in ROM that writes are enabled for. If a ROM is active then
    // we allow writes from this address onwards. For writable ROM we prevent
    // write below address 16 because the normal Spectrum rom does write to
    // these lower locations in a number of places and relies on this having
    // no effect. Setting this value above 16KB effectively disables writes and
    // means if are serving a ROM
    uint32_t writableStartAddress;
    uint32_t flags_on_nmi_exit;
};

volatile struct MYSTATE rom_state = {
    .flags = 0,  .nmi_active=0, .writableStartAddress=32768, .flags_on_nmi_exit= 0
};
uint8_t rom_data[32768];

bool send_nmi_request(uint8_t command, uint16_t param1, uint16_t param2);


static bool enable_logging = true;

void LOG(const char* format, ...) {
    if (enable_logging) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
    }
}


// // The RETN instruction is two bytes (ED 45) so the exit address is the 
// // for the second byte of the instruction.
// extern const uint32_t EXITNMI;
// extern const uint8_t NMI_ROM[];
// extern const uint32_t NMI_ROM_SIZE;

void __time_critical_func(serverom)() {
    // r8 holds nmiexit address shifted, or 1 if disabled
    asm(
    
    "   mov r0, #1\n" 
    "   mov r8, r0\n"                             // R8 is the NMI exit address shift, 1 is an invalid value
    "   lsl r0, r0, #15\n"
    "   mov r10, r0\n"                            // r10 is wriable address start and is initialised to 
                                                  // something outside the address range we handle.
    "   mov r4, %[rom_data]\n"
    "main_loop:\n"
    "   ldr r1,=%[addr_mask]\n"
    "check_fifo_empty:\n"
    "   ldr r0, [ %[base], %[fstat_offset]]\n"
    "   lsr r0, r0, %[rx0empty_shift]\n"
    "   bcs check_fifo_empty\n"
    "   ldr r0, [ %[base], %[sm0_rx_offset]]\n"   // r0 now holds data and address
    "   and r1, r0, r1\n"                         // r1 now holds just address
    "   lsl r2, r0, #17\n"                        // carry flag is set for a read, and clear for a write
    "   bcc write_op\n"
    "   ldr r3, [ %[rom_state], #0]\n"            // r3 holds rom_sate
    "   lsr r3, r3, #1\n"                         // CC is set if rom enable, zero flag is NMI check is enabled
    "   bcc read_rom_disabled\n"
    "   beq send_data\n"
    "   cmp r2, %[nmiaddr]\n"
    "   beq activate_nmi\n"
    "send_data:\n"
    "   ldrb r0, [r4, r1]\n"
    "   str r0, [ %[base], %[sm1_tx_offset] ]\n"   // This is effectively a pio_sm_put call
    // Check if have reached the exit point of the nmi routine
    // allow nmi to exit with either a M1 read or normal read of the exit address
    "   lsl r2, r2, #1\n"
    "   cmp r2, r8\n" // allow nmi to exit with either a M1 read or normal read of the exit address
    "   bne main_loop\n"

    // leaving nmi rom now
    "   ldr r0, [%[rom_state], #12]\n"     // read flags_on_nmi_exit
    "   str r0, [%[rom_state], #0]\n"      // restore the rom_state to what it was prior to entering the nmi routine
    "   ldr r0, [%[rom_state], #8]\n"      // read writableStartAddress
    "   mov r10, r0\n"                      // and put into r9 for later checks 
    "   mov r0, #1\n"                      // r8 is set to an invalid shifted address, because (addr << 18) is never going to
                                           // equal 1. Therefore we have effectively disabled the nmi exit check from now on
    "   mov r8, r1\n"                       
    "   mov r4, %[rom_data]\n"             // Restore the rom address to the normal rom, regardless of whether the rom is enabled or not
    // And need to set a variable to say we aren't in the nmi anymore
    "   mov r0, #0\n"
    "   str r0, [%[rom_state], #4]\n"    // no longer in nmi
    "   b main_loop\n"

    "read_rom_disabled:\n"
    "   beq main_loop\n"                         // zero flag is set if nmi check is not required
    "   cmp r2, %[nmiaddr]\n"
    "   bne main_loop\n"                         // If we aren't reading the nmi entry then loop back other activate the nmi rom

    "activate_nmi:\n"
    "   mov r0, #0\n"                            // always serve a NOP as first NMI instruction, this saves a read instruction
                                                 // but we need to ensure that nmi.asm also has a nop at location 0x66.
    "   str r0, [ %[base], %[sm1_tx_offset] ]\n" // Send data to the PIO like an pio_sm_put() and this sends it off to the speccy.
    "   mov r10, r0\n"                           // NMI always allows writes, so set allowable writes from 0 onwards
    "   mov r0, #1\n"
    "   str r0, [%[rom_state], #4]\n"            // nmi routine is now active
    "   str r0, [%[rom_state], #0]\n"            // rom is enabled, because it is the nmi rom
    "   mov r8, %[nmiexit]\n"                    // and set r8 to nmi exit address
    "   ldr r4, =%[rom_data_nmi]\n"
    "   b main_loop\n"

    "write_op:\n"
    // r0 is data and address, r1 is just address
    // writes are allowed from the address in r10 onwards
    "   cmp r1, r10\n"
    "   blo main_loop\n"
    "   lsr r0, r0, #16\n"
    "   strb r0, [r4, r1]\n"
    "   b main_loop\n"
    :
    : [base] "r" (PIO0_BASE), 
      [nmiaddr] "h" ((0 << 14 | 0x66) << 17),
      [nmiexit] "h" ((0 << 14 | EXITNMI) << 18),
      [fstat_offset] "I" (PIO_FSTAT_OFFSET),
      [sm0_rx_offset] "I" (PIO_RXF0_OFFSET),
      [sm0_tx_offset] "I" (PIO_TXF0_OFFSET),
      [sm1_tx_offset] "I" (PIO_TXF0_OFFSET + 4),
      [rx0empty_shift] "I" (PIO_FSTAT_RXEMPTY_LSB + 1),
      [addr_mask] "i" (0x3FFF),
      [rom_state] "r" (&rom_state),
      [rom_data]  "h" (rom_data),
      [rom_data_nmi]  "i" (rom_data + 16384)
      
    : "r0", "r2", "r1", "r3", "r4", "r8", "r10"
    );

    return;
}



void __time_critical_func(do_my_pio)() {

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

    offset = pio_add_program(pio, &putdata_program);
    sm_config = putdata_program_get_default_config(offset);
    sm_config_set_sideset_pins(&sm_config, PIN_CSROM);
    sm_config_set_out_pins(&sm_config, PIN_D0, 8);
    pio_sm_init(pio, SM_OUTDATA, offset, &sm_config);
    pio_sm_set_enabled(pio, SM_OUTDATA, true);

    serverom();
}

#define SNA_SIZE 49179
#define SNA_LOAD_SIZE (LZ4_DECOMPRESS_INPLACE_BUFFER_SIZE(SNA_SIZE))

static char sna_load_buffer[SNA_LOAD_SIZE + 1024];

#define ACTION_BEGIN_SNA_READ 0x01
#define ACTION_SNA_READ_NEXT 0x02
#define ACTION_SNA_BEGIN_WRITE 0x03
#define ACTION_SNA_NEXT_WRITE 0x04
#define ACTION_SNA_LIST 0x05
#define ACTION_SNA_SAVE 0x06
#define ACTION_ROM_LIST 0x07
#define ACTION_ROM_CHANGE 0x08



const char* current_sna_data = NULL;
uint32_t current_sna_offset =  0;
uint32_t current_sna_size = 0;

uint32_t current_sna_write_offset = 0;


void load_snapshot_file(const char* fileAndExt) {
    char filename[128];
    int filePartLen = strlen(fileAndExt);
    memcpy(filename, fileAndExt, filePartLen);
    filename[filePartLen] = '.';
    const char* ext = fileAndExt + filePartLen + 1;
    bool isCompressed = strcasecmp(ext, "snaz") == 0;
    strcpy(filename + filePartLen + 1, ext);
    LOG("File name and extension: %s\n", filename);
    int file = pico_open(filename, LFS_O_RDONLY);
    size_t file_size = pico_size(file);

    if (isCompressed) {
        pico_read(file, sna_load_buffer + SNA_LOAD_SIZE - file_size, file_size);
        int res = LZ4_decompress_safe_partial(sna_load_buffer + SNA_LOAD_SIZE - file_size, sna_load_buffer, file_size, SNA_SIZE, SNA_SIZE);
        LOG("Decompress result: %d, original size: %d\n", res, file_size);
    }
    else {
        pico_read(file, sna_load_buffer, SNA_SIZE);
    }

    pico_close(file);
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

inline bool is_snapshot_file(const char* filename, size_t filename_len) {
    return (filename_len > 5 && strcasecmp(filename + filename_len - 5, ".snaz") == 0) ||
        (filename_len > 4 && strcasecmp(filename + filename_len - 4, ".sna") == 0);
}

const char* split_filename_and_get_ext(char* name) {
    char* ext = nullptr;
    char* pch = name;
    while (*pch != '\0') {
        if (*pch == '.')
            ext = pch;
        pch++;
    }
    if (ext == nullptr)
        return pch;
    *ext = '\0';
    return ext + 1;
}

void nmi_action_sna_save() {
    uint8_t* nmi_rom_data = rom_data + 16384;
    uint16_t nameoffset =  (nmi_rom_data[3] << 8) |  nmi_rom_data[2];
    uint8_t overwrite = nmi_rom_data[4];

    LOG("Saving SNA with name: %s, overwrite=%d\n", nmi_rom_data + nameoffset, overwrite);
    char filename[128];
    strcpy(filename, reinterpret_cast<const char*>(nmi_rom_data) + nameoffset);
    strcat(filename, ".sna");

    int file = pico_open(filename, (LFS_O_CREAT | LFS_O_WRONLY | LFS_O_TRUNC) 
    | (overwrite == 0 ? LFS_O_EXCL : 0));

    if (file == LFS_ERR_EXIST) {
        LOG("File exists\n");
        nmi_rom_data[0] = 1;
        return;
    }

    bool ok = file >= 0;

    if (ok) {
        ok = pico_write(file, sna_load_buffer, SNA_SIZE) >= 0;
        pico_close(file);
    }

    nmi_rom_data[0] = ok ? 0 : 2;        
}

bool add_file_to_list(const char* name, const uint16_t destoffset, uint16_t& stringoffset) {
    uint8_t* nmi_rom_data = rom_data + 16384;
    int index = nmi_rom_data[destoffset];
    if (index >= 10) {
        nmi_rom_data[destoffset + 1] = 1;
        return false;
    }
    nmi_rom_data[destoffset] = index + 1;
    nmi_rom_data[destoffset + 2 + 2 * index] = stringoffset & 0xFF;
    nmi_rom_data[destoffset + 2 + 2 * index + 1] = (stringoffset >> 8) & 0xFF;
    int filenamelen = strlen(name) + 1;
    filenamelen += strlen(name + filenamelen) + 1;
    memcpy(nmi_rom_data + stringoffset, name, filenamelen);
    stringoffset += filenamelen;
    return true;
}



void nmi_action_list_sna() {
    uint8_t* nmi_rom_data = rom_data + 16384;
    uint16_t start =  (nmi_rom_data[3] << 8) |  nmi_rom_data[2];
    uint16_t destoffset =  (nmi_rom_data[5] << 8) |  nmi_rom_data[4];
    uint16_t stringoffset = destoffset + 22;

    LOG("List snaps starting at %d and storing at %X\n", start, destoffset);

    int dir = pico_dir_open("/");
    int16_t foundsofar = 0;
    struct lfs_info info;

    nmi_rom_data[destoffset] = 0;
    nmi_rom_data[destoffset + 1] = 0;

    while (pico_dir_read(dir, &info) > 0) {
        if (info.type == LFS_TYPE_REG) {
            const char* ext = split_filename_and_get_ext(info.name);
            if (strcasecmp(ext, "snaz") == 0 || strcasecmp(ext, "sna") == 0) {
                if (foundsofar >= start) {
                    if (!add_file_to_list(info.name, destoffset, stringoffset))
                        break;
                }
                foundsofar++;
            }
        }
    }
    pico_dir_close(dir);
}



void nmi_action_list_rom() {
    uint8_t* nmi_rom_data = rom_data + 16384;
    uint16_t start =  (nmi_rom_data[3] << 8) |  nmi_rom_data[2];
    uint16_t destoffset =  (nmi_rom_data[5] << 8) |  nmi_rom_data[4];
    uint16_t stringoffset = destoffset + 22;

    LOG("List roms starting at %d and storing at %X\n", start, destoffset);

    int dir = pico_dir_open("/");
    struct lfs_info info;

    nmi_rom_data[destoffset] = 0;
    nmi_rom_data[destoffset + 1] = 0;
    int16_t foundsofar = 1;

    if (start == 0) {
        add_file_to_list("Internal Rom\0int", destoffset, stringoffset);
    }

    while (pico_dir_read(dir, &info) > 0) {
        if (info.type == LFS_TYPE_REG) {
            const char* ext = split_filename_and_get_ext(info.name);
            if (strcasecmp(ext, "romz") == 0 || strcasecmp(ext, "rom") == 0) {
                if (foundsofar >= start) {
                    if (!add_file_to_list(info.name, destoffset, stringoffset))
                        break;
                }
                foundsofar++;
            }
        }
    }
    pico_dir_close(dir);
}

void nmi_action_change_rom() {
    uint8_t* nmi_rom_data = rom_data + 16384;
    uint32_t nameoffset = (nmi_rom_data[3] << 8) |  nmi_rom_data[2];
    bool makeWritable = nmi_rom_data[4] != 0;

    

    const char* fileAndExt = reinterpret_cast<const char*>(nmi_rom_data + nameoffset);
    char filename[128];
    int filePartLen = strlen(fileAndExt);
    memcpy(filename, fileAndExt, filePartLen);
    filename[filePartLen] = '.';
    const char* ext = fileAndExt + filePartLen + 1;
    bool isCompressed = strcasecmp(ext, "romz") == 0;
    strcpy(filename + filePartLen + 1, ext);
    LOG("Rom File name and extension: %s, is compressed = %d\n", filename, isCompressed);
    LOG("Writable: %d\n", makeWritable);

    if (strcmp(filename, "Internal Rom.int") == 0) {
         // disable PICO from supplying its own rom when the nmi routine exits
         // and thereby return to the Spectrum internal ROM
        rom_state.flags_on_nmi_exit = 0;
        rom_state.writableStartAddress = 32768;
        nmi_rom_data[0] = 0;
    }
    else {
        int file = pico_open(filename, LFS_O_RDONLY);
        if (file < 0) {
            nmi_rom_data[0] = 1;
            return;
        }

        size_t file_size = pico_size(file);

        if (isCompressed) {
            pico_read(file, sna_load_buffer, file_size);
            int res = LZ4_decompress_safe_partial(sna_load_buffer, 
                reinterpret_cast<char*>(rom_data), file_size, 16384, 16384);
            LOG("Decompress result: %d, original size: %d\n", res, file_size);
        }
        else {
            pico_read(file, sna_load_buffer, SNA_SIZE);
        }

        pico_close(file);
        rom_state.flags_on_nmi_exit = 1;
        rom_state.writableStartAddress = makeWritable ? 16 : 32768;
        nmi_rom_data[0] = 0;
    }
}


void process_nmi_request() {
    uint8_t* nmi_rom_data = rom_data + 16384;
    uint8_t action = nmi_rom_data[1];
    LOG("Action to process: %d\n", action);
    if (action == ACTION_BEGIN_SNA_READ) {
        uint32_t nameoffset = (nmi_rom_data[3] << 8) |  nmi_rom_data[2];
        uint32_t headeroffset =  (nmi_rom_data[5] << 8) |  nmi_rom_data[4];

        LOG("Loading %s\n", nmi_rom_data + nameoffset);
        load_snapshot_file(reinterpret_cast<const char*>(nmi_rom_data) + nameoffset);

        LOG("Start SNA, head destination: %X\n", headeroffset);
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
        LOG("Continue SNA, destination: %X, count: %X\n", offset, tocopy);
        memcpy(nmi_rom_data + offset, current_sna_data + current_sna_offset, tocopy);
        current_sna_offset += tocopy;
        nmi_rom_data[4] = tocopy & 0xFF;
        nmi_rom_data[5] = (tocopy >> 8) & 0xFF;
    }  else if (action == ACTION_SNA_BEGIN_WRITE) {
        uint16_t headeroffset =  (nmi_rom_data[3] << 8) |  nmi_rom_data[2];
        LOG("Start SNA save, head destination: %X\n", headeroffset);
        memcpy(sna_load_buffer, nmi_rom_data + headeroffset, 27);
        current_sna_write_offset = 27;
    }  else if (action == ACTION_SNA_NEXT_WRITE) {
        uint16_t offset =  (nmi_rom_data[3] << 8) |  nmi_rom_data[2];
        uint16_t count =  (nmi_rom_data[5] << 8) |  nmi_rom_data[4];
        uint32_t tocopy = MIN(count, SNA_SIZE - current_sna_write_offset);
        LOG("Continue SNA save, offset: %X, length = %X; tocopy = %X\n", offset, count, tocopy);
        memcpy(sna_load_buffer + current_sna_write_offset, nmi_rom_data + offset, tocopy);
       current_sna_write_offset += tocopy;
    } else if (action == ACTION_SNA_LIST) {
        nmi_action_list_sna();
    } else if (action == ACTION_SNA_SAVE) {
        nmi_action_sna_save();
        return;
    } else if (action == ACTION_ROM_LIST) {
        nmi_action_list_rom();
    } else if (action == ACTION_ROM_CHANGE) {
        nmi_action_change_rom();
        return;
    }

    nmi_rom_data[0] = 0;
}


// Attempt to obtain snapshot from the running spectrum
const uint8_t* getSnapshotData(size_t& snapshotLength) {
    // Without the volatile then the nmi_rom_data[0] is only
    // accessed once and we fail to process requests from the nmi
    // rom and the spectrum hangs.
    volatile uint8_t* nmi_rom_data = rom_data + 16384;

    bool loggingWasEnabled = enable_logging;
    enable_logging = false;
    const uint8_t* data = nullptr;

    if (send_nmi_request(STARTUP_ACTION_TRANSFER_SNAP, 0, 0)) {
        while (((rom_state.flags & 0x2) != 0) || rom_state.nmi_active) {
            if (nmi_rom_data[0] == 255) {
                process_nmi_request();
            }            
        }
        snapshotLength = SNA_SIZE;
        data = reinterpret_cast<const uint8_t*>(sna_load_buffer);
    }

    enable_logging = loggingWasEnabled;
    return data;
}

void init_file_system() {
    if (pico_mount(false) != LFS_ERR_OK) {
        LOG("Mount failed, try formatting\n");
        if (pico_mount(true) != LFS_ERR_OK) {
            LOG("Mount failed, after formatting\n");
        }
    }
}


bool send_nmi_request(uint8_t command, uint16_t param1, uint16_t param2) {
    uint8_t* nmi_rom_data = rom_data + 16384;
    memcpy(nmi_rom_data, NMI_ROM, NMI_ROM_SIZE);

    if (((rom_state.flags & 0x2) != 0) || rom_state.nmi_active) {
        return false;
    }
    else {
        nmi_rom_data[0] = 0;
        nmi_rom_data[STARTUP_COMMAND_OFFSET] = command;
        rom_state.flags |= 2;
        gpio_put(PIN_NMI, false);
        sleep_ms(3);
        gpio_put(PIN_NMI, true);
        return true;
    }
}

//#define PIO_DEBUGGING


#ifdef PIO_DEBUGGING

void setup_pios() {
    PIO pio = pio0;
    pio_sm_claim(pio, MY_SM);
    pio_sm_claim(pio, SM_OUTDATA);
    

    uint offset = pio_add_program(pio, &pushpull_program);
    pio_sm_config sm_config = pushpull_program_get_default_config(offset);
    pio_sm_init(pio, MY_SM, offset, &sm_config);
    pio_sm_set_enabled(pio, MY_SM, true);

    pio_sm_init(pio, SM_OUTDATA, offset, &sm_config);
    pio_sm_set_enabled(pio, SM_OUTDATA, true);

}

void run_pio_test() {


    serverom();
}

int main() {
    stdio_init_all();
    setup_pios();

    rom_state.flags = 2;

    multicore_launch_core1(serverom);


    sleep_ms(1000);


    PIO pio = pio0;

    uint32_t dataval = (0x2 << 14) | 0x66;

    pio_sm_put_blocking(pio, MY_SM, dataval); // entry to nmi
    pio_sm_put_blocking(pio, MY_SM, (0x2 << 14) | EXITNMI); // exit from nmi


    while (true) {
        sleep_ms(1000);
    }

}


#else



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
                if (!send_nmi_request(0, 0, 0)) {
                    LOG("Couldn't send NMI because an NMI is already active.\n");
                }
            }
        }
    }
    
}

#endif