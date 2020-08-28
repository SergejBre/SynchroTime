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

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <QCommandLineOption>
#include <QLoggingCategory>
#include <QDebug>
#include <QDateTime>
#include "helper.h"

//------------------------------------------------------------------------------
// Preprocessor
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Enums
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
QT_USE_NAMESPACE
Q_LOGGING_CATEGORY(logHelper, "Helper")
// Output stream for all messages (not for error!)
static QTextStream standardOutput( stdout );

//------------------------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------------------------

// ------------------------------------------------------------------------
//! \brief
//! Set Command Line Parser's parameters and optins
//!
//! \details
//! \todo
//! \param[in,out] parser of the type QCommandLineParser
//!
// ------------------------------------------------------------------------
void setCommandLineParser( QCommandLineParser &parser )
{
#ifndef GUI_APP
    parser.setApplicationDescription( "Description: Console app is used for adjustment of the RTC DS3231 module.\n"
                                      "The console app can synchronize the time with a computer via Serial Port and \n"
                                      "compensate for the time-drift of the DS3231 and save the parameters into flash." );
#endif

    parser.addPositionalArgument( "value", QCoreApplication::translate( "main", "New value for offset reg. (from -12.8 to 12.7)" ) );

#ifndef GUI_APP
    QCommandLineOption discovery( QStringList() << DISCOVERY << "discovery",
                                       QCoreApplication::translate( "main", "Discover for existing Serial Ports in the System" ),
                                       QCoreApplication::translate( "main", "" ), "0" );
    parser.addOption( discovery );
#endif

    QCommandLineOption portName( QStringList() << PORT << "port",
                                       QCoreApplication::translate( "main", "Set up an available Serial Port (ttyUSBx or COMx)" ),
                                       QCoreApplication::translate( "main", "PortName" ), "1" );
    parser.addOption( portName );

    QCommandLineOption information( QStringList() << INFO << "info",
                                       QCoreApplication::translate( "main", "Read information about available RTC module" ),
                                       QCoreApplication::translate( "main", "" ), "0" );
    parser.addOption( information );

    QCommandLineOption adjustment( QStringList() << ADJUST << "adjust",
                                       QCoreApplication::translate( "main", "Adjust time from the computer" ),
                                       QCoreApplication::translate( "main", "" ), "0" );
    parser.addOption( adjustment );

#ifndef GUI_APP
    QCommandLineOption calibration( QStringList() << CALIBR << "calibration",
                                       QCoreApplication::translate( "main", "Calibrate the clock of the DS3231 module" ),
                                       QCoreApplication::translate( "main", "" ), "0" );
    parser.addOption( calibration );
#endif

    QCommandLineOption reset( QStringList() << RESET << "reset",
                                       QCoreApplication::translate( "main", "Reset the offset register to its default value" ),
                                       QCoreApplication::translate( "main", "" ), "0" );
    parser.addOption( reset );

    QCommandLineOption setregister( QStringList() << SETREG << "setreg",
                                       QCoreApplication::translate( "main", "Set a new value in the offset register" ),
                                       QCoreApplication::translate( "main", "value"), "1" );
    parser.addOption( setregister );

#ifndef GUI_APP
    parser.addVersionOption();
    parser.addHelpOption();
#endif
}

// ------------------------------------------------------------------------
//! \brief
//! Request for the Reset of the device.
//!
//! \details
//!
//! \param[in] parent Pointer to the parent object used for QObject.
// ------------------------------------------------------------------------
int handleResetRequest( Session *const session )
{
    Q_ASSERT( session != nullptr );

    // Open the interface for communication with the device
    if ( !session->getInterface()->openSocket() )
    {
        qCritical( logHelper ) << "Request for return of the Reset failed.";
        return 1;
    }

    // Request for Reset
    QByteArray requestForReset("@r");

    // Send a command to the device
    session->getInterface()->writeTheData( requestForReset );
    qDebug() << "Send command: " << requestForReset;

    session->getInterface()->readTheData( TIME_WAIT );
    qDebug() << "Received bytes: " << session->getInterface()->getReceivedData().size();

    session->getInterface()->closeSocket();

    // Check of the Received request
    if ( !session->getInterface()->getReceivedData().isEmpty() )
    {
        standardOutput << "Request for reset " << session->getInterface()->getReceivedData() << endl;
    }
    else
    {
        qCritical( logHelper ) << "Request for reset failed.";
        return 1;
    }
    return 0;
}

// ------------------------------------------------------------------------
//! \brief
//! Request for the Information of the device.
//!
//! \details
//!
//! \param[in] parent Pointer to the parent object used for QObject.
//!
int handleInformationRequest(Session * const session)
{
    Q_ASSERT( session != nullptr );

    // Open the interface for communication with the device
    if ( !session->getInterface()->openSocket() )
    {
        qCritical( logHelper ) << "Request for return of the Information failed.";
        return 1;
    }

    // Request for Information
    QByteArray requestForVersion("@ittttmm");

    QDateTime local(QDateTime::currentDateTime());
    qint64 localTimeMSecs = local.toMSecsSinceEpoch();
    quint32 localTimeSecs = localTimeMSecs/1000;
    quint16 milliSecs = localTimeMSecs - localTimeSecs * 1000;
    memcpy( requestForVersion.data() + 2, &localTimeSecs, sizeof(localTimeSecs) );
    memcpy( requestForVersion.data() + 6, &milliSecs, sizeof(milliSecs) );

    // Send a command to the device
    session->getInterface()->writeTheData( requestForVersion );
    qDebug() << "Send command: " << requestForVersion;

    session->getInterface()->readTheData( TIME_WAIT );
    qDebug() << "Received bytes: " << session->getInterface()->getReceivedData().size();

    session->getInterface()->closeSocket();

    // Check of the Received request
    if ( !session->getInterface()->getReceivedData().isEmpty() && session->getInterface()->getReceivedData().count() >= 8 )
    {
        qint64 numberOfMSec = 0LL;
        quint16 numberOfSec = 0U;
        memcpy( &numberOfMSec, session->getInterface()->getReceivedData().data(), 4 );
        numberOfMSec *= 1000;
        memcpy( &numberOfSec, session->getInterface()->getReceivedData().data() + 4, sizeof(numberOfSec) );
        numberOfMSec += numberOfSec;
        QDateTime time( QDateTime::fromMSecsSinceEpoch( numberOfMSec ) );
        standardOutput << "DS3231 clock time\t" << numberOfMSec << "ms: " << time.toString("d.MM.yyyy hh:mm:ss.zzz") << endl;
        standardOutput << "System local time\t" << localTimeMSecs << "ms: " << local.toString("d.MM.yyyy hh:mm:ss.zzz") << endl;
        standardOutput << "Difference between\t" << numberOfMSec - localTimeMSecs << "ms" << endl;
        if ( session->getInterface()->getReceivedData().count() >= 6 ) {
            qint8 offset_reg = session->getInterface()->getReceivedData().at(6);
            standardOutput << "Offset reg. value\t" << offset_reg << endl;
            if ( session->getInterface()->getReceivedData().count() >= 10 ) {
                float drift_in_ppm = 0;
                memcpy( &drift_in_ppm, session->getInterface()->getReceivedData().data() + 7, sizeof(drift_in_ppm) );
                standardOutput << "Time drift in ppm\t" << drift_in_ppm << "ppm" << endl;
            }
        }
    }
    else
    {
        qCritical( logHelper ) << "Request for return of the Information failed.";
        return 1;
    }
    return 0;
}

// ------------------------------------------------------------------------
//! \brief
//! Request for the Adjustment of the device.
//!
//! \details
//!
//! \param[in] parent Pointer to the parent object used for QObject.
//!
int handleAdjustmentRequest( Session * const session )
{
    Q_ASSERT( session != nullptr );

    // Open the interface for communication with the device
    if ( !session->getInterface()->openSocket() )
    {
        qCritical( logHelper ) << "Request for return of the Adjustment failed.";
        return 1;
    }

    // Request for Adjustment
    QByteArray requestForAdjustment("@attttmm");

    QDateTime local(QDateTime::currentDateTime());
    qint64 localTimeMSecs = local.toMSecsSinceEpoch();
    quint32 localTimeSecs = localTimeMSecs/1000;
    quint16 milliSecs = localTimeMSecs - localTimeSecs * 1000;
    localTimeSecs++;
    memcpy( requestForAdjustment.data() + 2, &localTimeSecs, sizeof(localTimeSecs) );
    memcpy( requestForAdjustment.data() + 6, &milliSecs, sizeof(milliSecs) );

    milliSecs = 1000 - milliSecs;
    QTime time;
    time.start();
    while ( time.elapsed() < milliSecs );

    // Send a command to the device
    session->getInterface()->writeTheData( requestForAdjustment );
    qDebug() << "Send command: " << requestForAdjustment;

    session->getInterface()->readTheData( TIME_WAIT );
    qDebug() << "Received bytes: " << session->getInterface()->getReceivedData().size();

    session->getInterface()->closeSocket();

    // Check of the Received request
    if ( !session->getInterface()->getReceivedData().isEmpty() )
    {
        standardOutput << "System local time " << localTimeMSecs << "ms: " << local.toString() << endl;
        standardOutput << "Request for adjustment " << session->getInterface()->getReceivedData() << endl;
    }
    else
    {
        qCritical( logHelper ) << "Request for adjustment failed.";
        return 1;
    }
    return 0;
}

// ------------------------------------------------------------------------
//! \brief
//! Request for the Calibration of the device.
//!
//! \details
//!
//! \param[in] parent Pointer to the parent object used for QObject.
//!
int handleCalibrationRequest( Session * const session )
{
    Q_ASSERT( session != nullptr );

    // Open the interface for communication with the device
    if ( !session->getInterface()->openSocket() )
    {
        qCritical( logHelper ) << "Request for return of the Calibration failed.";
        return 1;
    }

    // Request for Calibration
    QByteArray requestForCalibration("@c");

    // Send a command to the device
    session->getInterface()->writeTheData( requestForCalibration );
    qDebug() << "Send command: " << requestForCalibration;

    session->getInterface()->readTheData( TIME_WAIT );
    qDebug() << "Received bytes: " << session->getInterface()->getReceivedData().size();

    session->getInterface()->closeSocket();

    // Check of the Received request
    if ( !session->getInterface()->getReceivedData().isEmpty() )
    {
        standardOutput << "Request for calibration " << session->getInterface()->getReceivedData() << endl;
    }
    else
    {
        qCritical( logHelper ) << "Request for calibration failed.";
        return 1;
    }
    return 0;
}

// ------------------------------------------------------------------------
//! \brief
//! Request to update the register value.
//!
//! \details
//!
//! \param[in] parent Pointer to the parent object used for QObject.
//!
int handleSetRegisterRequest(Session * const session)
{
    Q_ASSERT( session != nullptr );

    // Open the interface for communication with the device
    if ( !session->getInterface()->openSocket() )
    {
        qCritical( logHelper ) << "Request to update the offset register value failed.";
        return 1;
    }

    return 0;
}
