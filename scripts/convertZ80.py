# Copyright (C) 2024 Brian Apps
#
# This file is part of picoFace.
#
# picoFace is free software: you can redistribute it and/or modify it under the terms of
# the GNU General Public License as published by the Free Software Foundation, either
# version 3 of the License, or (at your option) any later version.
#
# picoFace is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with picoFace. If 
# not, see <https://www.gnu.org/licenses/>.


from dataclasses import dataclass
import struct
import pathlib


@dataclass
class Header:
    A : int = 0
    F : int = 0
    BC : int = 0
    HL : int = 0
    PC : int = 0
    SP : int = 0
    I : int = 0
    R : int = 0
    flags1 : int = 9
    DE : int = 0
    exBC : int = 0
    exDE : int = 0
    exHL : int = 0
    exA : int = 0
    exF : int = 0
    IY : int = 0
    IX : int = 0
    IFF: int  = 0
    IFF2: int  = 0
    flags2 : int = 0

    v1format: bool = True
    v3format : bool = False

    v2PC : int = 0
    v2HardwareMode : int = 0



    def from_bytes(self, data):
        (self.A, self.F, self.BC, self.HL, self.PC, 
         self.SP,
         self.I, self.R, self.flags1, self.DE,
         self.exBC, self.exDE, self.exHL,
         self.exA, self.exF, self.IY, self.IX,
         self.IFF, self.IFF2, self.flags2) = struct.unpack(
             "<BBHHHHBBBHHHHBBHHBBB", data[:30])
        
        self.v1format = self.PC != 0
        if self.v1format:
            return 30
        else:
            addlen = struct.unpack("<H", data[30:32])[0]
            (self.v2PC, self.v2HardwareMode) = struct.unpack("<HB", data[32:35])
            self.v3format = addlen == 54 or addlen == 55
            return 30 + addlen + 2
        
    def is48KSnapshot(self):
       return (self.v2HardwareMode == 0 or self.v2HardwareMode == 1 or
                (self.v3format and self.v2HardwareMode == 3))
                 


def decomp(bytes, start, length, outbytes):
    outlen = 0
    i = start
    while i < start + length:
        if bytes[i] == 0xED and i + 1 < start + length:
            if bytes[i + 1] == 0xED:
                runlen = bytes[i + 2]
                outbytes[outlen : outlen + runlen] = [bytes[i + 3]] * runlen
                outlen += runlen
                i += 4
            else:
                outbytes[outlen : outlen + 2] = bytes[i : i + 2]
                outlen += 2
                i += 2
        else:
            outbytes[outlen] = bytes[i]
            outlen += 1
            i += 1

    return outlen


page_to_address = {
    4: 0x8000,
    5: 0xc000,
    8: 0x4000,
}

class Z80Snapshot:

    def __init__(self):
        self.membuffer = bytearray(48 * 1024)
        self.header = Header()

    def process_bytes(self, file_bytes):
        off = self.header.from_bytes(file_bytes)
        # print(self.header)
        decomp_data = bytearray(0x4000)

        if not self.header.is48KSnapshot():
            raise Exception(f"Z80 file is not for 48K spectrum, mode={self.header.v2HardwareMode}")

        if not self.header.v1format:
            while off < len(file_bytes):
                blocklen, pagenumber = struct.unpack("<HB", file_bytes[off : off + 3])
                # print(f"Block: Page={pagenumber}, length={blocklen}")
                page_addr = page_to_address.get(pagenumber)
                if page_addr is None:
                    raise Exception(f"Unsupported page number={pagenumber}")
                
                off += 3

                if blocklen != 0xFFFF:
                    s = decomp(file_bytes, off, blocklen, decomp_data)
                    self.membuffer[page_addr - 0x4000 : page_addr] = decomp_data
                    # print(f"Decomp length={s}")
                    off += blocklen
                else:
                    self.membuffer[page_addr - 0x4000 : page_addr] = file_bytes[off : off + 0x4000 ]
                    off += 16384
        else:
            if (self.header.flags1 & 1 << 5) != 0:
                # print("V1 compressed")
                if file_bytes[-4:] != bytes([0, 0xED, 0xED , 0]):
                    raise Exception("No end of block marker present")
                s = decomp(file_bytes, off, len(file_bytes) - 34, self.membuffer)
                # print(f"Decomp length={s}")
            else:
                self.membuffer = file_bytes[off: 48 * 1024 + off]
        

    def process_file(self, fn):
        file_bytes = pathlib.Path(fn).read_bytes()
        self.process_bytes(file_bytes)



    def to_bytes(self):
        """
        Obtain the snapshot as an uncompressed version 1 z80 file
        """
        outbytes = bytearray(48 * 1024 + 30)
        pc = self.header.PC if self.header.v1format  else self.header.v2PC

        outbytes[0:30] = struct.pack("<BBHHHHBBBHHHHBBHHBBB",
            self.header.A, self.header.F, self.header.BC, self.header.HL, pc, 
            self.header.SP, self.header.I, self.header.R, 
            self.header.flags1 & 0x1F, self.header.DE,
            self.header.exBC, self.header.exDE, self.header.exHL,
            self.header.exA, self.header.exF, self.header.IY, self.header.IX,
            self.header.IFF, self.header.IFF2, self.header.flags2)


        outbytes[30:] = self.membuffer
        return outbytes






if __name__ == "__main__":

    import argparse
    import lz4.block

    parser = argparse.ArgumentParser(
        description = 'Convert z80 file to z80z file'
    )

    parser.add_argument('infile')
    parser.add_argument('outfile')

    args = parser.parse_args()


    print("convert z80 to v1 uncompressed")
    snapshot = Z80Snapshot()
    snapshot.process_file(args.infile)
    input_data = snapshot.to_bytes()
    input_data = lz4.block.compress(input_data, mode='high_compression', store_size=False, compression=12)        

    with open(args.outfile, 'wb') as fp:
        fp.write(input_data)