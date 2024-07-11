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


# convert_sna('Manic Miner.sna', 'ManicMiner')
# convert_sna('Knight.sna', 'KnightLore')
# convert_sna('matchday2.sna', 'MatchDay2')
# convert_sna('Wheelie.sna', 'Wheelie')
# convert_sna('Underwurlde.sna', 'Underwurlde')
# convert_sna('Fist.sna', 'ExplodingFist')
# convert_sna('Atic.sna', 'AticAtac')

#convert_sna('Sabre.sna', 'Sabre Wulf')

convert_sna('Jetman.sna', 'LunarJetman')
convert_sna('jetpack.sna', 'JetPack')


# convert_sna('sna/Wheelie.sna', 'manic')
# convert_sna('sna/Underwurlde.sna', 'knight')