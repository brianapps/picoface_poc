Custom gal chip

To compile use:

`galasm <filename.pld>`

and this produces .chp .fus .jed and .pin files

To program use

`minipro -p GAL22V10D -w GAL22V10.jed -u`

The GAL16v8 files also work with ATF16v8, use `-p ATF16V8B` as the device.

The -u flag makes the chip unprotected so you can double check things worked with

`minipro -p GAL22V10D --verify GAL22V10.jed`

There's also a python script (`updatesym.py`) that updates a KiCad part library file so that the pin names match those defined in the PLD files.


