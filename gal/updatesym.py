# Copyright (C) 2024 Brian Apps
#
# This file is part of picoFace.
#
# picoFace is free software: you can redistribute it and/or modify it under the terms of
# the GNU General Public License as published by the Free Software Foundation, either
# version 3 of the License, or (at your option) any later version.
#
# picoFace is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY# 
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with picoFace. If 
# not, see <https://www.gnu.org/licenses/>.



# Attempt to update a kicad parts library using the .pin file generated by GALasm

import pathlib
import sexpdata



def read_pin_defs(filename) -> dict:
    pin_defs = {}

    with open(filename, 'r') as fp:
        for l in fp.readlines()[4:]:
            toks = [x.strip() for x in l.split('|')]
            if len(toks) > 1:
                name = toks[1]
                if name.startswith('/'):
                    name = "~{" + name[1:] + "}"
                pin_defs[toks[0]] = (name, toks[2] == 'Output')
    return pin_defs



sym_file = pathlib.Path('/home/brian/dev/kicad/picoface_16v8') / 'mysyms.kicad_sym'


with sym_file.open('r') as fp:
    syms = sexpdata.load(fp)


kisym = sexpdata.Symbol('symbol')
kipin  = sexpdata.Symbol('pin')
kinumber  = sexpdata.Symbol('number')
kiname  = sexpdata.Symbol('name')
kiinput  = sexpdata.Symbol('input')
kioutput = sexpdata.Symbol('output')



def findallsymbols(sym_list, sym_to_find):
    syms_found = []
    for x in sym_list:
        if isinstance(x, list) and x[0] == sym_to_find:
            syms_found.append(x)
    return syms_found


def findfirstsymbol(sym_list, sym_to_find):
    for x in sym_list:
        if isinstance(x, list) and x[0] == sym_to_find:
            return x
    return None


def findsymbolname(syms, name):
    for x in syms:
        if isinstance(x, list):
            if x[0] == kisym and x[1] == name:
                return x
    return None



def update_symbol(syms, part_name, pin_file):

    pin_defs = read_pin_defs(pin_file)

    print(pin_defs)
    mypart = findsymbolname(syms, part_name)

    if mypart:
        print("Update it")
        partinfo = findfirstsymbol(mypart, kisym)
        print(partinfo[1])
        for pin in findallsymbols(partinfo, kipin):
            num = findfirstsymbol(pin, kinumber)
            name = findfirstsymbol(pin, kiname)

            if pin_def := pin_defs.get(num[1]):
                name[1] = pin_def[0]
                pin[1] = kioutput if pin_def[1] else kiinput

        

update_symbol(syms, 'GAL_CHIPA', 'GAL16v8_A.pin')
update_symbol(syms, 'GAL_CHIPB', 'GAL16v8_B.pin')



with sym_file.open('w') as fp:
    sexpdata.dump(syms, fp)