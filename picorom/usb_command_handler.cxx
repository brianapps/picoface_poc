
#include <stdarg.h>
#include "pico/stdlib.h"
#include "pico_hal.h"
#include "pico/stdio_usb.h"
#include "pico/stdio/driver.h"
#include "picorom.h"

/*
Supported commands

ls [-json]
rm <filename>
mkdir <dirname>
mv <filename> <filename1>
upload <filename>
download <filename>

memread <start> <length>
memwrite <start> <length>
snapupload
snapdownload


*/


constexpr uint8_t SOH = 1;
constexpr uint8_t EOT = 4;
constexpr uint8_t ACK = 6;
constexpr uint8_t NACK = 'X';


static int usbGetChars(char* buffer, int len, absolute_time_t expiry) {
    int readsofar = 0;

    while (absolute_time_diff_us(get_absolute_time(), expiry) > 0)  {
        int read = (stdio_usb.in_chars)(buffer + readsofar, len - readsofar);
        if (read > 0) {
            readsofar += read;
            if (readsofar == len)
                return len;
        }
    }
    return PICO_ERROR_TIMEOUT;
}


static bool readChars(char* buffer, size_t bufferlen, absolute_time_t expiryTime) {
    size_t pos = 0;
    while (pos < bufferlen) {

        int c = getchar_timeout_us(30 * 1000 * 1000);
        if (c == PICO_ERROR_TIMEOUT) {
            return false;
        }
        buffer[pos] = c;
        pos++;
    }
    return true;
}




class CommStreamReader {
private:
    bool isOK;
    bool atEOF;
    uint32_t total_size; // or -1 if unspecified
    uint16_t packetRemaining;

    inline void putchar(char c) {
        stdio_usb.out_chars(&c, 1);
    }

  
    inline int getchars(char* buffer, int len, absolute_time_t expiry) {
        return usbGetChars(buffer, len, expiry);
    }

    void nextPacket(absolute_time_t expiry);

public:

    bool begin();
    // 
    int read(char* buffer, int len, absolute_time_t expiry);
};




class CommStreamWriter {
private:
    bool isOK;
    uint32_t total_size;
    uint32_t total_sent;
    

    inline void putchar(char c) {
        stdio_usb.out_chars(&c, 1);
    }

    inline int getchars(char* buffer, int len, absolute_time_t expiry) {
        return usbGetChars(buffer, len, expiry);
    }

    void nextPacket(absolute_time_t expiry);

public:

    bool begin(uint32_t totalSize = 0xFFFFFFFF);
    bool write(const char* buffer, int len, absolute_time_t expiry);
    bool end();
};


template<size_t bufferSize> class CommStreamBufferWriter {
private:
    char buffer[bufferSize];
    int bufferUsed;
    CommStreamWriter writer;


public:
    bool begin(uint32_t totalSize = 0xFFFFFFFF) {
        bufferUsed = 0;
        return writer.begin();
    }

    bool write(const char* data, int len, absolute_time_t expiry);
    bool printf(const char* format, ...);
    bool end();
   
};

template<size_t bufferSize> bool CommStreamBufferWriter<bufferSize>::printf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    while (true) {
        size_t available = bufferSize - bufferUsed;

        if (available > 0) {
            int written = vsnprintf(buffer + bufferUsed, available, format, args);

            if (written < 0) {
                return false;
            }

            if (written <= available) {
                bufferUsed += written;
                return true;
            }
        }

        if (bufferUsed == 0)
            return false;

        if (!writer.write(buffer, bufferUsed, make_timeout_time_ms(2000)))
            return false;
        bufferUsed = 0;
    }
}



template<size_t bufferSize> bool CommStreamBufferWriter<bufferSize>::write(const char* data, int len, absolute_time_t expiry) {
    int written = 0;

    while (written < len) {
        int toCopy = MIN(bufferSize - bufferUsed, len - written);
        memcpy(buffer + bufferUsed, data + written, toCopy);
        bufferUsed += toCopy;
        written += toCopy;

        if (bufferUsed == bufferSize) {
            if (!writer.write(buffer, bufferUsed, make_timeout_time_ms(2000)))
                return false;
            bufferUsed = 0;
        }
    }
    return true;
}


template<size_t bufferSize> bool CommStreamBufferWriter<bufferSize>::end() {
    if (bufferUsed != 0) {
        if (!writer.write(buffer, bufferUsed, make_timeout_time_ms(2000)))
            return false;
    }
    return writer.end();
}







bool CommStreamWriter::begin(uint32_t totalSize) {
    this->total_size = totalSize;
    this->total_sent = 0;
    char header[6];
    header[0] = SOH;
    header[1] = 0;
    //otal_size = (header[2] << 24) | (header[3] << 16) | (header[4] << 8) | (header[5]);
    header[2] = 0xFF & (totalSize >> 24);
    header[3] = 0xFF & (totalSize >> 16);
    header[4] = 0xFF & (totalSize >> 8);
    header[5] = 0xFF & (totalSize);
    stdio_usb.out_chars(header, 6);
    char c;
    isOK = getchars(&c, 1, make_timeout_time_ms(2000)) == 1 && c == ACK;
    return isOK;
}

bool CommStreamWriter::write(const char* buffer, int len, absolute_time_t expiry) {
    int sentSoFar = 0;
    while (isOK && sentSoFar < len) {
        int lenToSend = MIN(65355, len - sentSoFar);
        char header[4];
        header[0] = SOH;
        header[1] = 1;
        header[2] = 0xFF & (lenToSend >> 8);
        header[3] = 0xFF & lenToSend;
        stdio_usb.out_chars(header, 4);
        stdio_usb.out_chars(buffer + sentSoFar, lenToSend);
        sentSoFar += lenToSend;
        char c;
        isOK = getchars(&c, 1, make_timeout_time_ms(2000)) == 1 && c == ACK;
    }
    return isOK;
}

bool CommStreamWriter::end() {
    if (isOK) {
        char c = EOT;
        putchar(c);
        isOK = getchars(&c, 1, make_timeout_time_ms(2000)) == 1 && c == ACK;
    }
    return isOK;
}




bool CommStreamReader::begin() {
    atEOF = false;

    putchar(ACK);

    absolute_time_t packetTimeout = make_timeout_time_ms(80000);
    char header[6];

    packetRemaining = 0;

    // Return the header which should be SOH 0 LEN*4
    if (getchars(header, 6, packetTimeout) != 6) {
        isOK = false;
    }
    else {
        isOK = header[0] == SOH && header[1] == 0;
        if (isOK)
            total_size = (header[2] << 24) | (header[3] << 16) | (header[4] << 8) | (header[5]);
    }

    if (!isOK) 
        putchar(NACK);
    return isOK;
}


void CommStreamReader::nextPacket(absolute_time_t expiry) {
    char header[4];
    // Acknowledge have processed the prevous packet and ready for the next
    putchar(ACK);
    isOK = getchars(header, 1, expiry) == 1;

    if (isOK) {
        if (header[0] == EOT) {
            atEOF = true;
            putchar(ACK);
        }
        if (header[0] == SOH) {
            isOK = getchars(header, 3, expiry) == 3 &&
                header[0] == 1;
            if (isOK) {
                packetRemaining = (header[1] << 8) | header[2];
            }
        }
    }
}



int CommStreamReader::read(char* buffer, int len, absolute_time_t expiry) {
    int readsofar = 0;

    while (!atEOF && readsofar < len) {
        if (!isOK) {
            return PICO_ERROR_IO;
        }
        if (packetRemaining == 0) {
            nextPacket(expiry);
        }
        else {
            int toRead = MIN(len - readsofar, packetRemaining);
            int read = getchars(buffer + readsofar, toRead, expiry);
            if (read > 0) {
                readsofar += read;
                packetRemaining -= read;
            }
            else {
                isOK = false;
                putchar(NACK);
            }

        }
    }

    return readsofar;
}

int escapeCommandLine(char* command) {
    char* dest = command;
    int args = 0;

    while (true) {

        while (*command == ' ') {
            command++;
        }

        bool added = false;

        while (*command != '\0' && *command != ' ') {
            if (*command == '\\' && command[1] != '\0') {
                command++;
            }
            *dest++ = *command++;
            added = true;
        }

        if (added)
            args++;

        if (*command == '\0') {
            *dest++ = '\0';
            *dest++ = '\0';
            return args;
        }

        *dest++ = '\0';
        command++;
    }




}


void handleupload(const char* name) {
    CommStreamReader reader;
    int file = pico_open(name, LFS_O_CREAT | LFS_O_TRUNC | LFS_O_WRONLY);
    if (reader.begin()) {
        char buffer[4096];
        while (true) {
            int read = reader.read(buffer, sizeof(buffer), make_timeout_time_ms(80000));
            if (read == 0)
                break;
            else if (read < 0) {
                printf("Error\n");
                break;
            }

            pico_write(file, buffer, read);
        }
    }

    pico_close(file);
}


void handleListFiles(const char* name) {
    CommStreamBufferWriter<1024> writer;

    writer.begin();
    writer.printf("Listing files for %s\n", name);

    int dir = pico_dir_open(name);

    if (dir < 0) {
        writer.printf("Failed to open directory %d\n", dir);
    }
    else {

        lfs_info info;

        while (pico_dir_read(dir, &info) > 0) {
            writer.printf("%s, %d, %d\n", info.name, info.size, info.type);
        }

        pico_dir_close(dir);
    }

    pico_fsstat_t stats;

    pico_fsstat(&stats);

    writer.printf("FSInfo count=%d, size=%d, used=%d\n", stats.block_count, stats.block_size, stats.blocks_used);



    writer.end();
}


void handleDownload(const char* filename) {
    int file = pico_open(filename, LFS_O_RDONLY);

    if (file < 0) {
        printf("XFailed to open file to download\n");
        return;
    }

    char buffer[4096];
    CommStreamWriter writer;
    writer.begin(pico_size(file));

    while (true) {
        int read = pico_read(file, buffer, sizeof(buffer));
        if (read <= 0)
            break;
        writer.write(buffer, read, make_timeout_time_ms(2000));
    }

    writer.end();
    pico_close(file);
}


void handleDelete(const char* filename) {
    int res = pico_remove(filename);
    if (res < 0) {
        printf("XFailed to delete file: %s (%d)", filename, res);
    }
    else {
        putchar(ACK);
    }
}


void handleRename(const char* from, const char* to) {
    int res = pico_rename(from, to);
    if (res < 0) {
        printf("XFailed to rename file: %s to %s (%d)", from, to, res);
    }
    else {
        putchar(ACK);
    }
}


void handleMkDir(const char* filename) {
    int res = pico_mkdir(filename);
    if (res < 0) {
        printf("XFailed to create directory: %s (%d)", filename, res);
    }
    else {
        putchar(ACK);
    }
}


void handleSnapDownload() {

    size_t snapShotlength;

    const uint8_t* data = getSnapshotData(snapShotlength);

    if (data == nullptr) {
        printf("XFailed to get snapshot data");
    }
    else {
        CommStreamWriter writer;
        writer.begin(snapShotlength);
        writer.write(reinterpret_cast<const char*>(data), snapShotlength, make_timeout_time_ms(8000));
        writer.end();
    }



}



static char commandBuffer[512];
static size_t commandLength = 0;




void pollUsbCommandHandler() {
    char read;
    if (stdio_usb.in_chars(&read, 1) == 1) {

        if (read == '\r') {
            commandBuffer[commandLength] = 0;
            int args = escapeCommandLine(commandBuffer);
            // process command
            if (strcmp(commandBuffer, "upload") == 0) {
                const char* name = commandBuffer + 7;
                handleupload(name);
            }
            else if (strcmp(commandBuffer, "ls") == 0) {
                const char* name = commandBuffer + strlen(commandBuffer) + 1;
                handleListFiles(name);
            }
            else if (strcmp(commandBuffer, "mkdir") == 0) {
                const char* name = commandBuffer + strlen(commandBuffer) + 1;
                handleMkDir(name);
            }
            else if (strcmp(commandBuffer, "rm") == 0) {
                const char* name = commandBuffer + strlen(commandBuffer) + 1;
                handleDelete(name);
            }
            else if (strcmp(commandBuffer, "download") == 0) {
                const char* name = commandBuffer + strlen(commandBuffer) + 1;
                handleDownload(name);
            }
            else if (strcmp(commandBuffer, "mv") == 0) {
                if (args != 3) {
                    printf("XMove Bad args\n");
                }
                else {
                    const char* n1 = commandBuffer + strlen(commandBuffer) + 1;
                    const char* n2 = n1 + strlen(n1) + 1;
                    handleRename(n1, n2);
                }
            }
            else if (strcmp(commandBuffer, "snapdownload") == 0) {
                handleSnapDownload();
            }
            else if (commandBuffer[0] != '\0') {
                printf("XUnknown command: %s\n", commandBuffer);
            }


            commandLength = 0;
        }
        else {
            if (commandLength < sizeof(commandBuffer) - 1) {
                commandBuffer[commandLength] = read;
                commandLength++;
            }
        }

    }


}

