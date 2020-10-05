QT += serialport testlib
QT -= gui

TARGET = test
DESTDIR = ../build
MOC_DIR = ../moc
CONFIG += console
CONFIG += qt c++11
CONFIG -= app_bundle

TEMPLATE = app
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
