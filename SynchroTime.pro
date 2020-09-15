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
TEMPLATE = subdirs

SUBDIRS += \
    SynchroTimeCon \
    SynchroTimeApp \
    tests

SynchroTimeCon.file = SynchroTimeCon/SynchroTimeCon.pro
SynchroTimeApp.file = SynchroTimeApp/SynchroTimeApp.pro
tests.file = tests/tests.pro

CONFIG += ordered
