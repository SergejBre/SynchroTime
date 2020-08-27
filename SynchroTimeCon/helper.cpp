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

    parser.addPositionalArgument( "Command", QCoreApplication::translate( "main", "String containing a storage command request" ) );
//    parser.addPositionalArgument( "InputFile", QCoreApplication::translate( "main", "Input file [path/]input.bin" ) );
//    parser.addPositionalArgument( "OutputFile", QCoreApplication::translate( "main", "Output file [path/]output.bin" ) );

#ifndef GUI_APP
    QCommandLineOption discovery( QStringList() << DISCOVERY << "discovery",
                                       QCoreApplication::translate( "main", "Discover for existing Serial Ports in the System" ),
                                       QCoreApplication::translate( "main", "" ), "0" );
    parser.addOption( discovery );
#endif

    QCommandLineOption portName( QStringList() << PORTNAME << "port",
                                       QCoreApplication::translate( "main", "Set up an available Serial Port (ttyUSB0 or COM2)" ),
                                       QCoreApplication::translate( "main", "PortName" ), "1" );
    parser.addOption( portName );

    QCommandLineOption information( QStringList() << INFORM << "info",
                                       QCoreApplication::translate( "main", "Read information about available RTC modules" ),
                                       QCoreApplication::translate( "main", "" ), "0" );
    parser.addOption( information );

    QCommandLineOption cmdCommand( QStringList() << CMDSTRING << "CMDstring",
                                       QCoreApplication::translate( "main", "String containing a storage command request" ),
                                       QCoreApplication::translate( "main", "Command" ), "1" );
    parser.addOption( cmdCommand );

#ifndef GUI_APP
    QCommandLineOption cmdList( QStringList() << CMDLIST << "list",
                                       QCoreApplication::translate( "main", "List of the containing a storage command request" ),
                                       QCoreApplication::translate( "main", "" ), "0" );
    parser.addOption( cmdList );
#endif

    QCommandLineOption test( QStringList() << TEST << "test",
                                       QCoreApplication::translate( "main", "Compare two binary files" ),
                                       QCoreApplication::translate( "main", "" ), "0" );
    parser.addOption( test );

    QCommandLineOption inputfile( QStringList() << INPUTFILE << "input",
                                       QCoreApplication::translate( "main", "Input binary file" ),
                                       QCoreApplication::translate( "main", "InputFile"), "1" );
    parser.addOption( inputfile );

    QCommandLineOption outputfile( QStringList() << OUTPUTFILE << "output",
                                       QCoreApplication::translate( "main", "Output binary file" ),
                                       QCoreApplication::translate( "main", "OutputFile"), "1" );
    parser.addOption( outputfile );

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

    // Request for Reset
    QByteArray requestForReset("@r");

    // Open the interface for communication with the device
    session->getInterface()->openSocket();

    // Send a command to the device
    session->getInterface()->writeTheData( requestForReset );
    qDebug() << "Send command: " << requestForReset;

    session->getInterface()->readTheData( TIME_WAIT );
    qDebug() << "Received bytes: " << session->getInterface()->getReceivedData().size();

    session->getInterface()->closeSocket();

    // Check of the Received request
    //! \todo : In the future, the return "RESET" should be checked!
    if ( !session->getInterface()->getReceivedData().isEmpty() )
    {
        standardOutput << session->getInterface()->getReceivedData() << endl;
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
        qCritical( logHelper ) << "Request for return of the Information failed.";
        return 1;
    }
    return 0;
}
