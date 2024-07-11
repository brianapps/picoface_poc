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


convert_sna('Manic Miner.sna', 'Manic Miner')
convert_sna('Knight.sna', 'Knight Lore')
convert_sna('matchday2.sna', 'Match Day II')
convert_sna('Wheelie.sna', 'Wheelie')
convert_sna('Underwurlde.sna', 'Underwurlde')
convert_sna('Fist.sna', 'Exploding Fist')
convert_sna('Atic.sna', 'Atic Atac')
convert_sna('Sabre.sna', 'Sabre Wulf')
convert_sna('Jetman.sna', 'Lunar Jetman')
convert_sna('jetpack.sna', 'Jet Pack')
convert_sna('SpyHunter.sna', 'Spy Hunter')
convert_sna('BoulderDash.sna', 'Boulder Dash')
convert_sna('Commando.sna', 'Commando')

convert_sna('Commando.sna', 'X0')
convert_sna('Commando.sna', 'X1')
convert_sna('Commando.sna', 'X2')
convert_sna('Commando.sna', 'X3')
convert_sna('Commando.sna', 'X4')
convert_sna('Commando.sna', 'X5')
convert_sna('Commando.sna', 'X6')
convert_sna('Commando.sna', 'X7')
convert_sna('Commando.sna', 'X8')
convert_sna('Commando.sna', 'X9')
convert_sna('Commando.sna', 'XA')


# convert_sna('sna/Wheelie.sna', 'manic')
# convert_sna('sna/Underwurlde.sna', 'knight')