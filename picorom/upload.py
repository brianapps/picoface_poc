#!/usr/bin/env python3

import usb
import argparse
import pathlib
import re
import lz4
import convertZ80


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description = 'Upload snap shot'
    )

    parser.add_argument('-p', '--port', default='/dev/ttyACM0')
    parser.add_argument('filename')

    args = parser.parse_args()


    file = pathlib.Path(args.filename)

    print(file.name)

    s = file.stem


    
    s = re.sub("\([^)]+\)", "", s)
    s = re.sub("\[[^]]+\]", "", s).strip()
    print(s)


    if file.suffix.lower() == ".z80":
        print("convert z80 to v1 uncompressed")
        snapshot = convertZ80.Z80Snapshot()
        snapshot.process_file(args.filename)
        input_data = snapshot.to_bytes()
    else:
        input_data = file.read_bytes()

    input_data = lz4.block.compress(input_data, mode='high_compression', store_size=False, compression=12)    

    command = usb.join_params( 
        [ "upload", s + file.suffix + "z"]
    )

    usb.send_command(args.port, command, input_data, None)
