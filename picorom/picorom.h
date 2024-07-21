#ifndef INCLUDED_PICOROM_H
#define INCLUDED_PICOROM_H

#define SNA_SIZE 49179


// Attempt to obtain snapshot from the running spectrum
const uint8_t* getSnapshotData(size_t& snapshotLength);

uint8_t* beginSendSnapDataToMachine();
bool endSendSnapDataToMachine();

#endif

