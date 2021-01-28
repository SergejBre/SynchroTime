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
//!
//! \file rtc.cpp
//!
//! \brief This file contains the implementations of the methods of the RTC class.
//!
//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include "rtc.h"
#include <QDebug>
#include <QThread>
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
#define STARTBYTE '@';  //!< Start byte of the protocol of communication with the RTC device.
#define DEVICE_ID 0x00; //!< ID of the RTC device.
#define REBOOT_WAIT 1500 //!< Interval for a time-out in milliseconds
#define TIME_WAIT 50 //!< Interval for a time-out in milliseconds

//------------------------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------------------------

//!
//! \brief RTC::RTC
//! \param portName
//! \param parent
//!
RTC::RTC( const QString &portName, QObject *parent )
    : QObject( parent ),
      m_pSerialPort( nullptr ),
      m_pTimerCheckConnection( nullptr )
{
    // Initialization of the serial interface.
    m_pSerialPort = ::new( std::nothrow ) QSerialPort( this );
    if ( m_pSerialPort != nullptr ) {
        m_pSerialPort->setPortName( portName );
        // Data transfer rate: 115200 бит/с.
        m_pSerialPort->setBaudRate(QSerialPort::Baud115200);
        m_pSerialPort->setDataBits(QSerialPort::Data8);
        m_pSerialPort->setParity(QSerialPort::NoParity);
        m_pSerialPort->setStopBits(QSerialPort::OneStop);
        m_pSerialPort->setFlowControl(QSerialPort::NoFlowControl);

        QObject::connect( m_pSerialPort, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
                          this, &RTC::handleError );
        // Connect the serial port.
        connectToRTC();

        // Create a timer with 1 second intervals.
        m_pTimerCheckConnection = ::new( std::nothrow ) QTimer( this );
        if ( m_pTimerCheckConnection != nullptr ) {
        m_pTimerCheckConnection->setInterval( 1000 );

        // After a time of 1 s, the statusRequest() command is called.
        // This is where the lambda function is used to avoid creating a slot.
        QObject::connect( m_pTimerCheckConnection, &QTimer::timeout, [this]() {
            statusRequest();
        });
        m_pTimerCheckConnection->start();
        }
    }
}

//!
//! \brief RTC::RTC
//! \param portSettings
//! \param parent
//!
RTC::RTC( const Settings_t &portSettings, QObject *parent )
    : QObject( parent ),
      m_pSerialPort( nullptr ),
      m_pTimerCheckConnection( nullptr )
{
    // Initialization of the serial interface.
    m_pSerialPort = ::new( std::nothrow ) QSerialPort( this );
    if ( m_pSerialPort != nullptr ) {
        m_pSerialPort->setPortName( portSettings.name );
        m_pSerialPort->setBaudRate( portSettings.baudRate );
        m_pSerialPort->setDataBits( portSettings.dataBits );
        m_pSerialPort->setParity( portSettings.parity );
        m_pSerialPort->setStopBits( portSettings.stopBits );
        m_pSerialPort->setFlowControl( portSettings.flowControl );

        QObject::connect( m_pSerialPort, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
                          this, &RTC::handleError );
        // Open the serial port.
        m_isConnected = openSerialPort();

        // Create a timer with 1 second intervals.
        m_pTimerCheckConnection = ::new( std::nothrow ) QTimer( this );
        if ( m_pTimerCheckConnection != nullptr ) {
            m_pTimerCheckConnection->setInterval( 1000 );

            // After a time of 1 s, the statusRequest() command is called.
            // This is where the lambda function is used to avoid creating a slot.
            QObject::connect( m_pTimerCheckConnection, &QTimer::timeout, [this]() {
                statusRequest();
            });
            m_pTimerCheckConnection->start();
        }
    }
}

//!
//! \brief RTC::~RTC
//!
RTC::~RTC()
{
    if ( m_pTimerCheckConnection != nullptr ) {
        delete m_pTimerCheckConnection;
    }
    if ( m_pSerialPort != nullptr ) {
        delete m_pSerialPort;
    }
    qDebug() << QStringLiteral( "Obj RTC destroyed." );
}

//!
//! \brief RTC::isConnected
//! \return
//!
bool RTC::isConnected() const
{
    return m_isConnected;
}

//!
//! \brief RTC::informationRequestSlot
//!
void RTC::informationRequestSlot()
{
    if ( m_isConnected )
    {
        informationRequest();
    }
}

//!
//! \brief RTC::adjustmentRequestSlot
//!
void RTC::adjustmentRequestSlot()
{
    if ( m_isConnected )
    {
        adjustmentRequest();
    }
}

//!
//! \brief RTC::calibrationRequestSlot
//!
void RTC::calibrationRequestSlot()
{
    if ( m_isConnected )
    {
        calibrationRequest();
    }
}

//!
//! \brief RTC::resetRequestSlot
//!
void RTC::resetRequestSlot()
{
    if ( m_isConnected )
    {
        resetRequest();
    }
}

//!
//! \brief RTC::setRegisterRequestSlot
//!
void RTC::setRegisterRequestSlot( const float newValue )
{
    if ( m_isConnected )
    {
        setRegisterRequest( newValue );
    }
}

//!
//! \brief RTC::versionRequestSlot
//!
void RTC::statusRequestSlot()
{
    if ( m_isConnected )
    {
        statusRequest();
    }
}

//!
//! \brief RTC::handleError
//! \param error
//!
void RTC::handleError( QSerialPort::SerialPortError error )
{
    Q_ASSERT( m_pSerialPort != nullptr );
    if ( error == QSerialPort::ResourceError ) {
        emit portError( m_pSerialPort->errorString() );
        m_pSerialPort->blockSignals( true );
    }
    else if ( error != QSerialPort::NoError )
    {
        QString output( QStringLiteral( "SerialPortError::" ) );
        QTextStream out( &output );
        switch ( error ) {

        case QSerialPort::DeviceNotFoundError:
            out << QStringLiteral( "DeviceNotFoundError" );
            break;
        case QSerialPort::PermissionError:
            out << QStringLiteral( "PermissionError" );
            break;
        case QSerialPort::OpenError:
            out << QStringLiteral( "OpenError" );
            break;
        case QSerialPort::NotOpenError:
            out << QStringLiteral( "NotOpenError" );
            break;
        case QSerialPort::ParityError:
            out << QStringLiteral( "ParityError" );
            break;
        case QSerialPort::FramingError:
            out << QStringLiteral( "FramingError" );
            break;
        case QSerialPort::BreakConditionError:
            out << QStringLiteral( "BreakConditionError" );
            break;
        case QSerialPort::WriteError:
            out << QStringLiteral( "WriteError" );
            break;
        case QSerialPort::ReadError:
            out << QStringLiteral( "ReadError" );
            m_pSerialPort->blockSignals( true );
            break;
        case QSerialPort::ResourceError:
            out << QStringLiteral( "ResourceError" );
            break;
        case QSerialPort::UnsupportedOperationError:
            out << QStringLiteral( "UnsupportedOperationError" );
            break;
        case QSerialPort::TimeoutError:
            out << QStringLiteral( "TimeoutError" );
            break;
        case QSerialPort::UnknownError:
            out << QStringLiteral( "UnknownError" );
            break;
        default:
            break;
        }
        out << ' ' << m_pSerialPort->errorString();
        qCritical() << output;
    }
}

//!
//! \brief RTC::openSerialPort
//! \retval true
//! \retval false
//!
bool RTC::openSerialPort() const
{
    Q_ASSERT( m_pSerialPort != nullptr );
    if ( m_pSerialPort->open( QSerialPort::ReadWrite ) )
    {
        m_pSerialPort->setBreakEnabled( false );
        m_pSerialPort->setDataTerminalReady( true );
        this->thread()->msleep( REBOOT_WAIT );
        return true;
    }
    else
    {
        qDebug() << QStringLiteral( "Failed to open the serial interface" );
    }
    return false;
}

//!
//! \brief RTC::connectToRTC
//!
void RTC::connectToRTC()
{
    Q_ASSERT( m_pSerialPort != nullptr );
    if ( m_pSerialPort->open( QSerialPort::ReadWrite ) )
    {
        m_pSerialPort->setBreakEnabled( false );
        m_pSerialPort->setDataTerminalReady( true );
        this->thread()->msleep( REBOOT_WAIT );
        // Make sure the RTC device is actually connected to the serial port.
        m_isConnected = statusRequest();

        if ( m_isConnected )
        {
            qDebug() << QStringLiteral( "The RTC is connected." );
        }
        else
        {
            qDebug() << QStringLiteral( "Another device is connected to the RTC serial port!" );
        }
    }
    else
    {
        qDebug() << QStringLiteral( "The serial port is not connected." );
        m_isConnected = false;
    }
}

//!
//! \brief RTC::sendRequest
//! \param request
//! \param size
//! \param data
//! \return
//!
QByteArray RTC::sendRequest( Request request, quint8 size, const quint8 *const data ) const
{
    // Data that are sent to the serial interface.
    QByteArray sentData;
//    sentData.resize(size + 4);
    sentData.resize(size + 2);
    sentData[0] = STARTBYTE;
//    sentData[1] = DEVICE_ID;

    // Checksum = the sum of all bytes starting from the request command.
    quint8 crc = 0;
    crc += sentData[1] = static_cast<quint8>( request );
//    crc += sentData[2] = size;

    // If there are data bytes, then they are also added to the checksum.
    if ( size > 0 && data != nullptr )
    {
        int i;
        for ( i = 0; i < size; ++i)
        {
//            crc += sentData[i + 3] = data[i];
            crc += sentData[i + 2] = data[i];
        }
    }

    // The last byte is the checksum.
//    sentData[size + 3] = crc;

    Q_ASSERT( m_pSerialPort->isOpen() );
    // Send data to the serial port and wait up to 50 ms until the write is done
    m_pSerialPort->write( sentData );
    m_pSerialPort->waitForBytesWritten( TIME_WAIT );

    // Sleep 50 ms, waiting for the microcontroller to process the data and respond
    this->thread()->msleep( TIME_WAIT );
    // Reading data from RTC.
    m_pSerialPort->waitForReadyRead( TIME_WAIT );
    return m_pSerialPort->readAll();
}

//!
//! \brief RTC::informationRequest
//!
void RTC::informationRequest()
{
    // Data that are sent to the serial interface.
    quint8 sentData[6];
    const QDateTime local(QDateTime::currentDateTime());
    const qint64 localTimeMSecs = local.toMSecsSinceEpoch();
    const quint32 localTimeSecs = localTimeMSecs/1000;
    const quint16 milliSecs = localTimeMSecs - localTimeSecs * 1000;
    memcpy( sentData, &localTimeSecs, sizeof(localTimeSecs) );
    memcpy( sentData + sizeof(localTimeSecs), &milliSecs, sizeof(milliSecs) );

    // Send a request to the RTC device
    QByteArray receivedData = sendRequest( Request::INFO, sizeof( sentData ), sentData );

    QString output;
    QTextStream out( &output );
    out << QStringLiteral( "@i Request for Info:" ) << endl;
    const qint8 blength = receivedData.size();

    // Check of the received response to the request
    if ( blength > 5 )
    {
        qint64 numberOfMSec = 0LL;
        quint16 numberOfSec = 0U;
        auto p_byteBuffer = receivedData.constData();
        memcpy( &numberOfMSec, p_byteBuffer, sizeof( quint32 ) );
        numberOfMSec *= 1000;
        memcpy( &numberOfSec, p_byteBuffer + 4, sizeof( numberOfSec ) );
        numberOfMSec += numberOfSec;
        QDateTime time( QDateTime::fromMSecsSinceEpoch( numberOfMSec ) );
        out << "Time from DS3231 \t" << numberOfMSec << " ms: " << time.toString("dd.MM.yyyy hh:mm:ss.zzz") << endl;
        out << "System local time\t" << localTimeMSecs << " ms: " << local.toString("dd.MM.yyyy hh:mm:ss.zzz") << endl;
        out << "Difference between\t" << numberOfMSec - localTimeMSecs << " ms" << endl;
        if ( blength > 6 ) {
            const float offset_reg = float( p_byteBuffer[6] )/10;
            out << "Offset reg. in ppm\t" << offset_reg << " ppm" << endl;
            if ( blength > 10 ) {
                float drift_in_ppm = 0;
                memcpy( &drift_in_ppm, p_byteBuffer + 7, sizeof( drift_in_ppm ) );
                out << "Time drift in ppm\t" << drift_in_ppm << " ppm" << endl;
                if ( blength > 14 ) {
                    qint64 lastAdjustOfTimeMSec = 0LL;
                    memcpy( &lastAdjustOfTimeMSec, p_byteBuffer + 11, sizeof( quint32 ) );
                    if ( lastAdjustOfTimeMSec < 0xFFFFFFFF ) {
                        lastAdjustOfTimeMSec *= 1000;
                        time = QDateTime::fromMSecsSinceEpoch( lastAdjustOfTimeMSec );
                        out << "Last time adjustm.\t" << lastAdjustOfTimeMSec << " ms: " << time.toString("dd.MM.yyyy hh:mm:ss.zzz") << endl;
//                        out << "Time drift in ppm\t" << float(numberOfMSec - localTimeMSecs)*1000000/(localTimeMSecs - lastAdjustOfTimeMSec ) << " ppm" << endl;
                    }
                }
            }
        }
    }
    else
    {
        out << QStringLiteral( "Request for the information failed" );
    }
    emit getData( output.toLocal8Bit() );
}

//!
//! \brief RTC::adjustmentRequest
//!
void RTC::adjustmentRequest()
{
    // Data that are sent to the serial interface.
    quint8 sentData[6];
    QDateTime local(QDateTime::currentDateTime());
    const qint64 localTimeMSecs = local.toMSecsSinceEpoch();
    quint32 localTimeSecs = localTimeMSecs/1000;
    const quint16 milliSecs = localTimeMSecs - localTimeSecs * 1000;
    localTimeSecs++;
    memcpy( sentData, &localTimeSecs, sizeof(localTimeSecs) );
    sentData[4] = sentData[5] = 0;

    this->thread()->msleep( 1000 - milliSecs );

    // Send a request to the RTC device
    QByteArray receivedData = sendRequest( Request::ADJUST, sizeof( sentData ), sentData );

    QString output;
    QTextStream out( &output );
    out << QStringLiteral( "@a Request for Adjustment:" ) << endl;
    const qint8 blength = receivedData.size();

    // Check of the received response to the request.
    if ( blength > 0 )
    {
        const quint8 ret_value = receivedData.at( blength - 1 );
        local.setTime_t( localTimeSecs );
        out << "System local time\t" << local.toString( "ddd d MMM yyyy hh:mm:ss.zzz" ) << endl;
        out << "Request for adjustment " << ( ret_value ? "completed successfully" : "fail" ) << endl;
    }
    else
    {
        out << QStringLiteral( "Request for adjustment failed" );
    }
    emit getData( output.toLocal8Bit() );
}

//!
//! \brief RTC::calibrationRequest
//!
void RTC::calibrationRequest()
{
    // Data that are sent to the serial interface.
    quint8 sentData[6];
    QDateTime local( QDateTime::currentDateTime() );
    const qint64 localTimeMSecs = local.toMSecsSinceEpoch();
    quint32 localTimeSecs = localTimeMSecs/1000;
    const quint16 milliSecs = localTimeMSecs - localTimeSecs * 1000;
    localTimeSecs++;
    memcpy( sentData, &localTimeSecs, sizeof( localTimeSecs ) );
    sentData[4] = sentData[5] = 0;

    this->thread()->msleep( 1000 - milliSecs );

    // Send a request to the RTC device
    QByteArray receivedData = sendRequest( Request::CALIBR, sizeof( sentData ), sentData );

    QString output;
    QTextStream out( &output );
    out << QStringLiteral( "@c Request for Calibration:" ) << endl;
    const qint8 blength = receivedData.size();

    // Check of the received response to the request
    if ( blength > 0 )
    {
        auto byteBuffer = receivedData.constData();
        const quint8 ret_value = byteBuffer[ blength-1 ];
        local.setTime_t( localTimeSecs );
        out << "System local time\t" << local.toString( "ddd d MMM yyyy hh:mm:ss.zzz" ) << endl;
        qint8 offset_reg = byteBuffer[0];
        out << "Offset last value\t" << offset_reg << endl;
        if ( blength > 5 ) {
            float drift_in_ppm = 0;
            memcpy( &drift_in_ppm, byteBuffer + 1, sizeof( drift_in_ppm ) );
            out << "Time drift in ppm\t" << drift_in_ppm << " ppm" << endl;
            offset_reg = byteBuffer[5];
            out << "Offset new value\t" << offset_reg << endl;
        }
        out << "Request for calibration " << ( ret_value ? "completed successfully" : "fail" ) << endl;
    }
    else
    {
        out << QStringLiteral( "Request for calibration failed" );
    }
    emit getData( output.toLocal8Bit() );
}

//!
//! \brief RTC::resetRequest
//!
void RTC::resetRequest()
{
    // Send a request to the RTC device
    QByteArray receivedData = sendRequest( Request::RESET );

    QString output;
    QTextStream out( &output );
    out << QStringLiteral( "@r Request for Reset:" ) << endl;
    const qint8 blength = receivedData.size();

    // Check of the received response to the request
    if ( blength > 0 )
    {
        const quint8 ret_value = receivedData.at( blength - 1 );
        out << "Request for reset " << ( ret_value ? "completed successfully" : "fail" ) << endl;
    }
    else
    {
        out << "Request for reset failed";
    }
    emit getData( output.toLocal8Bit() );
}

//!
//! \brief RTC::setRegisterRequest
//!
void RTC::setRegisterRequest( const float newValue )
{
    // Data that are sent to the serial interface.
    quint8 sentData[4];
    memcpy( sentData, &newValue, sizeof( newValue ) );

    // Send a request to the RTC device
    QByteArray receivedData = sendRequest( Request::SETREG, sizeof( sentData ), sentData );

    QString output;
    QTextStream out( &output );
    out << QStringLiteral( "@s Request for SetRegister:" ) << endl;
    const qint8 blength = receivedData.size();

    // Check of the Received request
    if ( blength > 0 )
    {
        const quint8 ret_value = receivedData.at( blength - 1 );
        out << "Request for SetRegister " << ( ret_value ? "completed successfully" : "fail" ) << endl;
    }
    else
    {
        out << QStringLiteral( "Request for SetRegister failed" );
    }
    emit getData( output.toLocal8Bit() );
}

//!
//! \brief RTC::statusRequest
//! \return
//!
bool RTC::statusRequest()
{
    bool result = false;
    // Send a request to the RTC device
    QByteArray receivedData = sendRequest( Request::STATUS );

    QString output;
    QTextStream out( &output );
    // Check of the received response to the request
    if ( receivedData.size() == 1 )
    {
        StatusMessages status = static_cast<StatusMessages>( receivedData.at(0) );
        switch ( status ) {
        case StatusMessages::STATUS_SUCCESS:
            result =  true;
            break;
        case StatusMessages::STATUS_ERROR:
            out << QStringLiteral( "Processing the data failed" );
            break;
        case StatusMessages::STATUS_INVALID_PARAMETER:
            out << QStringLiteral( "Received parameter(s) are invalid" );
            break;
        case StatusMessages::STATUS_INPUT_DATA_TOLONG:
            out << QStringLiteral( "Input data too long" );
            break;
        case StatusMessages::STATUS_NOT_SUPPORTED:
            out << QStringLiteral( "The state of the device is undefined" );
            break;
        case StatusMessages::STATUS_UNKNOWN_ERROR:
            out << QStringLiteral( "Unexpected error" );
            break;
        case StatusMessages::STATUS_DISCONNECTION:
            out << QStringLiteral( "No confirmation of connection received" );
            break;
        default:
            out << receivedData.toHex() << endl;
        }
    }
    else
    {
        // Not received a response to the device status request.
        out << QStringLiteral( "Status Request failed" ) << endl;
        emit portError( QStringLiteral( "Another device is connected to the RTC serial port! " ) + m_pSerialPort->errorString() );
        m_pSerialPort->blockSignals( true );
    }
    if ( !result ) emit getData( output.toLocal8Bit() );
    return result;
}

