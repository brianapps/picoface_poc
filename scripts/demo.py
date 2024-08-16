import pathlib


romdata = pathlib.Path('demo.raw').read_bytes()

exitnmi = -1

with pathlib.Path('demo.sym').open("r") as fp:
    for l in fp.readlines():
        if l.startswith("exitnmi: EQU "):
            exitnmi = int(l[13:], 16)



with open("nmi.c", "w") as fp:
    fp.write("#include \"pico/stdlib.h\"\n\n")

    fp.write("const uint8_t NMI_ROM[] = {")

    for i in range(0, len(romdata)):
        if i % 16 == 0:
            fp.write("\n    ")
        fp.write(f"0x{romdata[i]:02x}, ")
    fp.write("\n};\n\n")


    fp.write("const uint32_t NMI_ROM_SIZE = sizeof(NMI_ROM);\n")

    fp.write(f"const uint32_t EXITNMI = 0x{exitnmi + 1:x};\n")


