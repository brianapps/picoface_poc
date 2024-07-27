#ifndef INCLUDED_PICOROM_H
#define INCLUDED_PICOROM_H

#define SNA_SIZE 49179


// Attempt to obtain snapshot from the running spectrum
const uint8_t* getSnapshotData(size_t& snapshotLength);

uint8_t* beginSendSnapDataToMachine();
bool endSendSnapDataToMachine();

// Return a pointer to 48K of memory used a so load/save
// buffer between the pico and the real device. This pointer
// represents memory address 16384
uint8_t* getRamLoadBuffer();

bool sendLoadBufferToMachine(uint32_t startAddress, uint32_t length);

bool readMachineMemToLoadBuffer(uint32_t startAddress, uint32_t length);



#endif

