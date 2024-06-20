import pathlib



def convert_sna(sna_file_input, name : str):
    snadata = pathlib.Path(sna_file_input).read_bytes()


    with open(f"{name}.c", "w") as fp:
        fp.write("#include \"pico/stdlib.h\"\n\n")

        fp.write(f"const uint8_t {name.upper()}_DATA[] = {{")

        for i in range(0, len(snadata)):
            if i % 16 == 0:
                fp.write("\n    ")
            fp.write(f"0x{snadata[i]:02x}, ")
        fp.write("\n};\n\n")


        fp.write(f"const uint32_t {name.upper()}_SIZE = sizeof({name.upper()}_DATA);\n")




convert_sna('sna/Manic Miner.sna', 'manic')