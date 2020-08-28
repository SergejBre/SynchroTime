//------------------------------------------------------------------------------
//  Home Office
//  Nürnberg, Germany
//  E-Mail: sergej1@email.ua
//
//  Copyright (C) 2020 free Project SynchroTime. All rights reserved.
//------------------------------------------------------------------------------
//  Project SynchroTime: Time synchronization of the Precision RTC module DS3231
//  with the UTC System Time via the Serial Interface (UART).
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

//------------------------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------------------------
void setCommandLineParser( QCommandLineParser &parser );
int handleInformationRequest( Session *const session );
int handleAdjustmentRequest( Session *const session );
int handleCalibrationRequest( Session *const session );
int handleResetRequest( Session *const session );
int handleSetRegisterRequest( Session *const session );

#endif // HELPER_H
