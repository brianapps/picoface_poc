

with open("gw03.rom", "rb") as fp:
    romdata = fp.read()

print('#include "pico/stdlib.h"\n')

print("const uint8_t FGH_ROM[] = {", end="")

for i in range(0, len(romdata)):
    if i % 16 == 0:
        print("\n    ", end="")
    print(f"0x{romdata[i]:02x}, ", end="")


print("\n};")

print("\nconst uint32_t FGH_ROM_SIZE = sizeof(FGH_ROM);\n")

