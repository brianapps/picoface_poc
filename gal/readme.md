Custom gal chip

To compile use:

galasm <filename.pld>

and this produces .chp .fus .jed and .pin files

To program use

minipro -p GAL22V10D -w GAL22V10.jed -u

The -u flag makes the chip unprotected so you can double check things worked with

minipro -p GAL22V10D --verify GAL22V10.jed 
