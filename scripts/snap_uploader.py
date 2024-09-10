#!/usr/bin/env python3


# Copyright (C) 2024 Brian Apps
#
# This file is part of picoFace.
#
# picoFace is free software: you can redistribute it and/or modify it under the terms of
# the GNU General Public License as published by the Free Software Foundation, either
# version 3 of the License, or (at your option) any later version.
#
# picoFace is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with picoFace. If 
# not, see <https://www.gnu.org/licenses/>.


import sys
from PySide6.QtWidgets import QApplication, QWidget, QListWidget, QVBoxLayout, QListWidgetItem
import zipfile
import usb

snapzips = zipfile.ZipFile('snaps.zip', 'r', compression=zipfile.ZIP_DEFLATED)

app = QApplication(sys.argv)
window = QWidget()
layout = QVBoxLayout(window)

list_view = QListWidget()
for zi in snapzips.filelist:
    lwi = QListWidgetItem(zi.filename)
    list_view.addItem(lwi)

def myclick(itm : QListWidgetItem):
    with snapzips.open(itm.text()) as fp:
        data = fp.read()
    usb.send_command(usb.default_port(), "snapupload", data, None)

list_view.sortItems()
list_view.itemDoubleClicked.connect(myclick)

layout.addWidget(list_view)
window.resize(800, 800)
window.show()

sys.exit(app.exec())