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
//!
//! \details
//!
bool commandLineParser( CMDcommand *const cmdCom, const QString &str )
{
    Q_ASSERT( cmdCom != NULL );
    Q_ASSERT( !str.isEmpty() );

    if ( str.isEmpty() || cmdCom == NULL )
    {
        qCritical( logHelper ) << QObject::tr( "The parameters are invalid" );
        return false;
    }
    cmdCom->status = -1;

    switch ( str.at(0).toLatin1() )
    {
    case 'c':
        cmdCom->command = 'c';
        break;
    case 'i':
        cmdCom->command = 'i';
        break;
    case 'v':
        cmdCom->command = 'v';
        break;
    case 'r':
        cmdCom->command = 'r';
        break;
    case 's':
        cmdCom->command = 's';

        if ( str.at(1).toLatin1() == ':' && str.size() >= 5 )
        {
            QString tmp = str.section( ':', 1, 1 );
            cmdCom->number = tmp.toInt();

            if ( str.section( ':', 2, 2 ).size() == 1 )
            {
                switch ( str.section( ':', 2, 2 ).at( 0 ).toLatin1() )
                {
                case 'f':
                    cmdCom->task = 'f';
                    break;
                case 'r':
                    cmdCom->task = 'r';
                    if ( !parserHelper( cmdCom, str ) )
                    {
                        return false;
                    }
                    break;
                case 'e':
                    cmdCom->task = 'e';
                    if ( !parserHelper( cmdCom, str ) )
                    {
                        return false;
                    }
                    break;
                case 'w':
                    cmdCom->task = 'w';
                    if ( !parserHelper( cmdCom, str ) )
                    {
                        return false;
                    }
                    break;
                default:
                    qCritical( logHelper ) << QObject::tr( "Unknown task: %1" ).arg( str.section( ':', 2, 2 ).at( 0 ).toLatin1() );
                    return false;
                }
            }
            else
            {
                qCritical( logHelper ) << QObject::tr( "Unknown task: %1" ).arg( str.section( ':', 2, 2 ) );
                return false;
            }
        }
        else
        {
            qCritical( logHelper ) << QObject::tr( "Bad CMD string: %1" ).arg( str.at(1).toLatin1() );
            return false;
        }
        break;
    default :
        qCritical( logHelper ) << QObject::tr( "Unknown command: %1" ).arg( str.at(0).toLatin1() );
        return false;
    }
    cmdCom->status = 0;
    return true;
}

// ------------------------------------------------------------------------
//! \brief
//!
//! \details
//!
inline bool parserHelper( CMDcommand *const cmdCom, const QString &str )
{
    QString tmp = str.section( ':', 3, 3 );
    if ( tmp.isEmpty() )
    {
        qCritical( logHelper ) << QObject::tr( "Unrecognized start address" );
        return false;
    }
    cmdCom->addressStart = tmp.toInt();
    tmp.clear();
    tmp = str.section( ':', 4, 4 );
    if ( tmp.isEmpty() )
    {
        qCritical( logHelper ) << QObject::tr( "Unrecognized end address" );
        return false;
    }
    cmdCom->addressEnd = tmp.toInt();
    return true;
}

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
//! Request for the Version of the device.
//!
//! \details
//!
//! \param[in] parent Pointer to the parent object used for QObject.
//!
int handleVersionRequest( Session *const session )
{
    Q_ASSERT( session != nullptr );

    // Open the interface for communication with the device
    if ( !session->getInterface()->openSocket() )
    {
        qCritical( logHelper ) << "Request for return of the Version failed.";
        return 1;
    }

    // Request for Version
    QByteArray requestForVersion("@ibbbbmm");

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
        standardOutput << "DS3231 clock time " << numberOfMSec << "ms: " << time.toString() << endl;
        standardOutput << "System local time " << localTimeMSecs << "ms: " << local.toString() << endl;
        standardOutput << "Difference between " << numberOfMSec - localTimeMSecs << "ms" << endl;
        if ( session->getInterface()->getReceivedData().count() >= 6 ) {
            qint8 offset_reg = session->getInterface()->getReceivedData().at(6);
            standardOutput << "Offset register val " << offset_reg << endl;
            if ( session->getInterface()->getReceivedData().count() >= 10 ) {
                float drift_in_ppm = 0.0;
                memcpy( &drift_in_ppm, session->getInterface()->getReceivedData().data() + 7, sizeof(drift_in_ppm) );
                standardOutput << "Time drift in ppm: " << drift_in_ppm << endl;
            }
        }
    }
    else
    {
        qCritical( logHelper ) << "Request for return of the Version failed.";
        return 1;
    }
    return 0;
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
        standardOutput << "DS3231 clock time " << numberOfMSec << "ms: " << time.toString() << endl;
        standardOutput << "System local time " << localTimeMSecs << "ms: " << local.toString() << endl;
        standardOutput << "Difference between " << numberOfMSec - localTimeMSecs << "ms" << endl;
        if ( session->getInterface()->getReceivedData().count() >= 6 ) {
            qint8 offset_reg = session->getInterface()->getReceivedData().at(6);
            standardOutput << "Offset register val " << offset_reg << endl;
            if ( session->getInterface()->getReceivedData().count() >= 10 ) {
                float drift_in_ppm = 0;
                memcpy( &drift_in_ppm, session->getInterface()->getReceivedData().data() + 7, sizeof(drift_in_ppm) );
                standardOutput << "Time drift in ppm: " << drift_in_ppm << endl;
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
