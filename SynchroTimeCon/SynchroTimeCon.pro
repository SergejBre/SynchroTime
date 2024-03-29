#------------------------------------------------------------------------------
#  Home Office
#  Nürnberg, Germany
#  E-Mail: sergej1@email.ua
#
#  Copyright (C) 2020 free Project SynchroTime RTC DS3231. All rights reserved.
#------------------------------------------------------------------------------
#  Project SynchroTime: Command-line client for adjust the exact time and
#  calibrating the RTC DS3231 module via the serial interface (UART).
#------------------------------------------------------------------------------
QT += core serialport
QT -= gui

DESTDIR = ../build
MOC_DIR = ../moc
CONFIG += console
CONFIG += debug_and_release
CONFIG -= app_bundle
greaterThan(QT_MAJOR_VERSION, 4): CONFIG += c++11
lessThan(QT_MAJOR_VERSION, 5): QMAKE_CXXFLAGS += -std=c++11

CONFIG(debug, debug|release) {
    TARGET = synchroTimed
    CONFIG += debug
} else {
    TARGET = synchroTime
    CONFIG += release
    DEFINES += QT_NO_DEBUG_OUTPUT
}
TEMPLATE = app
# Define for the console application
DEFINES += CONSOLE_APP

INCLUDEPATH += ../include

SOURCES += main.cpp \
    ../src/base.cpp \
    ../src/interface.cpp \
    ../src/session.cpp \
    helper.cpp \
    settings.cpp

HEADERS += \
    ../include/base.h \
    ../include/interface.h \
    ../include/session.h \
    helper.h \
    settings.h

# for static-build ->
linux|macx {
# for dynamic build uncomment ->
#    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/lib\'"
}
win32 {
    VERSION = 1.1.0.0
    QMAKE_TARGET_COMPANY = Free Project
    QMAKE_TARGET_PRODUCT = SynchroTime
    QMAKE_TARGET_DESCRIPTION = Time adjust and calibration for RTC DS3231
    QMAKE_TARGET_COPYRIGHT = (c) 2020 sergej1@email.ua
    RC_ICONS = ../images/icon.ico
}
