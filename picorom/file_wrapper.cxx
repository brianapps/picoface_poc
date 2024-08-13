#include "file_wrapper.h"

bool FileHandle::open(Drive drive, const char* filename, bool readOnly) {
    this->drive = drive;
    if (drive == Drive::EXTERNAL_SD_CARD) {;
        memset(&sdFile, 0, sizeof(sdFile));
        FRESULT res = f_open(&sdFile, filename, 
            readOnly ? FA_READ : (FA_WRITE | FA_CREATE_ALWAYS));
        return res == FR_OK;
    }
    else {
        intFile = pico_open(
            filename, readOnly ? (LFS_O_RDONLY) :
            (LFS_O_CREAT | LFS_O_TRUNC | LFS_O_WRONLY)
        );
        return intFile >= 0;
    }
}

bool FileHandle::open(const char* filename, bool readOnly) {
    if (strStartsWith(filename, "/sd")) {
        return open(Drive::EXTERNAL_SD_CARD, filename + 3, readOnly);
    }
    else {
        return open(Drive::INTERNAL_FLASH, filename, readOnly);
    }
}


bool DirHandle::open(const char* path) {
    if (strStartsWith(path, "/sd")) {
        drive = Drive::EXTERNAL_SD_CARD;
        memset(&sdDir, 0, sizeof(sdDir));
        memset(&sdFileInfo, 0, sizeof(sdFileInfo));
        return f_findfirst(&sdDir, &sdFileInfo, path + 3, "*") == FR_OK;
    }
    else {
        drive = Drive::INTERNAL_FLASH;
        intDir = pico_dir_open(path);
        if (intDir < 0) {
            return false;
        }
        int ret = pico_dir_read(intDir, &intInfo);
        if (ret == 0) {
            intInfo.name[0] = '\0';
        }
        return ret >= 0;
    }
}


bool DirHandle::next() {
    if (drive == Drive::EXTERNAL_SD_CARD) {
        return f_findnext(&sdDir, &sdFileInfo) == FR_OK;
    }
    else {
        int ret = pico_dir_read(intDir, &intInfo);
        if (ret == 0) {
            intInfo.name[0] = '\0';
        }
        return ret >= 0;
    }
}


bool DirHandle::getStats(uint32_t& blockSize, uint32_t& blockCount, uint32_t& blocksUsed) {
    if (drive == Drive::EXTERNAL_SD_CARD) {
        return false;
    }
    else {
        pico_fsstat_t stats;
        if (pico_fsstat(&stats) < 0) {
            return false;
        }
        blockSize = stats.block_size;
        blocksUsed = stats.blocks_used;
        blockCount = stats.block_count;
        return true;
    }
}


