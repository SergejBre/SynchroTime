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

//! \brief
//! This structure will hold context information for CMD command string
//!
typedef struct
{
    //! storage command request: c, i, v, r, s
    char command;
    //! number of the storage device
    int number;
    //! task for operation with the storage device: f, r, e, w
    char task;
    //! start of the block
    quint32 addressStart;
    //! end of the block
    quint32 addressEnd;
    //! status of the parser
    int status;

} CMDcommand;

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
bool commandLineParser( CMDcommand *const cmdCom, const QString &str );
inline bool parserHelper( CMDcommand *const cmdCom, const QString &str );
int handleInformationRequest( Session *const session );
int handleAdjustmentRequest( Session *const session );
int handleCalibrationRequest( Session *const session );
int handleResetRequest( Session *const session );
int handleVersionRequest( Session *const session );
/*
bool validateCommandLine( QCommandLineParser &parser, const QString &commandline, IOHandler *const ioHandler, const int line );
int executeCommandLine( QCommandLineParser &parser, const QString &commandline, IOHandler *const ioHandler, Session *const session, const QUrl &configFile, const int line );
int handleConfiguration( IOHandler *const ioHandler, Session *const session, const QUrl &configFile, const QUrl &urlBinaryFile );
int handleComparation( IOHandler *const ioHandler, const QUrl &urlFile1, const QUrl &urlFile2 );
int handleConfigurationRequest( IOHandler *const ioHandler, Session *const session, const QUrl &configFile, const QUrl &urlBinaryFile );
int handleInformationRequest( IOHandler *const ioHandler, Session *const session, const QUrl &outputFile );
int checkStatus( QByteArray *data );
QStringList discoverySerialPorts( void );
*/
#endif // HELPER_H
