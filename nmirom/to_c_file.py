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


with open("../picorom/nmi.c", "w") as fp:
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
    fp.write(f"const uint32_t EXITNMI = 0x{exitnmi:x};\n")

    # fp.write(f"const uint16_t STARTUP_COMMAND_OFFSET = 0x{startup_command:x};\n")
    # fp.write(f"const uint16_t STARTUP_PARAM1_OFFSET = 0x{startup_param1:x};\n")
    # fp.write(f"const uint16_t STARTUP_PARAM2_OFFSET = 0x{startup_param2:x};\n")


with open("../picorom/nmi.h", "w") as fp:

    fp.write("extern const uint8_t NMI_ROM[];\n")
    fp.write("extern const uint32_t NMI_ROM_SIZE;\n")
    fp.write("extern const uint32_t EXITNMI;\n")

    fp.write(f"#define STARTUP_COMMAND_OFFSET 0x{startup_command:x}\n")
    fp.write(f"#define STARTUP_PARAM1_OFFSET 0x{startup_param1:x}\n")
    fp.write(f"#define STARTUP_PARAM2_OFFSET 0x{startup_param2:x}\n")    

    for k, v in symbols.items():
        if k.startswith("STARTUP_ACTION_"):
            fp.write(f"#define {k} 0x{v:x}\n")




