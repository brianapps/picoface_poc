# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'filemanager.ui'
##
## Created by: Qt User Interface Compiler version 6.7.2
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide6.QtCore import (QCoreApplication, QDate, QDateTime, QLocale,
    QMetaObject, QObject, QPoint, QRect,
    QSize, QTime, QUrl, Qt)
from PySide6.QtGui import (QBrush, QColor, QConicalGradient, QCursor,
    QFont, QFontDatabase, QGradient, QIcon,
    QImage, QKeySequence, QLinearGradient, QPainter,
    QPalette, QPixmap, QRadialGradient, QTransform)
from PySide6.QtWidgets import (QAbstractItemView, QApplication, QHBoxLayout, QHeaderView,
    QLabel, QMainWindow, QPushButton, QSizePolicy,
    QSpacerItem, QTreeView, QVBoxLayout, QWidget)

class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        if not MainWindow.objectName():
            MainWindow.setObjectName(u"MainWindow")
        MainWindow.resize(800, 600)
        self.centralwidget = QWidget(MainWindow)
        self.centralwidget.setObjectName(u"centralwidget")
        self.horizontalLayout = QHBoxLayout(self.centralwidget)
        self.horizontalLayout.setObjectName(u"horizontalLayout")
        self.verticalLayout_2 = QVBoxLayout()
        self.verticalLayout_2.setObjectName(u"verticalLayout_2")
        self.fileTreeView = QTreeView(self.centralwidget)
        self.fileTreeView.setObjectName(u"fileTreeView")
        self.fileTreeView.setDragDropMode(QAbstractItemView.DropOnly)
        self.fileTreeView.setIndentation(0)

        self.verticalLayout_2.addWidget(self.fileTreeView)

        self.lblStats = QLabel(self.centralwidget)
        self.lblStats.setObjectName(u"lblStats")

        self.verticalLayout_2.addWidget(self.lblStats)

        self.horizontalLayout_2 = QHBoxLayout()
        self.horizontalLayout_2.setObjectName(u"horizontalLayout_2")

        self.verticalLayout_2.addLayout(self.horizontalLayout_2)


        self.horizontalLayout.addLayout(self.verticalLayout_2)

        self.verticalLayout = QVBoxLayout()
        self.verticalLayout.setObjectName(u"verticalLayout")
        self.btnAddFile = QPushButton(self.centralwidget)
        self.btnAddFile.setObjectName(u"btnAddFile")

        self.verticalLayout.addWidget(self.btnAddFile)

        self.btnDownload = QPushButton(self.centralwidget)
        self.btnDownload.setObjectName(u"btnDownload")

        self.verticalLayout.addWidget(self.btnDownload)

        self.btnDelete = QPushButton(self.centralwidget)
        self.btnDelete.setObjectName(u"btnDelete")

        self.verticalLayout.addWidget(self.btnDelete)

        self.btnRefresh = QPushButton(self.centralwidget)
        self.btnRefresh.setObjectName(u"btnRefresh")

        self.verticalLayout.addWidget(self.btnRefresh)

        self.btnExit = QPushButton(self.centralwidget)
        self.btnExit.setObjectName(u"btnExit")

        self.verticalLayout.addWidget(self.btnExit)

        self.verticalSpacer = QSpacerItem(20, 40, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Expanding)

        self.verticalLayout.addItem(self.verticalSpacer)


        self.horizontalLayout.addLayout(self.verticalLayout)

        MainWindow.setCentralWidget(self.centralwidget)

        self.retranslateUi(MainWindow)

        QMetaObject.connectSlotsByName(MainWindow)
    # setupUi

    def retranslateUi(self, MainWindow):
        MainWindow.setWindowTitle(QCoreApplication.translate("MainWindow", u"picoFace File Manager", None))
        self.lblStats.setText(QCoreApplication.translate("MainWindow", u"Available: 0KB, Used: 0KB, Free: 0KB (100%)", None))
        self.btnAddFile.setText(QCoreApplication.translate("MainWindow", u"Add File...", None))
        self.btnDownload.setText(QCoreApplication.translate("MainWindow", u"Download File...", None))
        self.btnDelete.setText(QCoreApplication.translate("MainWindow", u"Delete File", None))
#if QT_CONFIG(shortcut)
        self.btnDelete.setShortcut(QCoreApplication.translate("MainWindow", u"Del", None))
#endif // QT_CONFIG(shortcut)
        self.btnRefresh.setText(QCoreApplication.translate("MainWindow", u"Refresh", None))
#if QT_CONFIG(shortcut)
        self.btnRefresh.setShortcut(QCoreApplication.translate("MainWindow", u"F9", None))
#endif // QT_CONFIG(shortcut)
        self.btnExit.setText(QCoreApplication.translate("MainWindow", u"Exit", None))
    # retranslateUi

