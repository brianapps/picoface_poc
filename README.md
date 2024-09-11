# picoFace

The picoFace is 


For a demonstration of the device in action see this video:

[![picoFace Video](https://img.youtube.com/vi/BUWlcWCz8AM/0.jpg)](https://www.youtube.com/watch?v=BUWlcWCz8AM)



## Building the pico firmware

The firmware build relies on the following being installed and on the path:

- [sjasmplus](https://github.com/z00m128/sjasmplus)
- Python 3.x
- Raspberry Pi Pico SDK 
- VSCode and VSCode Pico plugin.

Building is performed by CMake, for example on Linux:

```
mkdir build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ../pico
ninja
```

or on Windows:

```
md build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ../pico
ninja
```

This will create the `picoface.uf2` that can be uploaded to the Pico e.g. using the `picotool`

```
picotool load picoface.uf2 -f
```

## GALASM

The [gal](./gal) folder holds the logic design for the PLDs used by picoface.

The logic is used to multiplex the address lines, perform memory access decoding, and

- `GALV22V10.pld` The original logic for the proof of concept design. Two identical GAL22V10s are used and some redudant logic is duplicated.
- `GAL16v8_A.pld` and `GAL16v8_B.pld`. The revised design for the prototype PCB. The logic was reworked to fit onto smaller chips and redudant logic removed.

The `.pld` should be compiled with [GALasm](https://github.com/daveho/GALasm) as follows:

```
galasm <filename.pld>
```

and this produces `.chp`, `.fus`, `.jed` and `.pin` files.

I programmed the chips using a XGecu T48 programmer using the [minipro](https://gitlab.com/DavidGriffith/minipro) programmer.

To program the GAL22V10D use:

```
minipro -p GAL22V10D -w GAL22V10.jed -u
```

And to program the ATF16V8Bs use


```
minipro -p ATF16V8B -w GAL16v8A.jed -u
```

The `-u` flag makes the chip unprotected so you can double check things worked with

`minipro -p GAL22V10D --verify GAL22V10.jed`

There's also a python script [`updatesym.py`](gal/updatesym.py) that updates a custom KiCad part library so the the pin names in KiCad match those specified in the .pld file. This ensures parts placed in the schematic have the correct pin names (and this can be with KiCad pin name feature) and that they also mapped to the correct pin numbers on the footprint. This eliminates transcribing errors helps ensure PCB are designed and routed correctly.

## Pico


## nmirom



## Scripts

* `usb.py`. This script handles the command line interface to the picoface. It uses the pySerial library to communicate with the RPi Pico via the USB port. 


## License

picoFace is free software: you can redistribute it and/or modify it under the terms of
the GNU General Public License as published by the Free Software Foundation, either
version 3 of the License, or (at your option) any later version.

picoFace is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

A copy of the license is provided in the [LICENSE](./LICENSE) file.


## Acknowledgments

The picoface firmware includes the following libraries:

* [pico-littlefs](https://github.com/lurk101/pico-littlefs) by lurk101
* [no-OS-FatFS-SD-SDIO-SPI-RPi-Pico](https://github.com/carlk3/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico) by Carl J Kugler III
* [lz4](https://github.com/lz4/lz4) by Yann Collet.
* [Carton](https://damieng.com/typography/zx-origins/carton/) font by Damien Guard

