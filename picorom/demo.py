import pathlib


romdata = pathlib.Path('demo.raw').read_bytes()


print("const uint8_t NMI_ROM[] = {", end="")

for i in range(0, len(romdata)):
    if i % 16 == 0:
        print("\n    ", end="")
    print(f"0x{romdata[i]:02x}, ", end="")


print("\n};")

