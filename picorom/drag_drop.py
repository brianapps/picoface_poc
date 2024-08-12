

from PySide6 import QtGui, QtCore, QtWidgets
import sys
from convertZ80 import Z80Snapshot
import usb



class MainWindowWidget(QtWidgets.QWidget):
    def __init__(self):
        super(MainWindowWidget, self).__init__()

        # Button that allows loading of images
        self.load_button = QtWidgets.QPushButton("Convert snapshot")
        self.load_button.clicked.connect(self.load_image_but)

        # Image viewing region
        self.lbl = QtWidgets.QLabel(self)
        self.lbl.setText("")

        # A horizontal layout to include the button on the left
        # layout_button = QtWidgets.QHBoxLayout()
        # layout_button.addWidget(self.load_button)
        # layout_button.addStretch()

        # A Vertical layout to include the button layout and then the image
        layout = QtWidgets.QVBoxLayout()
        layout.addWidget(self.load_button)
        layout.addWidget(self.lbl)

        self.setLayout(layout)

        # Enable dragging and dropping onto the GUI
        self.setAcceptDrops(True)

        self.show()

    def load_image_but(self):
        #Get the file location
        
        self.fname, _ = QtWidgets.QFileDialog.getOpenFileName(self, 'Open file')
        # Load the image from the location

    # The following three methods set up dragging and dropping for the app
    def dragEnterEvent(self, e):
        if e.mimeData().hasUrls:
            e.accept()
        else:
            e.ignore()

    def dragMoveEvent(self, e):
        if e.mimeData().hasUrls:
            e.accept()
        else:
            e.ignore()

    def dropEvent(self, e):
        if e.mimeData().hasUrls:
            e.setDropAction(QtCore.Qt.CopyAction)
            e.accept()
            for url in e.mimeData().urls():
                fname = str(url.toLocalFile())

            self.fname = fname
            try:

                snapshot = Z80Snapshot()
                snapshot.process_file(self.fname)

                usb.send_command('/dev/ttyACM0', "snapupload", snapshot.to_bytes(), None)
                self.lbl.setText("Sent")
            except Exception as ex:
                self.lbl.setText(str(ex))

            
            
        else:
            e.ignore()

# Run if called directly
if __name__ == '__main__':
    # Initialise the application
    app = QtWidgets.QApplication(sys.argv)
    # Call the widget
    ex = MainWindowWidget()
    sys.exit(app.exec())