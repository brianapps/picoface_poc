/*
Copyright (C) 2024 Brian Apps

This file is part of picoFace.

picoFace is free software: you can redistribute it and/or modify it under the terms of
the GNU General Public License as published by the Free Software Foundation, either
version 3 of the License, or (at your option) any later version.

picoFace is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with picoFace. If 
not, see <https://www.gnu.org/licenses/>.
*/

#include <stdarg.h>
#include "pico/stdlib.h"
#include "pico_hal.h"
#include "pico/stdio_usb.h"
#include "pico/stdio/driver.h"
#include "picorom.h"
#include "ff.h"
#include "hw_config.h"
#include "file_wrapper.h"



/*
Supported commands

ls [-json]
rm <filename>
mkdir <dirname>
mv <filename> <filename1>
upload <filename>
download <filename>

memupload <start>
memdownload <start> <length>
snapupload
snapdownload


*/


/*

Communication protocol

log messages are:

STX <text> ETX


Commands are sent to pico as

COMMAND_START <text> COMMAND_END


Then a 

COMMAND_RESPONSE_BEGIN

followed by an optional
COMMAND_RECEIVE_DATA  <DATA FROM CLIENT>

followed by an optional
COMMAND_SEND_DATA <DATA TO CLIENT>

followed by one of

COMMAND_SUCCESS

COMMAND_FAILURE

then optional text followed by

COMMAND_RESPONSE_END


Data transmission is as follows:


Sender begins with a
START_OF_DATA <length as 4 bytes LSB first>
receiver responds with either a
ACK to continue or NAK to stop

Sender then sends a number of packets
START_OF_PACKET <packet length as 2 bytes LSB first> <data packet as length bytes>
receiver responds with either a
ACK to continue or NAK to stop


Sender then sends to indicate end of data packet
END_OF_DATA
The recieve responds with either a 
ACK to indicate it has received it OK or NAK.



*/


constexpr uint8_t STX = 0x02;
constexpr uint8_t ETX = 0x03;

constexpr uint8_t COMMAND_START = 0x04;
constexpr uint8_t COMMAND_END = 0x05;
constexpr uint8_t COMMAND_RESPONSE_BEGIN = 0x06;
constexpr uint8_t COMMAND_RECEIVE_DATA = 0x07;
constexpr uint8_t COMMAND_SEND_DATA = 0x08;
constexpr uint8_t COMMAND_SUCCESS = 0x09;
// 0x0A -0x0F missed out because these include line feeds
constexpr uint8_t COMMAND_FAILURE = 0x10;
constexpr uint8_t COMMAND_RESPONSE_END = 0x11;



constexpr uint8_t START_OF_DATA = 0x14;
constexpr uint8_t START_OF_PACKET = 0x15;
constexpr uint8_t END_OF_DATA = 0x16;
constexpr uint8_t ACK = 0x17;
constexpr uint8_t NAK = 0x18;


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
    uint32_t totalExpecting; // or -1 if unspecified
    uint32_t totalRead;

    inline void putchar(char c) {
        stdio_usb.out_chars(&c, 1);
    }

  
    inline int getchars(char* buffer, int len, absolute_time_t expiry) {
        return usbGetChars(buffer, len, expiry);
    }

    void nextPacket(absolute_time_t expiry);

public:


    bool atEndOfFile() {
        if (!atEOF && isOK && packetRemaining == 0) {
            nextPacket(make_timeout_time_ms(8000));
        }
        return atEOF;
    }

    void close();

    bool begin(uint32_t bytesExpecting = -1);
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

            // written does not include the null terminator, so if written == avialable then
            // the entire string has not been written because vsnprintf will always null terminate
            if (written < available) {
                bufferUsed += written;
                return true;
            }
        }

        // The printf can't fit into the entire buffer so just abandon the attempt
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
    header[0] = COMMAND_SEND_DATA;
    header[1] = START_OF_DATA;
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
        char header[3];
        header[0] = START_OF_PACKET;
        header[1] = 0xFF & (lenToSend >> 8);
        header[2] = 0xFF & lenToSend;
        stdio_usb.out_chars(header, 3);
        stdio_usb.out_chars(buffer + sentSoFar, lenToSend);
        sentSoFar += lenToSend;
        char c;
        isOK = getchars(&c, 1, make_timeout_time_ms(2000)) == 1 && c == ACK;
    }
    return isOK;
}

bool CommStreamWriter::end() {
    if (isOK) {
        char c = END_OF_DATA;
        putchar(c);
        isOK = getchars(&c, 1, make_timeout_time_ms(2000)) == 1 && c == ACK;
    }
    return isOK;
}




bool CommStreamReader::begin(uint32_t bytesExpecting) {
    atEOF = false;
    totalExpecting = bytesExpecting;
    totalRead = 0;

    putchar(COMMAND_RECEIVE_DATA);

    absolute_time_t packetTimeout = make_timeout_time_ms(80000);
    char header[6];

    packetRemaining = 0;

    // Return the header which should be START_OF_DATA LEN*4
    if (getchars(header, 5, packetTimeout) != 5) {
        isOK = false;
    }
    else {
        isOK = header[0] == START_OF_DATA;
        if (isOK)
            total_size = (header[1] << 24) | (header[2] << 16) | (header[3] << 8) | (header[4]);
    }

    if (!isOK) 
        putchar(NAK);
    return isOK;
}


// Some client expect a packet to be completely read before they accept responses
// so this routine completes (and discards) data in outstanding packets
void CommStreamReader::close() {
    if (isOK && !atEOF) {
        absolute_time_t expiry = make_timeout_time_ms(8000);
        while (packetRemaining > 0) {
            char buffer[1024];
            int toRead = MIN(sizeof(buffer), packetRemaining);
            int read = getchars(buffer, toRead, expiry);  
            if (read < 0)
                break;
            packetRemaining -= read;
        }
        isOK = false;
    }
}


void CommStreamReader::nextPacket(absolute_time_t expiry) {
    char header[4];
    // Acknowledge have processed the prevous packet and ready for the next
    putchar(ACK);
    isOK = getchars(header, 1, expiry) == 1;

    if (isOK) {
        if (header[0] == END_OF_DATA) {
            atEOF = true;
            if (totalExpecting != -1 && totalExpecting != totalRead) {
                putchar(NAK);
                printf("Reached end but expected %d bytes, recieved %d\n", totalExpecting, totalRead);
            }
            else {
                putchar(ACK);
            }
        }
        if (header[0] == START_OF_PACKET) {
            isOK = getchars(header, 2, expiry) == 2;
            if (isOK) {
                packetRemaining = (header[0] << 8) | header[1];
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
                totalRead += read;
                if (packetRemaining == 0) {
                    nextPacket(expiry);
                }
            }
            else {
                isOK = false;
                putchar(NAK);
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

    FileHandle file;

    file.open(name, false);
    
    if (reader.begin()) {
        char buffer[4096];
        while (true) {
            int read = reader.read(buffer, sizeof(buffer), make_timeout_time_ms(80000));
            if (read == 0) {
                putchar(COMMAND_SUCCESS);
                break;
            }
            else if (read < 0) {
                putchar(COMMAND_FAILURE);
                printf("Error\n");
                break;
            }

            file.write(buffer, read);
        }
    }

    file.close();
}


void handleListFiles(const char* name, bool json=false) {
    CommStreamBufferWriter<1024> writer;

    writer.begin();
    if (json) {
        writer.printf("{\"files\": [");
    }
    else {
        writer.printf("Listing files for %s\n", name);
    }
    DirHandle dir;
    bool first = true;

    if (dir.open(name)) {
        while (dir.fileName()[0] != '\0') {
            if (json) {
                if (!first) {
                    writer.printf(",");
                }
                // TODO esccape the file name
                writer.printf("{\"name\": \"%s\", \"dir\": %d, \"size\": %d}", 
                    dir.fileName(), dir.isDirectory(), dir.fileSize());
                first = false;
            }
            else{
                writer.printf("%s, %d, %d\n", dir.fileName(), dir.fileSize(), dir.isDirectory());
            }
            if (!dir.next()) {
                break;
            }
        }

        if (json)
            writer.printf("]");

        uint32_t blockCount, blockSize, blocksUsed;
        if (dir.getStats(blockSize, blockCount, blocksUsed)) {
            if(json) {
                writer.printf(",\"stats\": {\"count\":%d, \"size\":%d, \"used\":%d}", blockCount, blockSize, blocksUsed);
            }
            else {
                writer.printf("FSInfo count=%d, size=%d, used=%d\n", blockCount, blockSize, blocksUsed);
            }
        }
    }
    else {
       writer.printf("Failed to open directory %d\n", dir);
    }

    if (json) {
        writer.printf("}");
    }


    writer.end();
    putchar(COMMAND_SUCCESS);
}





void handleDownload(const char* filename) {

    FileHandle file;

    if (!file.open(filename, true)) {
        putchar(COMMAND_FAILURE);
        printf("Failed to open file to download %s\n", filename);
        return;
    }

    char buffer[4096];
    CommStreamWriter writer;
    writer.begin(file.size());

    while (true) {
        int read = file.read(buffer, sizeof(buffer));

        if (read == 0) {
            writer.end();
            putchar(COMMAND_SUCCESS);
            break;
        }
        else if (read < 0) {
            writer.end();
            putchar(COMMAND_FAILURE);
            printf("pico_read returned %d\n", read);
            break;
        }
        writer.write(buffer, read, make_timeout_time_ms(2000));
    }

    file.close();
}


void handleDelete(const char* filename) {
    int res = pico_remove(filename);
    if (res < 0) {
        putchar(COMMAND_FAILURE);
        printf("Failed to delete file: %s (%d)", filename, res);
    }
    else {
        putchar(COMMAND_SUCCESS);
    }
}


void handleRename(const char* from, const char* to) {
    int res = pico_rename(from, to);
    if (res < 0) {
        putchar(COMMAND_FAILURE);
        printf("Failed to rename file: %s to %s (%d)", from, to, res);
    }
    else {
        putchar(COMMAND_SUCCESS);
    }
}


void handleMkDir(const char* filename) {
    int res = pico_mkdir(filename);
    if (res < 0) {
        putchar(COMMAND_FAILURE);
        printf("Failed to create directory: %s (%d)", filename, res);
    }
    else {
        putchar(COMMAND_SUCCESS);
    }
}


void handleSnapDownload() {
    size_t snapShotlength;
    const uint8_t* data = getSnapshotData(snapShotlength);

    if (data == nullptr) {
        putchar(COMMAND_FAILURE);
        printf("Failed to get snapshot data");
    }
    else {
        CommStreamWriter writer;
        writer.begin(snapShotlength);
        writer.write(reinterpret_cast<const char*>(data), snapShotlength, make_timeout_time_ms(8000));
        writer.end();
        putchar(COMMAND_SUCCESS);
    }
}

void handleSnapUpload() {
    CommStreamReader reader;
    if (reader.begin()) {
        uint8_t* buffer = beginSendSnapDataToMachine();
        int read = reader.read(reinterpret_cast<char*>(buffer), Z80_FILE_SIZE, make_timeout_time_ms(8000));
        if (!reader.atEndOfFile()) {
            reader.close();
            putchar(COMMAND_FAILURE);
            printf("Error, snapshot data size is wrong got %d more available.", read);
        }
        else if (read != SNA_FILE_SIZE && read != Z80_FILE_SIZE) {
            putchar(COMMAND_FAILURE);
            printf("Error, snapshot data size is not supported  %d.", read);
        }
        else if (!endSendSnapDataToMachine(read)) {
            putchar(COMMAND_FAILURE);
            printf("Failed to send snapshot to machine.");
        }
        else {
            putchar(COMMAND_SUCCESS);
        }
    }
    else {
        putchar(COMMAND_FAILURE);
        printf("!!!");
    }
}

void handleMemDownload(const char* start, const char* len) {
    uint32_t startAddress = atoi(start);
    uint32_t lengthCount = atoi(len);

    if (startAddress >= 16384 && startAddress + lengthCount < 0x10000) {
        if (!readMachineMemToLoadBuffer(startAddress, lengthCount)) {
            putchar(COMMAND_FAILURE);
            printf("Failed to read machine memory.");
            return;
        }
        CommStreamWriter writer;
        writer.begin(lengthCount);
        uint8_t* data = getRamLoadBuffer() + startAddress - 16384;
        writer.write(reinterpret_cast<const char*>(data), lengthCount, make_timeout_time_ms(8000));
        writer.end();
        putchar(COMMAND_SUCCESS);
        return;
    }

    putchar(COMMAND_FAILURE);
    printf("Bad params to memdownload.");
    return;
}


void handleMemUpload(const char* start) {
    uint32_t startAddress = atoi(start);

    if (startAddress >= 16384 && startAddress < 0x10000) {
        uint8_t* data = getRamLoadBuffer() + startAddress - 16384;
        int maxLen = 0x10000 - startAddress;

        CommStreamReader reader;
        reader.begin();
        int bytesRead = reader.read(reinterpret_cast<char*>(data), maxLen, make_timeout_time_ms(8000));
        reader.close();

        if (bytesRead <= 0) {
            putchar(COMMAND_FAILURE);
            printf("Bad memory buffer %d, %d.", startAddress, bytesRead);
        }
        else {
            if (!sendLoadBufferToMachine(startAddress, bytesRead)) {
                putchar(COMMAND_FAILURE);
                printf("Failed to send buffer to machine.");    
            }
        }

        putchar(COMMAND_SUCCESS);
        return;
    }

    putchar(COMMAND_FAILURE);
    printf("Bad params to memdownload.");
    return;


}


static char commandBuffer[512];
static size_t commandLength = 0;




void pollUsbCommandHandler() {
    char read;
    if (stdio_usb.in_chars(&read, 1) == 1) {

        if (read == COMMAND_END) {
            commandBuffer[commandLength] = 0;
            int args = escapeCommandLine(commandBuffer);

            putchar(COMMAND_RESPONSE_BEGIN);
            // process command
            if (strcmp(commandBuffer, "upload") == 0) {
                const char* name = commandBuffer + 7;
                handleupload(name);
            }
            else if (strcmp(commandBuffer, "ls") == 0) {
                const char* name = commandBuffer + strlen(commandBuffer) + 1;
                bool isJson = args == 3 && (strcmp("/json", name + strlen(name) + 1) == 0);
                handleListFiles(name, isJson);
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
            else if (strcmp(commandBuffer, "snapupload") == 0) {
                handleSnapUpload();
            }
            else if (strcmp(commandBuffer, "memdownload") == 0) {
                if (args != 3) {
                    printf("Xmemdownload bad args\n");
                }
                else {
                    const char* n1 = commandBuffer + strlen(commandBuffer) + 1;
                    const char* n2 = n1 + strlen(n1) + 1;
                    handleMemDownload(n1, n2);
                }

            }
            else if (strcmp(commandBuffer, "memupload") == 0) {
                const char* n1 = commandBuffer + strlen(commandBuffer) + 1;
                handleMemUpload(n1);
            }              
            else {
                putchar(COMMAND_FAILURE);
                printf("Unknown command: %s\n", commandBuffer);
            }
            putchar(COMMAND_RESPONSE_END);
            commandLength = 0;
        }
        else {

            if (commandLength == 0) {
                if (read == COMMAND_START) {
                    commandBuffer[commandLength] = ' ';
                    commandLength++;
                }
            }
            else if (commandLength < sizeof(commandBuffer) - 1) {
                commandBuffer[commandLength] = read;
                commandLength++;
            }
        }

    }


}

