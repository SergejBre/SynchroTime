QT += network testlib
QT -= gui

TARGET = test
DESTDIR = ../build
MOC_DIR = ../moc
CONFIG += console
CONFIG += c++11
CONFIG -= app_bundle

TEMPLATE = app
# Define for the console application
DEFINES += CONSOLE_APP

INCLUDEPATH += ../include

SOURCES += tests.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"
