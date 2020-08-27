#------------------------------------------------------------------------------
#  Home Office
#  Nürnberg, Germany
#  E-Mail: sergej1@email.ua
#
#  Copyright (C) 2020 free Project SynchroTime RTC DS3231. All rights reserved.
#------------------------------------------------------------------------------
#  Project SynchroTime: Time synchronization of the Precision RTC module DS3231
#  with the system time via the serial interface (UART).
#
#------------------------------------------------------------------------------
QT += core serialport
QT -= gui

DESTDIR = ../build
MOC_DIR = ../moc
CONFIG += console
CONFIG += c++11
CONFIG += debug_and_release
CONFIG -= app_bundle

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

# For Win32 release
win32 {
    VERSION = 1.0.0.0
    QMAKE_TARGET_COMPANY = Free Project
    QMAKE_TARGET_PRODUCT = SynchroTime
    QMAKE_TARGET_DESCRIPTION = Time synchronization for RTC DS3231
    QMAKE_TARGET_COPYRIGHT = (c) 2020 sergej1@email.ua
    RC_ICONS = images/icon.ico
}
