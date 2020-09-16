//------------------------------------------------------------------------------
//  Home Office
//  Nürnberg, Germany
//  E-Mail: sergej1@email.ua
//
//  Copyright (C) 2020 free Project SynchroTime RTC DS3231. All rights reserved.
//------------------------------------------------------------------------------
//  Project SynchroTime: Command-line client for adjust the exact time and
//  calibrating the RTC DS3231 module via the serial interface (UART).
//------------------------------------------------------------------------------

/**
* @mainpage SynchroTime - Command-line application (CLI-app)
*
* SynchroTime: Command-line client for adjust the exact time and
* calibrating the RTC DS3231 module via the serial interface (UART).
*
* @author SergejBre sergej1@email.ua
*/

/**
 * @file main.cpp
 *
 * @brief The file contains two important functions, main() and logMessageOutput()
 *
 *  The main function executes an instance of a Qt-console application,
 *  sets it up with the specified special parameters, and installs
 *  a Qt message handler defined in the logMessageOutput function.
 *  In addition, a log journal of the application messages is set up.
 */

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include "helper.h"
#include "settings.h"
#include <QCoreApplication>
#include <QtSerialPort/QSerialPortInfo>
#include <QCommandLineParser>
#include <QLoggingCategory>
#include <QDebug>
#include <QFile>
#include <QDateTime>

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
Q_LOGGING_CATEGORY(logMain, "main")
// Output stream for all messages (not for error!)
static QTextStream standardOutput( stdout );

// Smart pointer an the log-file
static QScopedPointer<QFile> m_logFile;

//------------------------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------------------------
void logMessageOutput( const QtMsgType type, const QMessageLogContext &context, const QString &msg );

/**
 * @brief main function of SynchroTime
 *
 * In this function, an instance of a Qt-console application app is executed and
 * set up with the parameters entered.
 *
 * @param[in] argc the number of parameter.
 * @param[in] argv the command line options.
 *
 * @return value of the function QApplication::exec()
 * Enters the main event loop and waits until exit() is called.
 * Returns the value that was set to exit() (which is 0 if exit() is called
 * via quit()).
 *
 * @note
 * none
 * @warning
 * none
 */
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    app.setApplicationName( "SynchroTime" );
    app.setApplicationVersion( "v.1.0.0, built on: " + QString(__DATE__).simplified() + " " + __TIME__ );

    Settings *settings = new Settings( &app );

    // Set the log file in the work directory
    m_logFile.reset( new QFile( settings->pathToLog() ) );

    if ( !m_logFile.data()->open( QFile::Append | QFile::Text ) )
    {
        qCritical( QObject::tr( "To the log file '%s'  can not be added.").toLocal8Bit(), qPrintable( m_logFile.data()->fileName() ) );
    }
    else
    {
        qInstallMessageHandler( logMessageOutput );
    }

    // Set the UART port name
    const QString portName = settings->portName();

    // Set a new Session
    Session *session = nullptr;
    {
        // Set Interface type
        Interface *interface = nullptr;
        if ( !QSerialPortInfo( portName ).isNull() )
        {
            interface = new InterfaceSP( &app, portName );
            interface->setBlockSize( 528U );
        }
        else
        {
            interface = new InterfaceSP( &app );
            qCritical( QObject::tr( "Serial Port '%s' does not exist in the system.").toLocal8Bit(), qPrintable( portName ) );
        }

        Q_ASSERT( interface != nullptr );
        session = new Session( &app, interface );
    }

    QCommandLineParser parser;
    setCommandLineParser( parser );
    // ------------------------------------------------------------------------
    // Process the actual command line options
    // ------------------------------------------------------------------------
    parser.process(app);

    const QStringList options = parser.optionNames();

    if ( options.count() < 1 )
    {
        parser.showHelp( 1 );
    }
/*
    foreach ( const QString & names, options )
    {
        qDebug() << "Option: " << names;
    }
*/
    // ------------------------------------------------------------------------
    // Process the actual command line arguments
    // ------------------------------------------------------------------------
    const QStringList args = parser.positionalArguments();
/*
    foreach ( const QString & argument, args )
    {
        qDebug() << "Argument: " << argument;
    }
*/
    // ------------------------------------------------------------------------
    // command line option discovery: -d
    // ------------------------------------------------------------------------
    if ( parser.isSet( DISCOVERY ) )
    {

        if ( args.count() > 0 )
        {
            qFatal( QObject::tr( "Invalid arguments '%s'" ).toLocal8Bit(), qPrintable( args.at(0) ) );
        }

        Q_ASSERT( session != nullptr );
        if ( session != nullptr )
        {
            // Discovery available virtual Serial ports
            InterfaceSP *interfaceSP = static_cast< InterfaceSP* >( session->getInterface() );

            // Info about all available in system serial ports.
            interfaceSP->searchAllSerialPort();
        }
    }

    // ------------------------------------------------------------------------
    // command line option port name: -p
    // ------------------------------------------------------------------------
    else if ( parser.isSet( PORT ) )
    {

        if ( args.count() > 0 )
        {
            qFatal( QObject::tr( "Invalid arguments '%s'" ).toLocal8Bit(), qPrintable( args.at(0) ) );
        }

        // Read Serial Port Name
        QString portName = parser.value( PORT );

        // Information about this serial port
        if ( !QSerialPortInfo( portName ).isNull() )
        {
            settings->setPortName( portName );
            standardOutput << QObject::tr( "Added new serial interface %1." ).arg( portName ).toLocal8Bit() << endl;
        }
        else
        {
            standardOutput << QObject::tr( "Serial Port %1 does not exist in the system. Command discard." ).arg( portName ).toLocal8Bit() << endl;
        }
    }

    // ------------------------------------------------------------------------
    // command line option information: -i
    // ------------------------------------------------------------------------
    else if ( parser.isSet( INFO ) )
    {

        if ( args.count() > 0 )
        {
            qFatal( QObject::tr( "Invalid arguments '%s'" ).toLocal8Bit(), qPrintable( args.at(0) ) );
        }
        handleInformationRequest( session );
    }

    // ------------------------------------------------------------------------
    // command line option Adjustment: -a
    // ------------------------------------------------------------------------
    else if ( parser.isSet( ADJUST ) )
    {

        if ( args.count() > 0 )
        {
            qFatal( QObject::tr( "Invalid arguments '%s'" ).toLocal8Bit(), qPrintable( args.at(0) ) );
        }

        Q_ASSERT( session != nullptr );
        if ( session != nullptr )
        {
            handleAdjustmentRequest( session );
        }
    }

    // ------------------------------------------------------------------------
    // command line option calibration: -c
    // ------------------------------------------------------------------------
    else if ( parser.isSet( CALIBR ) )
    {

        if ( args.count() > 0 )
        {
            qFatal( QObject::tr( "Invalid arguments '%s'" ).toLocal8Bit(), qPrintable( args.at(0) ) );
        }

        Q_ASSERT( session != nullptr );
        if ( session != nullptr )
        {
            handleCalibrationRequest( session );
        }
    }

    // ------------------------------------------------------------------------
    // command line option reset: -r
    // ------------------------------------------------------------------------
    else if ( parser.isSet( RESET ) )
    {

        if ( args.count() > 0 )
        {
            qFatal( QObject::tr( "Invalid arguments '%s'" ).toLocal8Bit(), qPrintable( args.at(0) ) );
        }

        Q_ASSERT( session != nullptr );
        if ( session != nullptr )
        {
            handleResetRequest( session );
        }
    }

    // ------------------------------------------------------------------------
    // command line option set register: -s
    // ------------------------------------------------------------------------
    else if ( parser.isSet( SETREG ) )
    {

        if ( args.count() > 0 )
        {
            qFatal( QObject::tr( "Invalid arguments '%s'" ).toLocal8Bit(), qPrintable( args.at(0) ) );
        }

        bool ok;
        // Read new value for offset register
        auto value = parser.value( SETREG ).toFloat( &ok );
        if ( !ok || ( value < -12.81 ) || ( value > 12.7 ) )
        {
            qFatal( QObject::tr( "Invalid argument '%s'" ).toLocal8Bit(), qPrintable( parser.value( SETREG ) ) );
        }

        Q_ASSERT( session != nullptr );
        if ( session != nullptr )
        {
            handleSetRegisterRequest( session, value );
        }
    }

    // Release the memory
    session->deleteLater();
    QTimer::singleShot( 0, &app, SLOT( quit() ) );

    return app.exec();
}

/**
 * @brief The function logMessageOutput is a message handler.
 *
 * This function redirects the messages by their category (QtDebugMsg,
 * QtInfoMsg, QtWarningMsg, QtCriticalMsg, QtFatalMsg) to the log file (m_logFile).
 * The message handler is a function that prints out debug messages,
 * warnings, critical and fatal error messages. The Qt library (debug mode)
 * contains hundreds of warning messages that are printed when internal errors
 * (usually invalid function arguments) occur. Qt built in release mode
 * also contains such warnings unless QT_NO_WARNING_OUTPUT and/
 * or QT_NO_DEBUG_OUTPUT have been set during compilation.
 * If you implement your own message handler, you get total control of these messages.
 *
 * @param[in] type of the type QtMsgType
 * @param[in] context of type QMessageLogContext
 * @param[in] msg of the type QString
 *
 * @note
 * - The output of messages is also output to the terminal console. This is for debugging purposes.
 * - Additional information, such as a line of code, the name of the source file, function names
 * cannot be displayed for the release of the program.
 *
 * @warning
 * - The default message handler prints the message to the standard output
 * under X11 or to the debugger under Windows. If it is a fatal message,
 * the application aborts immediately.
 * - Only one message handler can be defined, since this is usually done on
 * an application-wide basis to control debug output.
 */
void logMessageOutput( const QtMsgType type, const QMessageLogContext &context, const QString &msg )
{
    QTextStream out( m_logFile.data() );
    // Write the date of the recording
    out << QDateTime::currentDateTime().toString( "yyyy-MM-dd hh:mm:ss.zzz " );

    QByteArray localMsg = msg.toLocal8Bit();
    switch ( type )
    {
    case QtDebugMsg:
        out << "DBG " << context.category << ": " << msg << endl;
        break;
    case QtInfoMsg:
        out << "INF " << context.category << ": " << msg << endl;
        break;
    case QtWarningMsg:
        out << "WRN " << context.category << ": " << msg << " (" << context.file << ":" << context.line << ", " << context.function << ")" << endl;
        fprintf( stderr, "Warning: " );
        break;
    case QtCriticalMsg:
        out << "CRT " << context.category << ": " << msg << " (" << context.file << ":" << context.line << ", " << context.function << ")" << endl;
        fprintf( stderr, "Critical: " );
        break;
    case QtFatalMsg:
        out << "FTL " << context.category << ": " << msg << " (" << context.file << ":" << context.line << ", " << context.function << ")" << endl;
        fprintf( stderr, "Fatal: " );
        break;
    default :
        out << "ERR " << context.category << ": " << msg << " (" << context.file << ":" << context.line << ", " << context.function << ")" << endl;
        fprintf( stderr, "Unkown category: %d", type );
    }
    // Write to the output category of the message and the message itself
//    out << context.category << ": " << msg << " (" << context.file << ":" << context.line << ", " << context.function << ")" << endl;
    out.flush();
    fprintf( stderr, "%s\n", localMsg.constData() );
}
