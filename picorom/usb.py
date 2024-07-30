#!/usr/bin/env python3

import serial
import struct
import argparse
import lz4.block
import time
import convertZ80

ACK = b'\x06'

SOH = b'\x01'
EOT = b'\x04'
ACK = b'\x06'
NACK = b'X'

PACKET_SIZE = 8192 * 3

#PACKET_SIZE=4096


def sendData(port, cmd, bytesToSend):
    with serial.Serial(port, timeout=8) as ser:
        ser.write("\r\r".encode())
        ser.read_all()
        
        ser.write((cmd + "\r").encode('ascii'))

        if ser.read(1) != ACK:
            print("Expecting ACK in first instance but didn't get it")
            print(ser.read_all())
            return
        
        header =  struct.pack(">BBL", 1, 0, len(bytesToSend))
        ser.write(header)
        if (resp := ser.read(1)) != ACK:
            print(f"Expecting ACK but didn't get it. Got {resp} instead.")
            print(ser.read_all())
            return
        
        sentSoFar = 0
        while sentSoFar < len(bytesToSend):
            packetSize = min(len(bytesToSend) - sentSoFar, PACKET_SIZE)
            ser.write(struct.pack(">BBH", 1, 1, packetSize))
            packetBytes = bytesToSend[sentSoFar:sentSoFar+packetSize]
            ser.write(packetBytes)

           
            if ser.read(1) != ACK:
                print("Expecting ACK but didn't get it")
                print(ser.read_all())
                return

            sentSoFar += len(packetBytes)

        ser.write(b'\x04') # EOT
        if ser.read(1) != ACK:
            print("Expecting ACK from EOT but didn't get it")
            print(ser.read_all())
            return
        
        time.sleep(0.1)
        for i in range(10):
            b = ser.read_all()
            if len(b) > 0:
                print(b)



def receiveData(port, cmd):
    data = bytes()
    with serial.Serial(port, timeout=8) as ser:
        ser.write("\r\r".encode())
        ser.read_all()
        ser.write((cmd + "\r").encode('iso8859_2'))

        status = ser.read(1)

        if status == NACK:
            print("Error: " + ser.read_all().decode())
            return data
        elif status == ACK:
            print("OK: " + ser.read_all().decode())
            return data
        elif status != SOH:
            ser.write(NACK)
            print(f"Expecting SOH {status}")
            print(ser.read_all())
            return data

        header = ser.read(5)

        if header[0] != 0:
            print("Expecting begin header")
            ser.write(NACK)
            return

        total_size = struct.unpack(">L", header[1:] )[0]
        ser.write(ACK)


        while True:
            packettype = ser.read(1)
            if packettype == EOT:
                ser.write(ACK)
                return data
            elif packettype == SOH:
                header = ser.read(3)
                if header[0] != 1:
                    print("Expecting continue header")
                    ser.write(NACK)
                    return
                packet_size = struct.unpack(">H", header[1:])[0]
                data += ser.read(packet_size)
                ser.write(ACK)
            else:
                ser.write(NACK)
                print(f"Didn't get a SOT or EOT {packettype}:{ser.read_all()}")
                return data


def escape_param(param : str) -> str:
    return param.replace(" ", "\\ ")


def join_params(params: list[str]):
    return ' '.join(x.replace(" ", "\\ ") for x in params)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description = 'send commands to pico board over serial usb'
    )

    parser.add_argument('-p', '--port', default='/dev/ttyACM0')
    parser.add_argument('-i', '--input', help='input file')
    parser.add_argument('-z', '--compress', help='compress the input', action='store_const', const=True)
    parser.add_argument('-o', '--output', help='output file')
    parser.add_argument('cmdparams', nargs='+')

    args = parser.parse_args()

    if args.input is None:
        data = receiveData(args.port, join_params(args.cmdparams))
        if args.output is None:
            print(data)
            print(data.decode('ascii',errors='ignore'))
        else:
            with open(args.output, "wb") as fp:
                fp.write(data)
    else:
        if args.input.lower().endswith(".z80"):
            print("convert z80 to v1 uncompressed")
            snapshot = convertZ80.Z80Snapshot()
            snapshot.process_file(args.input)
            data = snapshot.to_bytes()
        else:
            with open(args.input, 'rb') as fp:
                data = fp.read()
        if args.compress:
            data = lz4.block.compress(data, mode='high_compression', store_size=False, compression=12)
        sendData(args.port, join_params(args.cmdparams), data)


