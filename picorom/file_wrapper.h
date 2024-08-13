#ifndef INCLUDED_FILE_WRAPPER_H
#define INCLUDED_FILE_WRAPPER_H

#include <string.h>
#include "ff.h"
#include "pico_hal.h"


inline bool strStartsWith(const char* str, const char* prefix) {
    size_t prefixlen = strlen(prefix);
    return strncmp(str, prefix, prefixlen) == 0;
}


enum class Drive {
    INTERNAL_FLASH, EXTERNAL_SD_CARD
}; 


class FileHandle {

    Drive drive;
    union {
        int intFile;
        FIL sdFile;
    };

public:

    bool open(const char* filename, bool readOnly);
    bool open(Drive drive, const char* filename, bool readOnly);

    int read(void* buffer, size_t bufferSize) {
        if (drive == Drive::EXTERNAL_SD_CARD) {
            UINT read;
            FRESULT res = f_read(&sdFile, buffer, bufferSize, &read);
            return res == FR_OK ? read : -1;
        }
        else {
            return pico_read(intFile, buffer, bufferSize);
        }
    }


    int write(const void* buffer, size_t bufferSize) {
        if (drive == Drive::EXTERNAL_SD_CARD) {
            UINT written;
            FRESULT res = f_write(&sdFile, buffer, bufferSize, &written);
            return res == FR_OK ? written : -res;
        }
        else {
            return pico_write(intFile, buffer, bufferSize);
        }
    }

    int size() {
        if (drive == Drive::EXTERNAL_SD_CARD) {
            return f_size(&sdFile);
        }
        else {
            return pico_size(intFile);
        }
    }

    bool close() {
        if (drive == Drive::EXTERNAL_SD_CARD) {
            return f_close(&sdFile) == FR_OK;
        }
        else {
            return pico_close(intFile) >= 0;
        }
    }
};


class DirHandle {

    Drive drive;
    union {
        struct {
            int intDir;
            lfs_info intInfo;
        };
        struct {
            DIR sdDir;
            FILINFO sdFileInfo;
        };
    };

public:

    bool open(const char* path);

    bool next();

    bool atEnd() {
        return fileName()[0] == '\0';
    }

    const char* fileName() {
        return drive == Drive::EXTERNAL_SD_CARD ? sdFileInfo.fname : intInfo.name;
    }

    uint32_t fileSize() {
        return drive == Drive::EXTERNAL_SD_CARD ? (uint32_t) sdFileInfo.fsize : intInfo.size;
    }

    bool isDirectory() {
        return drive == Drive::EXTERNAL_SD_CARD ?
            (sdFileInfo.fattrib & AM_DIR) != 0 :
            (intInfo.type == LFS_TYPE_DIR);
    }

    bool close() {
        if (drive == Drive::EXTERNAL_SD_CARD) {
            return f_closedir(&sdDir) == FR_OK;
        }
        else {
            return pico_dir_close(intDir) >= 0;
        }
    }


    bool getStats(uint32_t& blockSize, uint32_t& blockCount, uint32_t& blocksUsed);

};



#endif