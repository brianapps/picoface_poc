import pathlib
import lz4.block



def convert_sna(sna_file_input, name : str):
    snadata = pathlib.Path(sna_file_input).read_bytes()

    if len(snadata) != 48 * 1024 + 27:
        raise("SNA is wrong size")
    
    snadata = lz4.block.compress(snadata, mode='high_compression', store_size=False, compression=12)

    print(f"{name} compressed to {len(snadata)} bytes.")


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
convert_sna('sna/Knight.sna', 'knight')
convert_sna('sna/matchday2.sna', 'matchday2')