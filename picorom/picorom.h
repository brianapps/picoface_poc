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


#ifndef INCLUDED_PICOROM_H
#define INCLUDED_PICOROM_H

#define SNA_HEADER_SIZE 27
#define Z80_HEADER_SIZE 30

#define SNA_FILE_SIZE 49179
#define Z80_FILE_SIZE (48 * 1024 + Z80_HEADER_SIZE)


// Attempt to obtain snapshot from the running spectrum
const uint8_t* getSnapshotData(size_t& snapshotLength);

uint8_t* beginSendSnapDataToMachine();
bool endSendSnapDataToMachine(uint32_t snapshotLength);

// Return a pointer to 48K of memory used a so load/save
// buffer between the pico and the real device. This pointer
// represents memory address 16384
uint8_t* getRamLoadBuffer();

bool sendLoadBufferToMachine(uint32_t startAddress, uint32_t length);

bool readMachineMemToLoadBuffer(uint32_t startAddress, uint32_t length);



#endif

