//------------------------------------------------------------------------------
//  Home Office
//  Nürnberg, Germany
//  E-Mail: sergej1@email.ua
//
//  Copyright (C) 2020 free Project SynchroTime. All rights reserved.
//------------------------------------------------------------------------------
//  Project SynchroTime: Command-line client for adjust the exact time and
//  calibrating the RTC DS3231 module via the serial interface (UART).
//------------------------------------------------------------------------------
#ifndef HELPER_H
#define HELPER_H

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include "session.h"
#include <QCommandLineParser>

//------------------------------------------------------------------------------
// Preprocessor
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Enums
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
// Interval for a time-out in milliseconds
#define TIME_WAIT 10U
// optins for the Command line parser
#ifndef GUI_APP
#define DISCOVERY  "d"
#endif
#define PORT       "p"
#define INFO       "i"
#define ADJUST     "a"
#define CALIBR     "c"
#define RESET      "r"
#define SETREG     "s"
// number of expected bytes
#define RECEIVED_BYTES 1U
#define RECEIVED_BYTES_INFO 15U
#define RECEIVED_BYTES_CALIBR 7U

//------------------------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------------------------
void setCommandLineParser( QCommandLineParser &parser );
int handleInformationRequest( Session *const session );
int handleAdjustmentRequest( Session *const session );
int handleCalibrationRequest( Session *const session );
int handleResetRequest( Session *const session );
int handleSetRegisterRequest( Session * const session, const float value );

#endif // HELPER_H
