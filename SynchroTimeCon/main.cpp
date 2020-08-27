//------------------------------------------------------------------------------
//  Home Office
//  Nürnberg, Germany
//  E-Mail: sergej1@email.ua
//
//  Copyright (C) 2020 free Project SynchroTime RTC DS3231. All rights reserved.
//------------------------------------------------------------------------------
//  Project SynchroTime: Time synchronization of the Precision RTC module DS3231
//  with the UTC System Time via the Serial Interface (UART).
//------------------------------------------------------------------------------

/**
* @mainpage SynchroTime - client console application
*
* SynchroTime - Time synchronization of the Precision RTC module DS3231
* with the UTC System Time via the Serial Interface (UART).
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
#include <QCommandLineOption>
#include <QLoggingCategory>
#include <QDebug>
#include <QFile>
#include <QUrl>
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
 * @brief main function
 *
 * In this function, an instance of a Qt-console application app is executed and
 * set up with the parameters entered.
 *
 * @param argc this parameter \todo
 * @param argv this parameter \todo
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
        qCritical() << "To the log file " << m_logFile.data()->fileName() << "  can not be added.";
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
            qCritical( QObject::tr( "Serial Port %1 does not exist in the system." ).arg( portName ).toLocal8Bit() );
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

    foreach ( const QString & names, options )
    {
        qDebug() << "Option: " << names;
    }

    // ------------------------------------------------------------------------
    // Process the actual command line arguments
    // ------------------------------------------------------------------------
    const QStringList args = parser.positionalArguments();

    foreach ( const QString & argument, args )
    {
        qDebug() << "Argument: " << argument;
    }

    // ------------------------------------------------------------------------
    // command line option discovery: -d
    // ------------------------------------------------------------------------
    if ( parser.isSet( DISCOVERY ) )
    {

        if ( args.count() > 0 )
        {
            qFatal( QObject::tr( "Invalid arguments %1" ).arg( args.at(0) ).toLocal8Bit() );
        }

        if ( session != nullptr )
        {
            // Discovery available virtual Serial ports
            InterfaceSP *interfaceSP = static_cast< InterfaceSP* >( session->getInterface() );

            // Info about all available in system serial ports.
            interfaceSP->searchAllSerialPort();
        }
    }

    // ------------------------------------------------------------------------
    // command line option port: -p
    // ------------------------------------------------------------------------
    else if ( parser.isSet( PORTNAME ) )
    {

        if ( args.count() > 0 )
        {
            qFatal( QObject::tr( "Invalid arguments %1" ).arg( args.at(0) ).toLocal8Bit() );
        }

        // Read Serial Port Name
        QString portName = parser.value( PORTNAME );

        // Information about this serial port
        if ( !QSerialPortInfo( portName ).isNull() )
        {
            settings->setPortName( portName );
            standardOutput << QObject::tr( "New serial interface %1 was set up." ).arg( portName ).toLocal8Bit() << endl;
        }
        else
        {
            standardOutput << QObject::tr( "Serial Port %1 does not exist in the system. Command discard." ).arg( portName ).toLocal8Bit() << endl;
        }
    }

    // ------------------------------------------------------------------------
    // command line option configure: -c -i <configfile>
    // ------------------------------------------------------------------------
    else if ( parser.isSet( CONFIGURE ) )
    {

        if ( args.count() > 0 )
        {
            qFatal( QObject::tr( "Invalid arguments %1" ).arg( args.at(0) ).toLocal8Bit() );
        }

        if ( !parser.isSet( INPUTFILE ) )
        {
            qFatal( "Second parameter -i <BinarFile> missing" );
        }

        // Read a url of configuration file
        QUrl urlBinaryFile( parser.value( INPUTFILE ) );

//        handleConfiguration( ioHandler, session, configFile, urlBinaryFile );
    }

    // ------------------------------------------------------------------------
    // command line option List of the command request: -l -i <listfile>
    // ------------------------------------------------------------------------
    else if ( parser.isSet( CMDLIST ) )
    {

        if ( args.count() > 0 )
        {
            qFatal( QObject::tr( "Invalid arguments %1" ).arg( args.at(0) ).toLocal8Bit() );
        }

        if ( parser.isSet( INPUTFILE ) )
        {
            // Read a list as a binary file
            QUrl urlBinaryFile( parser.value( INPUTFILE ) );

            // Check for Exist the binary file
/*            if ( !(ioHandler->isFileExists(urlBinaryFile)) )
            {
                qFatal( QObject::tr( "The binary file %1 not found" ).arg( urlBinaryFile.fileName() ).toLocal8Bit() );
            }
*/
            if ( args.count() > 0 )
            {
                qFatal( QObject::tr( "Invalid arguments %1" ).arg( args.at(0) ).toLocal8Bit() );
            }
/*
            // Read a List command from the binary file
            QByteArrayList cmdlist = ioHandler->readFileToList( urlBinaryFile );
            if ( cmdlist.size() < 1 )
            {
                qFatal( "Reading from the binary file failed" );
            }

            QMutableListIterator<QByteArray> iterator( cmdlist );
            while ( iterator.hasNext() )
            {
                int index = -1;
                while ( (index = iterator.peekNext().indexOf( "  " )) > -1 )
                {
                    iterator.peekNext().remove( index, 1 );
                }

                if ( iterator.peekNext().at( 0 ) == ' ' )
                {
                    iterator.peekNext().remove( 0, 1 );
                }
                if ( iterator.next().at( 0 ) == '\n' )
                {
//                    iterator.remove();
                }
            }

            int line = 0;
            foreach ( const QString &commandline, cmdlist )
            {
                line++;
                if ( commandline.at( 0 ) == '\n' || commandline.at( 0 ) == '#' )
                {
                    continue;
                }

                // ------------------------------------------------------------------------
                // Parse a command line from the list
                // ------------------------------------------------------------------------
                if ( validateCommandLine( parser, commandline, ioHandler, line ) )
                {
                    qFatal( QObject::tr( "Fatal error in the line: %1" ).arg( line ).toLocal8Bit() );
                }
            }

            //-------------------------------------------------------------
            // Section to process all commands line
            //-------------------------------------------------------------
            line = 0;
            foreach ( const QString &commandline, cmdlist )
            {
                line++;
                int status = 0;
                //-------------------------------------------------------------
                // execute a commands line
                //-------------------------------------------------------------
                if ( (status = executeCommandLine( parser, commandline, ioHandler, session, configFile, line )) != 0 )
                {
                    qFatal( QObject::tr( "Fatal error in the line: %1, status: %2" ).arg( line ).arg( status ).toLocal8Bit() );
                }
//                qDebug() << QObject::tr( "Processing of the command line %1 was successful." ).arg( line ).toLocal8Bit();
            }
*/
        }
        else
        {
            qFatal( "Second parameter -i <InputFile> missing" );
        }
    }

    // ------------------------------------------------------------------------
    // command line option CMD-String: -f c -o <configfile> or
    //                                 -f s-command [-i <inputfile>] [-o <outfile>] or
    //                                 -f i <-o <infofile> >
    // where s-command: s:number:command:addressBegin:addressEnd
    //
    // ------------------------------------------------------------------------
    //  Examples for CMD command and s-command:
    // -f c -o configfile
    //
    // -f s:0:e:0:0
    // -f s:0:w:0:3 -i partition_table.bin
    // -f s:0:r:0:3 -o outfile1
    // -t -i partition_table.bin -o outfile1
    //
    // -f s:0:e:16:16
    // -f s:0:w:256:259 -i partition_table.bin
    // -f s:0:r:256:259 -o outfile2
    // -t -i partition_table.bin -o outfile2
    //
    // -f i -o infofile
    // ------------------------------------------------------------------------
    // Storage Access:
    //
    // Read:
    // Instruction:	“@s”[<SNum>”r”<START><END>]<CRC>			(16 bytes)
    // Result:		<STATUS>[<DATA>]<CRC>				(5 + X bytes / 1 byte)
    //
    // Write:
    // Instruction:	“@s”[<SNum>”w”<START><END><DATA>]<CRC>		(16 + X bytes)
    // Result:		<STATUS>								(1 byte)
    //
    // Erase:
    // Instruction:	“@s”[<SNum>”e”<START><END>]<CRC>			(16 bytes)
    // Result:		<STATUS>								(1 byte)
    //
    // Format:
    // Instruction:	“@s”[<SNum>”f”]<CRC>						(8 bytes)
    // Result:		<STATUS>								(1 byte)
    //
    // <SNum>:	Storage Number (1 byte, 0x00-0x0F)
    // <START>: 	First block of storage device to be read/written/erased (4 bytes)
    // <END>:	Last block of storage device  to be read/written/erased (4 bytes)
    // <DATA>:	Data to be read/written (X = ((<END> - <START> + 1) * <BLOCKSIZE>) bytes)
    // <CRC>:	CRC checksum, calculated over payload (4 bytes)
    // <STATUS>:	Status for received request (1 byte). If status is different from 0x00 (“Success”),
    //          no further data follows.
    // ------------------------------------------------------------------------

    else if ( parser.isSet( CMDSTRING ) )
    {

        if ( args.count() > 0 )
        {
            qFatal( QObject::tr( "Invalid arguments %1" ).arg( args.at(0) ).toLocal8Bit() );
        }

        // Read CMD command
        QString command = parser.value( CMDSTRING );
        qDebug() << "command " << command;
        CMDcommand cmdCom;
        // Parse CMD command
        if ( commandLineParser( &cmdCom, command ) )
        {
            qDebug() << "number " << cmdCom.number;
            qDebug() << "task " << cmdCom.task;
            qDebug() << "start " << cmdCom.addressStart;
            qDebug() << "end " << cmdCom.addressEnd;

            switch ( cmdCom.command )
            {

            // Read current configuraton from DLC-X device and save into the outfile.
            // Requires option [-o <>] to be also specified.
            case 'c':
                if ( parser.isSet( OUTPUTFILE ) )
                {
                    // Set a url of the configfile
                    QUrl urlBinaryFile( parser.value( OUTPUTFILE ) );

                    if ( args.count() > 0 )
                    {
                        qFatal( QObject::tr( "Invalid arguments %1" ).arg( args.at(0) ).toLocal8Bit() );
                    }

//                    handleConfigurationRequest( ioHandler, session, configFile, urlBinaryFile );
                }
                else
                {
                    qFatal( "Second option -o <OutputFile> missing." );
                }
                break;

            // Read statistical information from DLC-X device and save into the outfile.
            // Requires option [-o <>] to be also specified.
            case 'i':
                if ( parser.isSet( OUTPUTFILE ) )
                {
                    // Set a url of the outfile
                    QUrl urlInfoFile( parser.value( OUTPUTFILE ) );

                    if ( args.count() > 0 )
                    {
                        qFatal( QObject::tr( "Invalid arguments %1" ).arg( args.at(0) ).toLocal8Bit() );
                    }

//                    handleInformationRequest( ioHandler, session, urlInfoFile );
                }
                else
                {
                    qFatal( "Second option -o <OutputFile> missing." );
                }
                break;

            // Read Version from DLC-X device. Result: “DLC-30 MFM V0000” / “DLC-30 QSB V0000”
            case 'v':

                if ( args.count() > 0 )
                {
                    qFatal( QObject::tr( "Invalid arguments %1" ).arg( args.at(0) ).toLocal8Bit() );
                }

                handleVersionRequest( session );
                break;

            // Reset ot the DLC-X device. Result: “RESET”
            case 'r':

                if ( args.count() > 0 )
                {
                    qFatal( QObject::tr( "Invalid arguments %1" ).arg( args.at(0) ).toLocal8Bit() );
                }

                handleResetRequest( session );
                break;

            // Operations with a storage device
            case 's':
            {
                // read configFile
//                session->getProtocol()->readDomDocument( ioHandler->getDomDocument_p() );

                // Open the interface for communication with the device
                session->getInterface()->openSocket();

                // Validation for the entered device: cmdCom
/*                if ( cmdCom.number < session->getProtocol()->getDeviceList().count() )
                {

                    switch ( cmdCom.task )
                    {
                    // Format storage device <DEVICE>.
                    case 'f':
                    {
                        // Access to the Format for a Device
                        const QByteArray accessForFormat = session->getProtocol()->accessFormat( cmdCom.number );

                        // Sendet einen Befehl an das Geräte
                        session->getInterface()->writeTheData( accessForFormat );
                        qDebug() << "Send command: " << accessForFormat;

                        session->getInterface()->readTheData( TIME_WAIT );
                        qDebug() << "Received bytes: " << session->getInterface()->getReceivedData().size();

                        // Check of the Status for received request
                        checkStatus( &session->getInterface()->getReceivedData() );
                    }
                        break;

                    // Read blocks <START>...<END> from storage device <DEVICE>.
                    // Requires option [-o <>] to be also specified.
                    case 'r':
                        if ( parser.isSet( OUTPUTFILE ) )
                        {
                            // Set a url of the output file
                            QString outfile = parser.value( OUTPUTFILE );
                            QUrl urlOutFile( outfile );

                            if ( args.count() > 0 )
                            {
                                qFatal( QObject::tr( "Invalid arguments %1" ).arg( args.at(0) ).toLocal8Bit() );
                            }

                            // Access for read from a storage device
                            const QByteArray accessForRead = session->getProtocol()->accessRead( cmdCom.number, cmdCom.addressStart, cmdCom.addressEnd );

                            // Sends a access to the device
                            session->getInterface()->writeTheData( accessForRead );
                            qDebug() << "Send command: " << accessForRead;

                            session->getInterface()->readTheData( TIME_WAIT );
                            qDebug() << "Received bytes: " << session->getInterface()->getReceivedData().size();

                            // Check of the Status for received request
                            if ( checkStatus( &session->getInterface()->getReceivedData() ) )
                            {
                                // Check CRC32 checksum
                                if ( session->getProtocol()->checkData( session->getInterface()->getReceivedData() ) )
                                {
                                    int size = session->getInterface()->getReceivedData().size();
                                    session->getInterface()->getReceivedData().remove( size-4, 4 );

                                    // save read bytes as a binary file
                                    if ( ioHandler->writeBinaryFile( session->getInterface()->getReceivedData(), urlOutFile ) )
                                    {
                                        standardOutput << QObject::tr( "Save binary file: %1 was successful." ).arg( urlOutFile.fileName() ).toLocal8Bit() << endl;
                                    }
                                    else
                                    {
                                        qFatal( QObject::tr( "Save binary file: %1 failed." ).arg( urlOutFile.fileName() ).toLocal8Bit() );
                                    }
                                }
                                else
                                {
                                    qCritical( "The CRC32 verification of data failed." );
                                }
                            }
                        }
                        else
                        {
                            qFatal( "Second option -o <OutputFile> missing." );
                        }
                        break;

                    // Erase blocks <START>...<END> from storage device <DEVICE>.
                    case 'e':
                    {
                        // Access for erase blocks <START>...<END>
                        const QByteArray accessForEraset = session->getProtocol()->accessErase( cmdCom.number, cmdCom.addressStart, cmdCom.addressEnd );

                        // Sendet einen Befehl an das Geräte
                        session->getInterface()->writeTheData( accessForEraset );
                        qDebug() << "Send command: " << accessForEraset;

                        session->getInterface()->readTheData( TIME_WAIT );
                        qDebug() << "Received bytes: " << session->getInterface()->getReceivedData().size();

                        // Check of the Status for received request
                        checkStatus( &session->getInterface()->getReceivedData() );
                    }
                        break;

                    // Write blocks <START>...<END> from storage device <DEVICE>.
                    // Requires option [-i <>] to be also specified.
                    case 'w':
                    {
                        if ( parser.isSet( INPUTFILE ) )
                        {
                            // Set a url of the input file
                            QUrl urlInFile( parser.value( INPUTFILE ) );

                            // Check for Exist the binary file 1
                            if ( !(ioHandler->isFileExists( urlInFile )) )
                            {
                                qFatal( QObject::tr( "The binary file %1 not found" ).arg( urlInFile.fileName() ).toLocal8Bit() );
                            }

                            if ( args.count() > 0 )
                            {
                                qFatal( QObject::tr( "Invalid arguments %1" ).arg( args.at(0) ).toLocal8Bit() );
                            }

                            QByteArray data = ioHandler->readBinaryFile( urlInFile );
#if 0
                            if ( dataList.count() > 1 )
                            {
                                qFatal( QObject::tr( "The contents of the file: %1 are not binary." ).arg( urlInFile.fileName() ).toLocal8Bit() );
                            }
#endif
                            // Access for write to a storage device
                            const QByteArray accessForWrite = session->getProtocol()->accessWrite( cmdCom.number, cmdCom.addressStart, cmdCom.addressEnd, data );

                            // Sends a access to the device
                            session->getInterface()->writeTheData( accessForWrite );
                            qDebug() << "Send command: " << accessForWrite;

                            session->getInterface()->readTheData( TIME_WAIT, 1U );
                            qDebug() << "Received bytes: " << session->getInterface()->getReceivedData().size();

                            // Check of the Status for received request
                            checkStatus( &session->getInterface()->getReceivedData() );
                        }
                        else
                        {
                            qFatal( "Second option -i <InputFile> missing." );
                        }
                    }
                        break;

                    default:
                        qFatal( "Unknown task" );
                    }
                }
                else
                {
                    qDebug() << session->getProtocol()->getDeviceList().count();
                    qFatal( QObject::tr( "The number entered by the device is invalid.").toLocal8Bit() );
                }
*/
                session->getInterface()->closeSocket();
            }
                break;

            default:
                qFatal( "Unknown command" );
            }
        }
    }

    // ------------------------------------------------------------------------
    // command line option test: -t -i file1.bin -o file2.bin
    // ------------------------------------------------------------------------
    else if ( parser.isSet( TEST ) )
    {
#if 0
        if ( args.count() > 0 )
        {
            qFatal( QObject::tr( "Invalid arguments %1" ).arg( args.at(0) ).toLocal8Bit() );
        }
#endif
        if ( !parser.isSet( INPUTFILE ) )
        {
            qFatal( "Second parameter -i <BinarFile> missing" );
        }

        if ( !parser.isSet( OUTPUTFILE ) )
        {
            qFatal( "Second parameter -o <BinarFile> missing" );
        }
/*
        // Read a url of binary file 1
        QUrl urlFile1( parser.value( INPUTFILE ) );

        // Read a url of binary file 2
        QUrl urlFile2( parser.value( OUTPUTFILE ) );

        if ( handleComparation( ioHandler, urlFile1, urlFile2 ) > 0 )
        {
            qFatal( QObject::tr( "Processing of the command line failed." ).toLocal8Bit() );
        }
*/
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
