import pathlib
import lz4.block
import requests



def convert_sna(sna_file_input, name : str):
    snadata = pathlib.Path(sna_file_input).read_bytes()

    if len(snadata) != 48 * 1024 + 27:
        raise("SNA is wrong size")
    
    snadata = lz4.block.compress(snadata, mode='high_compression', store_size=False, compression=12)

    print(f"{name} compressed to {len(snadata)} bytes.")


    requests.post(f"http://192.168.1.140/{name}.snaz", data=snadata)


convert_sna('sna/Manic Miner.sna', 'manic')
convert_sna('sna/Knight.sna', 'knightl')
convert_sna('sna/matchday2.sna', 'knight')


# convert_sna('sna/Wheelie.sna', 'manic')
# convert_sna('sna/Underwurlde.sna', 'knight')