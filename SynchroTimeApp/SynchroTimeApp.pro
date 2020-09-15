#------------------------------------------------------------------------------
#  Home Office
#  Nürnberg, Germany
#  E-Mail: sergej1@email.ua
#
#  Copyright (C) 2020 free Project SynchroTime. All rights reserved.
#------------------------------------------------------------------------------
#  Project SynchroTime: Command-line client for adjust the exact time and
#  calibrating the RTC DS3231 module via the serial interface (UART).
#------------------------------------------------------------------------------
QT += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DESTDIR = ../build
MOC_DIR = ../moc
CONFIG += c++11
CONFIG += debug_and_release

CONFIG(debug, debug|release) {
    TARGET = synchroTimeAppd
    CONFIG += debug
} else {
    TARGET = synchroTimeApp
    CONFIG += release
    DEFINES += QT_NO_DEBUG_OUTPUT
}

# Define for the GUI application
DEFINES += GUI_APP
TEMPLATE = app

INCLUDEPATH += ../include

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
