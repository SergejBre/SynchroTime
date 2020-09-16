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
//!
//! \param[in,out] parser of the type QCommandLineParser
// ------------------------------------------------------------------------
void setCommandLineParser( QCommandLineParser &parser )
{
#ifndef GUI_APP
    parser.setApplicationDescription( "Description: Console app is used for adjustment of the RTC DS3231 module.\n"
                                      "The app can adjust the time using a computer via the serial port, adjust\n"
                                      "clock drift and save parameters and calibration data to flash memory." );
#endif

    parser.addPositionalArgument( "<PortName>", QCoreApplication::translate( "main", "Name of serial port. Platform dependent ttyUSB or COM" ) );
    parser.addPositionalArgument( "<Value>", QCoreApplication::translate( "main", "New value for offset reg. (from -12.8 to 12.7)" ) );

#ifndef GUI_APP
    QCommandLineOption discovery( QStringList() << DISCOVERY << "discovery",
                                       QCoreApplication::translate( "main", "Detects for existing serial ports in the system." ),
                                       QCoreApplication::translate( "main", "" ), "0" );
    parser.addOption( discovery );
#endif

    QCommandLineOption portName( QStringList() << PORT << "port",
                                       QCoreApplication::translate( "main", "Sets an available serial interface, e.g. ttyUSB0." ),
                                       QCoreApplication::translate( "main", "PortName" ), "1" );
    parser.addOption( portName );

    QCommandLineOption information( QStringList() << INFO << "info",
                                       QCoreApplication::translate( "main", "Reads information about the available RTC module." ),
                                       QCoreApplication::translate( "main", "" ), "0" );
    parser.addOption( information );

    QCommandLineOption adjustment( QStringList() << ADJUST << "adjust",
                                       QCoreApplication::translate( "main", "Adjusts the time from the computer." ),
                                       QCoreApplication::translate( "main", "" ), "0" );
    parser.addOption( adjustment );

#ifndef GUI_APP
    QCommandLineOption calibration( QStringList() << CALIBR << "calibration",
                                       QCoreApplication::translate( "main", "Calibrates the clock of the DS3231 module." ),
                                       QCoreApplication::translate( "main", "" ), "0" );
    parser.addOption( calibration );
#endif

    QCommandLineOption reset( QStringList() << RESET << "reset",
                                       QCoreApplication::translate( "main", "Resets the offset register to its default value." ),
                                       QCoreApplication::translate( "main", "" ), "0" );
    parser.addOption( reset );

    QCommandLineOption setregister( QStringList() << SETREG << "setreg",
                                       QCoreApplication::translate( "main", "Sets a new value in the offset register of DS3231." ),
                                       QCoreApplication::translate( "main", "Value"), "1" );
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
//! The function creates a connection to the device and sends a request for
//! Reset using the Reset protocol of the form: {\@r}, 2 bytes long.
//!
//! \param[in] session Pointer to the current session.
//!
//! \retval 0
//! \retval 1
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
    const QByteArray requestForReset("@r");

    // Send a command to the device
    session->getInterface()->writeTheData( requestForReset );
    qDebug() << "Send command: " << requestForReset;

    session->getInterface()->readTheData( TIME_WAIT, RECEIVED_BYTES );
    const uint8_t blength = session->getInterface()->getReceivedData().size();
    qDebug() << "Received bytes: " << blength;

    session->getInterface()->closeSocket();

    // Check of the Received request
    if ( !session->getInterface()->getReceivedData().isEmpty() )
    {
        const uint8_t ret_value = session->getInterface()->getReceivedData().at( blength-1 );
        standardOutput << "Request for reset " << ( ret_value ? "completed successfully." : "fail." ) << endl;
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
//! The function creates a connection to the device and sends a request for
//! Information using the Information protocol of the form: {\@i|time|ms}, 2+4+2 bytes long.
//!
//! \param[in] session Pointer to the current session.
//!
//! \retval 0 if no error occurs,
//! \retval 1 terminate with an error.
// ------------------------------------------------------------------------
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
    QByteArray requestForInformation("@issssmm");

    const QDateTime local(QDateTime::currentDateTime());
    const qint64 localTimeMSecs = local.toMSecsSinceEpoch();
    const quint32 localTimeSecs = localTimeMSecs/1000;
    const quint16 milliSecs = localTimeMSecs - localTimeSecs * 1000;
    memcpy( requestForInformation.data() + 2, &localTimeSecs, sizeof(localTimeSecs) );
    memcpy( requestForInformation.data() + 6, &milliSecs, sizeof(milliSecs) );

    // Send a command to the device
    session->getInterface()->writeTheData( requestForInformation );
    qDebug() << "Send command: " << requestForInformation;

    session->getInterface()->readTheData( TIME_WAIT, RECEIVED_BYTES_INFO );
    const uint8_t blength = session->getInterface()->getReceivedData().size();
    qDebug() << "Received bytes: " << blength;

    session->getInterface()->closeSocket();

    // Check of the Received request
    if ( !session->getInterface()->getReceivedData().isEmpty() && blength > 5 )
    {
        qint64 numberOfMSec = 0LL;
        quint16 numberOfSec = 0U;
        auto byteBuffer = session->getInterface()->getReceivedData().constData();
        memcpy( &numberOfMSec, byteBuffer, sizeof( quint32 ) );
        numberOfMSec *= 1000;
        memcpy( &numberOfSec, byteBuffer + 4, sizeof( numberOfSec ) );
        numberOfMSec += numberOfSec;
        QDateTime time( QDateTime::fromMSecsSinceEpoch( numberOfMSec ) );
        standardOutput << "DS3231 clock time\t" << numberOfMSec << " ms: " << time.toString("dd.MM.yyyy hh:mm:ss.zzz") << endl;
        standardOutput << "System local time\t" << localTimeMSecs << " ms: " << local.toString("dd.MM.yyyy hh:mm:ss.zzz") << endl;
        standardOutput << "Difference between\t" << numberOfMSec - localTimeMSecs << " ms" << endl;
        if ( blength > 6 ) {
            const float offset_reg = float( byteBuffer[6] )/10;
            standardOutput << "Offset reg. in ppm\t" << offset_reg << " ppm" << endl;
            if ( blength > 10 ) {
                float drift_in_ppm = 0;
                memcpy( &drift_in_ppm, byteBuffer + 7, sizeof( drift_in_ppm ) );
                standardOutput << "Time drift in ppm\t" << drift_in_ppm << " ppm" << endl;
                if ( blength > 14 ) {
                    qint64 lastAdjustOfTimeMSec = 0LL;
                    memcpy( &lastAdjustOfTimeMSec, byteBuffer + 11, sizeof( quint32 ) );
                    if ( lastAdjustOfTimeMSec < 0xFFFFFFFF ) {
                        lastAdjustOfTimeMSec *= 1000;
                        time = QDateTime::fromMSecsSinceEpoch( lastAdjustOfTimeMSec );
                        standardOutput << "last adjust of time\t" << lastAdjustOfTimeMSec << " ms: " << time.toString("dd.MM.yyyy hh:mm:ss.zzz") << endl;
//                        standardOutput << "Time drift in ppm\t" << float(numberOfMSec - localTimeMSecs)*1000000/(localTimeMSecs - lastAdjustOfTimeMSec ) << " ppm" << endl;
                    }
                }
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
//! The function creates a connection to the device and sends a request for
//! Adjustment using the Adjustment protocol of the form: {\@a|time|ms}, 2+4+2 bytes long.
//!
//! \param[in] session Pointer to the current session.
//!
//! \retval 0 if no error occurs,
//! \retval 1 terminate with an error.
// ------------------------------------------------------------------------
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
    QByteArray requestForAdjustment("@assssmm");

    QDateTime local(QDateTime::currentDateTime());
    const qint64 localTimeMSecs = local.toMSecsSinceEpoch();
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

    session->getInterface()->readTheData( TIME_WAIT, RECEIVED_BYTES );
    const uint8_t blength = session->getInterface()->getReceivedData().size();
    qDebug() << "Received bytes: " << blength;

    session->getInterface()->closeSocket();

    // Check of the Received request
    if ( !session->getInterface()->getReceivedData().isEmpty() )
    {
        const uint8_t ret_value = session->getInterface()->getReceivedData().at( blength-1 );
        local.setTime_t( localTimeSecs );
        standardOutput << "System local time\t" << local.toString("ddd d MMM yyyy hh:mm:ss.zzz") << endl;
        standardOutput << "Request for adjustment " << ( ret_value ? "completed successfully." : "fail." ) << endl;
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
//! The function creates a connection to the device and sends a request for
//! Calibration using the Calibration protocol of the form: {\@c|time|ms}, 2+4+2 bytes long.
//!
//! \param[in] session Pointer to the current session.
//!
//! \retval 0 if no error occurs,
//! \retval 1 terminate with an error.
// ------------------------------------------------------------------------
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
    QByteArray requestForCalibration("@cssssmm");

    QDateTime local(QDateTime::currentDateTime());
    const qint64 localTimeMSecs = local.toMSecsSinceEpoch();
    quint32 localTimeSecs = localTimeMSecs/1000;
    quint16 milliSecs = localTimeMSecs - localTimeSecs * 1000;
    const quint16 ms = 0U;
    localTimeSecs++;
    memcpy( requestForCalibration.data() + 2, &localTimeSecs, sizeof(localTimeSecs) );
    memcpy( requestForCalibration.data() + 6, &ms, sizeof(ms) );

    milliSecs = 1000 - milliSecs;
    QTime time;
    time.start();
    while ( time.elapsed() < milliSecs );

    // Send a command to the device
    session->getInterface()->writeTheData( requestForCalibration );
    qDebug() << "Send command: " << requestForCalibration;

    session->getInterface()->readTheData( TIME_WAIT, RECEIVED_BYTES_CALIBR );
    const uint8_t blength = session->getInterface()->getReceivedData().size();
    qDebug() << "Received bytes: " << blength;

    session->getInterface()->closeSocket();

    // Check of the Received request
    if ( !session->getInterface()->getReceivedData().isEmpty() )
    {
        auto byteBuffer = session->getInterface()->getReceivedData().constData();
        const uint8_t ret_value = byteBuffer[ blength-1 ];
        local.setTime_t( localTimeSecs );
        standardOutput << "System local time\t" << local.toString("ddd d MMM yyyy hh:mm:ss.zzz") << endl;
        qint8 offset_reg = byteBuffer[0];
        standardOutput << "Offset last value\t" << offset_reg << endl;
        if ( blength > 5 ) {
            float drift_in_ppm = 0;
            memcpy( &drift_in_ppm, byteBuffer + 1, sizeof(drift_in_ppm) );
            standardOutput << "Time drift in ppm\t" << drift_in_ppm << " ppm" << endl;
            offset_reg = byteBuffer[5];
            standardOutput << "Offset new value\t" << offset_reg << endl;
        }
        standardOutput << "Request for calibration " << ( ret_value ? "completed successfully." : "fail." ) << endl;
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
//! The function creates a connection to the device and sends a request for
//! SetRegister using the SetRegister protocol of the form: {\@s|value}, 2+4 bytes long.
//!
//! \param[in] session Pointer to the current session.
//! \param[in] value
//!
//! \retval 0 if no error occurs,
//! \retval 1 terminate with an error.
// ------------------------------------------------------------------------
int handleSetRegisterRequest( Session * const session, const float value )
{
    Q_ASSERT( session != nullptr );

    // Open the interface for communication with the device
    if ( !session->getInterface()->openSocket() )
    {
        qCritical( logHelper ) << "Request to update the offset register value failed.";
        return 1;
    }

    // Request for SetRegister
    QByteArray requestForSetRegister("@sfloat");
    memcpy( requestForSetRegister.data() + 2, &value, sizeof(value) );

    // Send a command to the device
    session->getInterface()->writeTheData( requestForSetRegister );
    qDebug() << "Send command: " << requestForSetRegister;

    session->getInterface()->readTheData( TIME_WAIT, RECEIVED_BYTES );
    const uint8_t blength = session->getInterface()->getReceivedData().size();
    qDebug() << "Received bytes: " << blength;

    session->getInterface()->closeSocket();

    // Check of the Received request
    if ( !session->getInterface()->getReceivedData().isEmpty() )
    {
        const uint8_t ret_value = session->getInterface()->getReceivedData().at( blength-1 );
        standardOutput << "Request for SetRegister " << ( ret_value ? "completed successfully." : "fail." ) << endl;
    }
    else
    {
        qCritical( logHelper ) << "Request for SetRegister failed.";
        return 1;
    }

    return 0;
}
