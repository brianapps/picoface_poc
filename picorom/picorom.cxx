#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/pio.h"
#ifdef WIFI_ENABLE
#include "pico/cyw43_arch.h"
#endif
#include "hardware/structs/bus_ctrl.h"
#include "usb_command_handler.h"


#include "http_server.h"
#include "littlefs-lib/pico_hal.h"


#include "blink.pio.h"

#define LZ4_STATIC_LINKING_ONLY
#include "lz4.h"
#include "nmi.h"
#include "picorom.h"

extern const uint8_t FGH_ROM[];
extern const uint32_t FGH_ROM_SIZE;

/*
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

*/


// set high to disable rom.
// When low /CSROM is floating but when high this pulls /CSROM to GND and
// therefore gives the pico control of the data lines when the Z80 is 
// accessing memory in the lower 16KB.
const uint8_t PIN_CSROM = 26; 
const uint8_t PIN_PICOREQ = 22;
const uint8_t PIN_XE = 20;

const uint8_t PIN_USER_SWITCH = 27;
const uint8_t PIN_NMI = 21;

const uint MY_SM = 0;
const uint SM_OUTDATA = 1;

const uint8_t PIN_D0 = 12;
const uint8_t PIN_A0xA8 = 4;



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


extern "C" void piohandler();


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
    sm_config_set_in_pins(&sm_config, PIN_A0xA8);
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

    piohandler();
}



#define SNA_LOAD_SIZE (LZ4_DECOMPRESS_INPLACE_BUFFER_SIZE(SNA_FILE_SIZE))

static char snap_load_buffer[SNA_LOAD_SIZE + 1024];


const char* current_snap_data = NULL;
uint32_t current_snap_offset =  0;
uint32_t current_snap_size = 0;



uint32_t current_sna_write_offset = 0;


void load_snapshot_file(const char* fileAndExt) {
    char filename[128];
    int filePartLen = strlen(fileAndExt);
    memcpy(filename, fileAndExt, filePartLen);
    filename[filePartLen] = '.';
    const char* ext = fileAndExt + filePartLen + 1;

    bool isCompressed = false;

    if (strcasecmp(ext, "snaz") == 0) {
        isCompressed = true;
        current_snap_size = SNA_FILE_SIZE;
    }
    else if (strcasecmp(ext, "z80z") == 0) {
        isCompressed = true;
        current_snap_size = Z80_FILE_SIZE;
    }
    else if (strcasecmp(ext, "z80") == 0) {
        current_snap_size = Z80_FILE_SIZE;
    }
    else if (strcasecmp(ext, "sna") == 0) {
        current_snap_size = SNA_FILE_SIZE;
    }

    strcpy(filename + filePartLen + 1, ext);
    LOG("File name and extension: %s\n", filename);
    int file = pico_open(filename, LFS_O_RDONLY);
    size_t file_size = pico_size(file);

    if (isCompressed) {
        pico_read(file, snap_load_buffer + SNA_LOAD_SIZE - file_size, file_size);
        int res = LZ4_decompress_safe_partial(snap_load_buffer + SNA_LOAD_SIZE - file_size, snap_load_buffer, file_size, current_snap_size, current_snap_size);
        LOG("Decompress result: %d, original size: %d\n", res, file_size);
    }
    else {
        pico_read(file, snap_load_buffer, current_snap_size);
    }

    pico_close(file);
    current_snap_data = snap_load_buffer;
}

bool stringendswith(const char* str, const char* suffix) {
    int suffixlen = strlen(suffix);
    int len = strlen(str);
    if (suffixlen > len) {
        return false;
    }
    return strcasecmp(str + len - suffixlen, suffix) == 0;
}


inline bool is_snapshot_ext(const char* extension) {
    return (strcasecmp(extension, "sna") == 0) || (strcasecmp(extension, "snaz") == 0) ||
        (strcasecmp(extension, "z80") == 0) || (strcasecmp(extension, "z80z") == 0);

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
        ok = pico_write(file, snap_load_buffer, SNA_FILE_SIZE) >= 0;
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
            if (is_snapshot_ext(ext)) {
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
            pico_read(file, snap_load_buffer, file_size);
            int res = LZ4_decompress_safe_partial(snap_load_buffer, 
                reinterpret_cast<char*>(rom_data), file_size, 16384, 16384);
            LOG("Decompress result: %d, original size: %d\n", res, file_size);
        }
        else {
            pico_read(file, snap_load_buffer, SNA_FILE_SIZE);
        }

        pico_close(file);
        rom_state.flags_on_nmi_exit = 1;
        rom_state.writableStartAddress = makeWritable ? 16 : 32768;
        nmi_rom_data[0] = 0;
    }
}

void nmi_action_read_data_from_pico() {
    // WORD param1 pointer to destination location (in spectrum rom)
    // WORD param2 offset address to read from must be >= 16384
    // WORD param3 length of source data

    uint8_t* nmi_rom_data = rom_data + 16384;
    uint32_t destoffset = (nmi_rom_data[3] << 8) |  nmi_rom_data[2];
    uint32_t srcoffset = (nmi_rom_data[5] << 8) |  nmi_rom_data[4];
    uint32_t length = (nmi_rom_data[7] << 8) |  nmi_rom_data[6];

    if (srcoffset >= 16384) {
        memcpy(nmi_rom_data + destoffset, 
            snap_load_buffer + SNA_HEADER_SIZE + srcoffset - 16384, length);
    }
}


void nmi_action_write_data_to_pico() {
    // WORD param1 pointer to source location of data (in spectrum rom)
    // WORD param2 offset address to write data must be >= 16384
    // WORD param3 length of source data

    uint8_t* nmi_rom_data = rom_data + 16384;
    uint32_t srcoffset = (nmi_rom_data[3] << 8) |  nmi_rom_data[2];
    uint32_t destoffset = (nmi_rom_data[5] << 8) |  nmi_rom_data[4];
    uint32_t length = (nmi_rom_data[7] << 8) |  nmi_rom_data[6];

    if (destoffset >= 16384) {
        memcpy(snap_load_buffer + SNA_HEADER_SIZE + destoffset - 16384,
            nmi_rom_data + srcoffset , length);
    }
}


void process_nmi_request() {
    uint8_t* nmi_rom_data = rom_data + 16384;
    uint8_t action = nmi_rom_data[1];
    LOG("Action to process: %d\n", action);
    if (action == ACTION_BEGIN_SNAP_READ) {
        uint32_t nameoffset = (nmi_rom_data[3] << 8) |  nmi_rom_data[2];
        uint32_t headeroffset =  (nmi_rom_data[5] << 8) |  nmi_rom_data[4];

        if (nameoffset != 0) {
            LOG("Loading %s\n", nmi_rom_data + nameoffset);
            load_snapshot_file(reinterpret_cast<const char*>(nmi_rom_data) + nameoffset);
            LOG("Start snapshot, size = %d, head destination: %X\n", current_snap_size, headeroffset);
        } else {
            current_snap_data = snap_load_buffer;
        }

        if (current_snap_size == Z80_FILE_SIZE) {
            LOG("Sending Z80 file header\n");
            nmi_rom_data[2] = 1;
            memcpy(nmi_rom_data + headeroffset, current_snap_data, Z80_HEADER_SIZE);
            current_snap_offset = Z80_HEADER_SIZE;
        }
        else {
            LOG("Sending SNA file header\n");
            nmi_rom_data[2] = 0;
            memcpy(nmi_rom_data + headeroffset, current_snap_data, SNA_HEADER_SIZE);
            current_snap_offset = SNA_HEADER_SIZE;
        }
    }
    else if (action == ACTION_SNAP_READ_NEXT) {
        uint16_t offset =  (nmi_rom_data[3] << 8) |  nmi_rom_data[2];
        uint16_t count =  (nmi_rom_data[5] << 8) |  nmi_rom_data[4];
        uint32_t tocopy = MIN(count, current_snap_size - current_snap_offset);
        LOG("Continue SNA, destination: %X, count: %X\n", offset, tocopy);
        memcpy(nmi_rom_data + offset, current_snap_data + current_snap_offset, tocopy);
        current_snap_offset += tocopy;
        nmi_rom_data[4] = tocopy & 0xFF;
        nmi_rom_data[5] = (tocopy >> 8) & 0xFF;
    }  else if (action == ACTION_SNA_BEGIN_WRITE) {
        uint16_t headeroffset =  (nmi_rom_data[3] << 8) |  nmi_rom_data[2];
        LOG("Start SNA save, head destination: %X\n", headeroffset);
        memcpy(snap_load_buffer, nmi_rom_data + headeroffset, SNA_HEADER_SIZE);
        current_sna_write_offset = SNA_HEADER_SIZE;
    }  else if (action == ACTION_SNA_NEXT_WRITE) {
        uint16_t offset =  (nmi_rom_data[3] << 8) |  nmi_rom_data[2];
        uint16_t count =  (nmi_rom_data[5] << 8) |  nmi_rom_data[4];
        uint32_t tocopy = MIN(count, SNA_FILE_SIZE - current_sna_write_offset);
        LOG("Continue SNA save, offset: %X, length = %X; tocopy = %X\n", offset, count, tocopy);
        memcpy(snap_load_buffer + current_sna_write_offset, nmi_rom_data + offset, tocopy);
       current_sna_write_offset += tocopy;
    } else if (action == ACTION_SNAP_LIST) {
        nmi_action_list_sna();
    } else if (action == ACTION_SNA_SAVE) {
        nmi_action_sna_save();
        return;
    } else if (action == ACTION_ROM_LIST) {
        nmi_action_list_rom();
    } else if (action == ACTION_ROM_CHANGE) {
        nmi_action_change_rom();
        return;
    } else if (action == ACTION_READ_DATA_FROM_PICO) {
        nmi_action_read_data_from_pico();
    } else if (action == ACTION_WRITE_DATA_TO_PICO) {
        nmi_action_write_data_to_pico();
    }


    nmi_rom_data[0] = 0;
}


//---------------------------------------------
// CLI commands

inline bool sendNmiAndWaitForCompletion(uint8_t command, uint16_t param1, uint16_t param2) {
    // Without the volatile then the nmi_rom_data[0] is only
    // accessed once and we fail to process requests from the nmi
    // rom and the spectrum hangs.
    volatile uint8_t* nmi_rom_data = rom_data + 16384;
    if (send_nmi_request(command, param1, param2)) {
        while (((rom_state.flags & 0x2) != 0) || rom_state.nmi_active) {
            if (nmi_rom_data[0] == 255) {
                process_nmi_request();
            }            
        }
        return true;
    }
    return false;
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
        snapshotLength = SNA_FILE_SIZE;
        data = reinterpret_cast<const uint8_t*>(snap_load_buffer);
    }

    enable_logging = loggingWasEnabled;
    return data;
}


uint8_t* beginSendSnapDataToMachine() {
    return reinterpret_cast<uint8_t*>(snap_load_buffer);
}

uint8_t* getRamLoadBuffer() {
    return reinterpret_cast<uint8_t*>(snap_load_buffer + SNA_HEADER_SIZE);
}


bool endSendSnapDataToMachine(uint32_t snapshotSize) {
    bool loggingWasEnabled = enable_logging;
    enable_logging = false;
    current_snap_size = snapshotSize;
    bool ok = sendNmiAndWaitForCompletion(STARTUP_ACTION_LOAD_SNAP, 0, 0);
    enable_logging = loggingWasEnabled;
    return ok;
}


bool sendLoadBufferToMachine(uint32_t startAddress, uint32_t length) {
    if (startAddress >= 16384 && startAddress + length < 65536) {
        // PARAM1 is destination location
        // PARAM2 is total length
        bool loggingWasEnabled = enable_logging;
        enable_logging = false;
        bool ok = sendNmiAndWaitForCompletion(STARTUP_ACTION_LOAD_MEMORY, startAddress, length);
        enable_logging = loggingWasEnabled;
        return ok;
    }
    return false;
}

bool readMachineMemToLoadBuffer(uint32_t startAddress, uint32_t length) {
    if (startAddress >= 16384 && startAddress + length < 65536) {
        // PARAM1 is source location
        // PARAM2 is total length
        bool loggingWasEnabled = enable_logging;
        enable_logging = false;
        bool ok = sendNmiAndWaitForCompletion(STARTUP_ACTION_SAVE_MEMORY, startAddress, length);
        enable_logging = loggingWasEnabled;
        return ok;
    }
    return false;
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

    if (((rom_state.flags & 0x2) != 0) || rom_state.nmi_active) {
        return false;
    }
    else {
        uint8_t* nmi_rom_data = rom_data + 16384;
        memcpy(nmi_rom_data, NMI_ROM, NMI_ROM_SIZE);
        nmi_rom_data[0] = 0;
        nmi_rom_data[STARTUP_COMMAND_OFFSET] = command;
        nmi_rom_data[STARTUP_PARAM1_OFFSET] = param1 & 0xFF;
        nmi_rom_data[STARTUP_PARAM1_OFFSET + 1] = (param1 >> 8) & 0xFF;
        nmi_rom_data[STARTUP_PARAM2_OFFSET] = param2 & 0xFF;
        nmi_rom_data[STARTUP_PARAM2_OFFSET + 1] = (param2 >> 8) & 0xFF;
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



int main() {
    stdio_init_all();

    bus_ctrl_hw->priority = BUSCTRL_BUS_PRIORITY_PROC1_BITS;

    //set_sys_clock_khz(125 * 1000, true);

    memcpy(rom_data + 16384, NMI_ROM, NMI_ROM_SIZE);
    rom_state.flags = 0;

    multicore_launch_core1(do_my_pio);

    gpio_init(PIN_USER_SWITCH);
    gpio_set_dir(PIN_USER_SWITCH, false);
    gpio_pull_up(PIN_USER_SWITCH);

    gpio_init(PIN_NMI);
    gpio_put(PIN_NMI, true);
    gpio_set_dir(PIN_NMI, true);    


    sleep_ms(200);
    init_file_system();

    #ifdef WIFI_ENABLE
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
        #ifdef WIFI_ENABLE
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

        #ifdef WIFI_ENABLE
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