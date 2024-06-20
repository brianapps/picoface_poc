import pathlib


romdata = pathlib.Path('nmirom.bin').read_bytes()

exitnmi = -1

with pathlib.Path('nmirom.sym').open("r") as fp:
    for l in fp.readlines():
        l.strip()
        if l.startswith("exitnmi: EQU ") or l.startswith("nmiroutine.exit: EQU"):
            print(f"exit: {l[-12:]}")
            exitnmi = int(l[-12:], 16)



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


