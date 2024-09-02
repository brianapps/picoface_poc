#!/usr/bin/env python3

import serial
import serial.tools.list_ports
import struct
import argparse
import lz4.block
import time
import convertZ80



STX = b'\x02'
ETX = b'\x03'

COMMAND_START = b'\x04'
COMMAND_END = b'\x05'
COMMAND_RESPONSE_BEGIN = b'\x06'
COMMAND_RECEIVE_DATA = b'\x07'
COMMAND_SEND_DATA = b'\x08'
COMMAND_SUCCESS = b'\x09'
COMMAND_FAILURE = b'\x10'
COMMAND_RESPONSE_END = b'\x11'
START_OF_DATA = b'\x14'
START_OF_PACKET = b'\x15'
END_OF_DATA = b'\x16'
ACK = b'\x17'
NAK = b'\x18'


PACKET_SIZE = 8192 * 3

#PACKET_SIZE=4096



def send_data(ser, bytesToSend):
       
    header =  struct.pack(">BL", START_OF_DATA[0], len(bytesToSend))
    ser.write(header)
    if (resp := ser.read(1)) != ACK:
        print(f"Expecting ACK but didn't get it. Got {resp} instead.")
        return
    
    sentSoFar = 0
    while sentSoFar < len(bytesToSend):
        packetSize = min(len(bytesToSend) - sentSoFar, PACKET_SIZE)
        ser.write(struct.pack(">BH", START_OF_PACKET[0], packetSize))
        packetBytes = bytesToSend[sentSoFar:sentSoFar+packetSize]
        ser.write(packetBytes)

        
        if ser.read(1) != ACK:
            print("Expecting ACK but didn't get it")
            return

        sentSoFar += len(packetBytes)

    ser.write(END_OF_DATA)
    if ser.read(1) != ACK:
        print("Expecting ACK from server but didn't get it")
        return
            

def receive_data(ser):
    data = bytes()
    header = ser.read(5)

    if header[0:1] != START_OF_DATA:
        print(f"Expecting start of data {header}, {START_OF_DATA}, {header[0]}")
        ser.write(NAK)
        return

    total_size = struct.unpack(">L", header[1:] )[0]
    ser.write(ACK)


    while True:
        packettype = ser.read(1)
        if packettype == END_OF_DATA:
            ser.write(ACK)
            return data
        elif packettype == START_OF_PACKET:
            header = ser.read(2)
            packet_size = struct.unpack(">H", header)[0]
            # print(f"packet: {packet_size}")
            packet_data = ser.read(packet_size)
            if len(packet_data) != packet_size:
                print("Didn't read the entire packet")
            data += packet_data
            ser.write(ACK)
        else:
            ser.write(NAK)
            print(f"Didn't get a START_OF_PACK or END_OF_DATA {packettype}")
            return data            


def send_command(port, command_text, input_data, output_file):
    with serial.Serial(port, timeout=2) as ser:
        recv = ser.read_all()
        while len(recv) != 0:
            recv = ser.read_all()

        ser.write(COMMAND_START + command_text.encode('ascii') + COMMAND_END)


        while True:
            status = ser.read(1)
            if len(status) == 0:
                print("Timed out")
                return

            if status == COMMAND_RESPONSE_BEGIN:
                break


        succeeded = False

        while True:
            status = ser.read(1)
            if len(status) == 0:
                print("Timed out")
                return

            if status == COMMAND_RECEIVE_DATA:
                send_data(ser, input_data)
            elif status == COMMAND_SEND_DATA:
                data = receive_data(ser)
                if output_file is None:
                    print(data.decode('ascii',errors='ignore'))
                elif isinstance(output_file, str):
                    with open(output_file, "wb") as fp:
                        fp.write(data)
                else:
                     output_file.write(data)   
            elif status == COMMAND_SUCCESS:
                succeeded = True
                break
            elif status == COMMAND_FAILURE:
                print("Command failed")
                break
            else:
                print(status)


        message_bytes = bytearray()

        while True:
            status = ser.read(1)
            if len(status) == 0:
                print("Timed out")
                return
            
            if status == COMMAND_RESPONSE_END:
                if len(message_bytes) > 0:
                    print(message_bytes.decode('ascii', errors='ignore'))
                return
            else:
                message_bytes.extend(status)
                
            
def compress_data(input_data: bytes) -> bytes:
    return lz4.block.compress(input_data, mode='high_compression', store_size=False, compression=12)

def default_port() -> str:
    for p in serial.tools.list_ports.comports():
        if p.vid == 0x2E8A and p.pid == 0x0A:
            return p.device
    return "/dev/ttyACM0"
        
def escape_param(param : str) -> str:
    return param.replace(" ", "\\ ")


def join_params(params: list[str]):
    return ' '.join(x.replace(" ", "\\ ") for x in params)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description = 'send commands to pico board over serial usb'
    )

    parser.add_argument('-p', '--port', default=default_port())
    parser.add_argument('-i', '--input', help='input file')
    parser.add_argument('-z', '--compress', help='compress the input', action='store_const', const=True)
    parser.add_argument('-o', '--output', help='output file')
    parser.add_argument('cmdparams', nargs='+')

    args = parser.parse_args()

    input_data = None

    if args.input != None:
        if args.input.lower().endswith(".z80"):
            print("convert z80 to v1 uncompressed")
            snapshot = convertZ80.Z80Snapshot()
            snapshot.process_file(args.input)
            input_data = snapshot.to_bytes()
        else:
            with open(args.input, 'rb') as fp:
                input_data = fp.read()
        if args.compress:
            input_data = lz4.block.compress(input_data, mode='high_compression', store_size=False, compression=12)

    send_command(args.port, join_params(args.cmdparams), input_data, args.output)



