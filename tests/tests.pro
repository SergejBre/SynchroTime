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
QT += serialport testlib
QT -= gui

TARGET = test
DESTDIR = ../build
MOC_DIR = ../moc
OBJECTS_DIR = obj
CONFIG += console
CONFIG -= app_bundle
greaterThan(QT_MAJOR_VERSION, 4): CONFIG += c++11
lessThan(QT_MAJOR_VERSION, 5): QMAKE_CXXFLAGS += -std=c++11

TEMPLATE = app
LANGUAGE = C++
# Define for the console application
DEFINES += CONSOLE_APP

INCLUDEPATH += ../include

SOURCES += tests.cpp \
    ../src/interface.cpp \
    ../src/base.cpp

HEADERS += \
    ../include/interface.h \
#    ../include/session.h \
    ../include/base.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"
