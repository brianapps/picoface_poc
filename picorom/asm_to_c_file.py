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


import pathlib

romdata = pathlib.Path('nmirom.bin').read_bytes()

symbols = {}

with pathlib.Path('nmirom.sym').open("r") as fp:
    for l in fp.readlines():

        toks = l.split(': EQU')
        if len(toks) == 2:
            symbols[toks[0].strip()] = int(toks[1].strip(), 16)


exitnmi = symbols["exitnmi"]
startup_param2 = symbols["start_up_param2"]
startup_param1 = symbols["start_up_param1"]
startup_command = symbols["start_up_action"]


with open("nmi.c", "w") as fp:
    fp.write("#include \"pico/stdlib.h\"\n\n")

    fp.write("const uint8_t NMI_ROM[] = {")

    for i in range(0, len(romdata)):
        if i % 16 == 0:
            fp.write("\n    ")
        fp.write(f"0x{romdata[i]:02x}, ")
    fp.write("\n};\n\n")


    fp.write("const uint32_t NMI_ROM_SIZE = sizeof(NMI_ROM);\n")

    print(f"exit op code: {romdata[exitnmi]}\n")
    if romdata[exitnmi] != 201:
        print("exit is not simple ret")
        exitnmi += 1
    

    # fp.write(f"const uint16_t STARTUP_COMMAND_OFFSET = 0x{startup_command:x};\n")
    # fp.write(f"const uint16_t STARTUP_PARAM1_OFFSET = 0x{startup_param1:x};\n")
    # fp.write(f"const uint16_t STARTUP_PARAM2_OFFSET = 0x{startup_param2:x};\n")


with open("nmi.h", "w") as fp:

    fp.write("#ifndef __ASSEMBLER__\n")
    fp.write("extern const uint8_t NMI_ROM[];\n")
    fp.write("extern const uint32_t NMI_ROM_SIZE;\n")
    fp.write("#endif\n")


    for k, v in symbols.items():
        if k.startswith("ACTION_"):
            fp.write(f"#define {k} {v}\n")



    fp.write(f"#define EXITNMI 0x{exitnmi:x}\n")
    fp.write(f"#define STARTUP_COMMAND_OFFSET 0x{startup_command:x}\n")
    fp.write(f"#define STARTUP_PARAM1_OFFSET 0x{startup_param1:x}\n")
    fp.write(f"#define STARTUP_PARAM2_OFFSET 0x{startup_param2:x}\n") 
    fp.write(f"#define STARTUP_SDCARD_PRESENT 0x{symbols['start_up_sdcard_present']:x}\n") 

    for k, v in symbols.items():
        if k.startswith("STARTUP_ACTION_"):
            fp.write(f"#define {k} 0x{v:x}\n")




