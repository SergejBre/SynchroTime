#------------------------------------------------------------------------------
#  Home Office
#  Nürnberg, Germany
#  E-Mail: sergej1@email.ua
#
#  Copyright (C) 2020 free Project SynchroTime. All rights reserved.
#------------------------------------------------------------------------------
#  Project SynchroTime: Time synchronization via Serial Port (UART)
#  with a DS3231 Precision RTC module.
#
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
