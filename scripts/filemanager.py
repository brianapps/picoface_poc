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
from PySide6.QtWidgets import QApplication, QTableView, QMainWindow, QItemDelegate, QAbstractItemView, QTreeView,QMessageBox, QFileDialog, QDialog
from PySide6.QtGui import QDragEnterEvent, QDragMoveEvent, QKeyEvent
from PySide6.QtCore import QAbstractTableModel, QModelIndex

from  PySide6.QtCore import Qt, Signal
import usb
import io
import json
from pathlib import Path
from convertZ80 import Z80Snapshot

from filemanager_ui import Ui_MainWindow

import dataclasses


@dataclasses.dataclass
class FileSystemStats:
    block_size :  int = 0
    total_blocks: int = 0
    used_blocks: int = 0
    

def findchar(s : str, c) -> int:
    pos = s.find(c)
    return len(s) if pos < 0 else pos

def strip_guff(file_name : str) -> str:
    end = min(findchar(file_name, '('), findchar(file_name, '['))
    return file_name[0:end].strip()            

   

class MyTableModel(QAbstractTableModel):
    statsChanged = Signal(FileSystemStats)

    def __init__(self):
        super().__init__()
        self._data = []
        self._headers = ["Filename", "Size"]
        self._stats = FileSystemStats()
        self.updateData()


    def updateData(self):
        output = io.BytesIO()
        usb.send_command(usb.default_port(), "ls / /json", None, output)
        output.seek(0)
        d = json.load(output)
        newdata = [
            [f['name'], f['size']] for f in d['files'] if f['dir'] == 0]
        
        currentRows = len(self._data)
        newRows = len(newdata)

        olddata = self._data
        self._data = newdata
        
        if newRows > currentRows:
            self.beginInsertRows(QModelIndex(), currentRows, newRows - 1)
            self.endInsertRows()
        elif newRows < currentRows:
            self.beginRemoveRows(QModelIndex(), newRows, currentRows)
            self.endRemoveRows()

        for i in range(min(currentRows, newRows)):
            if olddata[i][0] != self._data[i][0]:
                ind = self.createIndex(i, 0)
                self.dataChanged.emit(ind, ind)
            if olddata[i][1] != self._data[i][1]:
                ind = self.createIndex(i, 1)
                self.dataChanged.emit(ind, ind)

        newStats = FileSystemStats(d['stats']['size'], d['stats']['count'], d['stats']['used'])

        if newStats != self._stats:
            self._stats = newStats
            self.statsChanged.emit(self._stats)


    def rowCount(self, parent):
        return len(self._data)

    def columnCount(self, parent):
        return 2

    def data(self, index, role):
        if not index.isValid():
            return None
        elif role != Qt.ItemDataRole.DisplayRole and role != Qt.ItemDataRole.EditRole:
            return None
        return self._data[index.row()][index.column()]
    
    def headerData(self, col, orientation, role):
        if orientation == Qt.Horizontal and role == Qt.DisplayRole:
            return self._headers[col]
        return None
        
    def setData(self, index, value, role) -> bool:
        oldname = self._data[index.row()][0]
        if oldname != value:
            usb.send_command(usb.default_port(), usb.join_params(["mv", oldname, value]), None, None)
            self.updateData()
        return True

    def deleteFile(self, row):
        usb.send_command(usb.default_port(), usb.join_params(["rm", self._data[row][0]]), None, None)
        self.updateData()        

    def flags(self, index):
        if index.column() != 0:
            return super(QAbstractTableModel, self).flags(index)
        else:
            return Qt.ItemFlag.ItemIsEditable | super(QAbstractTableModel, self).flags(index)


class MainWindow(QMainWindow, Ui_MainWindow):
    def __init__(self, *args, obj=None, **kwargs):
        super().__init__(*args, **kwargs)
        self.setupUi(self)

        self.uploading = False
        self.statusText = ""


        self.setAcceptDrops(False)
        self.model = MyTableModel()
        self.fileTreeView.setModel(self.model)
        self.fileTreeView.setColumnWidth(0, 300)
        #self.fileTreeView.setItemDelegateForColumn(1, ReadOnlyDelegate())

        self.fileTreeView.dragEnterEvent = self.treeDragEnter
        self.fileTreeView.dragMoveEvent = self.treeDragMove
        self.fileTreeView.dropEvent = self.treeDragDrop

        self.btnDelete.clicked.connect(self.deleteFile)
        self.btnRefresh.clicked.connect(self.model.updateData)
        self.btnExit.clicked.connect(self.close)
        self.btnAddFile.clicked.connect(self.addFile)
        self.btnDownload.clicked.connect(self.downloadFile)
        self.model.statsChanged.connect(self.statsChanged)
        self.statsChanged(self.model._stats)

    def addFile(self):
        filename, filter = QFileDialog.getOpenFileName(self, "Add File to Pico")
        if filename is not None:
            self.sendFileToPico(filename)
            self.model.updateData()

    def statsChanged(self, stats : FileSystemStats):
        available = stats.block_size * stats.total_blocks
        used = stats.block_size * stats.used_blocks
        free = available - used
        self.statusText = f"Available: {available // 1024}KB, Used: {used // 1024}KB, Free {free // 1024}KB ({(100 * free) // available}%)"

        if not self.uploading:
            self.lblStats.setText(self.statusText)

    def downloadFile(self):
        selected = self.fileTreeView.selectedIndexes()
        if len(selected) > 0:
            remote_file = self.model.data(selected[0], Qt.ItemDataRole.DisplayRole)
            filename, filter = QFileDialog.getSaveFileName(self, "Download file to", remote_file)

            try:
                cmd = usb.join_params(
                    ["download", remote_file]
                )
                usb.send_command(usb.default_port(), cmd, None, filename)
                
            

            except Exception as ex:
                msg_box = QMessageBox(title=str(ex))
                msg_box.exec()



    def dragEnterEvent(self, event: QDragEnterEvent) -> None:#
        event.setAccepted(False)
        # event.setDropAction(Qt.DropAction.IgnoreAction)
        # event.ignore()

    def dragMoveEvent(self, event: QDragMoveEvent) -> None:
        event.ignore()

    def deleteFile(self):
        selected = self.fileTreeView.selectedIndexes()
        if len(selected) > 0:
            self.model.deleteFile(selected[0].row())

    def treeDragEnter(self, e):
        if e.mimeData().hasUrls:
            e.accept()
        else:
            e.ignore()

    def treeDragMove(self, e : QDragMoveEvent):
        if e.mimeData().hasUrls:
            e.accept()
        else:
            e.ignore()


    def sendFileToPico(self, filename):
        try:
            path = Path(filename)
            shortened_name = strip_guff(path.stem)
            if len(shortened_name) > 26:
                shortened_name = shortened_name[0:26]
            if path.suffix.upper() == ".Z80":
                snapshot = Z80Snapshot()
                snapshot.process_file(filename)
                upload_data = snapshot.to_bytes()
            else:
                upload_data = path.read_bytes()

            if len(path.suffix ) == 5 and path.suffix[-1] == 'z':
                cmd = usb.join_params(
                    ["upload", shortened_name + path.suffix]
                )
                usb.send_command(usb.default_port(), cmd, upload_data, None)
            else:
                cmd = usb.join_params(
                    ["upload", shortened_name + path.suffix + "z"]
                )
                usb.send_command(usb.default_port(), cmd, usb.compress_data(upload_data), None)
            return True

        except Exception as ex:
            QMessageBox.critical(self, 
                                 "Error copying file to Pico",
                                 str(ex),
                                 QMessageBox.StandardButton.Ok,
                                 QMessageBox.StandardButton.Default)
                                
                            
            return False

    def treeDragDrop(self, e):
        if e.mimeData().hasUrls:
            e.setDropAction(Qt.CopyAction)
            e.accept()

            self.uploading = True

            for url in e.mimeData().urls():
                            
                self.lblStats.setText(
                    f"Uploading {Path(url.toLocalFile()).name}"
                )
                self.repaint()
                if self.sendFileToPico(url.toLocalFile()):
                    self.model.updateData()
                else:
                    break

            self.uploading = False
            self.lblStats.setText(self.statusText)

        else:
            e.ignore()            





app = QApplication(sys.argv)

window = MainWindow()
window.show()
app.exec()