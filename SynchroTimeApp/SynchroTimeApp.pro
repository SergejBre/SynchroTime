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

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

DESTDIR = ../build
MOC_DIR = ../moc
UI_DIR = ../ui
CONFIG += debug_and_release
greaterThan(QT_MAJOR_VERSION, 4): CONFIG += c++11
lessThan(QT_MAJOR_VERSION, 5): QMAKE_CXXFLAGS += -std=c++11

CONFIG(debug, debug|release) {
    TARGET = synchroTimeAppd
    CONFIG += debug
} else {
    TARGET = synchroTimeApp
    CONFIG += release
    DEFINES += QT_NO_DEBUG_OUTPUT
}

# Define for the GUI application
DEFINES += GUI_APP #QT_NO_TRANSLATION
TEMPLATE = app

INCLUDEPATH += ../include

SOURCES += main.cpp\
    mainwindow.cpp \
    rtc.cpp \
    console.cpp \
    settingsdialog.cpp \
    ../../qcustomplot/qcustomplot.cpp

HEADERS  += mainwindow.h \
    rtc.h \
    console.h \
    settingsdialog.h \
    serialportsettings.h \
    ../../qcustomplot/qcustomplot.h

FORMS    += mainwindow.ui \
    settingsdialog.ui

TRANSLATIONS += \
    ../translations/synchroTimeApp_de_DE.ts \
    ../translations/synchroTimeApp_ru_RU.ts

EXTRA_TRANSLATIONS += ../translations/synchroTimeApp_en.ts

RESOURCES += \
    synchrotime.qrc \
    qm_files.qrc

CONFIG += lrelease #embed_translations

# for static-build ->
QTPLUGIN += qminimal
linux|macx: equals(QT_MAJOR_VERSION, 5): greaterThan(QT_MINOR_VERSION, 7): QTPLUGIN += qxcb
win32:      QTPLUGIN += qwindows
CONFIG -= import_plugins

linux|macx {
# for dynamic build uncomment ->
#    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/lib\'"
}
win32 {
    VERSION = 2.0.0
    QMAKE_TARGET_COMPANY = Free Project
    QMAKE_TARGET_PRODUCT = SynchroTime
    QMAKE_TARGET_DESCRIPTION = Time adjust and calibration for RTC DS3231
    QMAKE_TARGET_COPYRIGHT = (c) 2022 sergej1@email.ua
    RC_ICONS = ../images/icon.ico
}

DISTFILES +=
