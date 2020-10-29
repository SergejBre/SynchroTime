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
RTC::RTC( const QString & portName, QObject *parent )
    : QObject( parent ),
      m_pSerialPort( nullptr ),
      m_pTimerCheckConnection( nullptr )
{
    // Initialization of the serial interface.
    m_pSerialPort = new QSerialPort( this );
    m_pSerialPort->setPortName( portName );
    // Data transfer rate: 115200 бит/с.
    m_pSerialPort->setBaudRate(QSerialPort::Baud115200);
    m_pSerialPort->setDataBits(QSerialPort::Data8);
    m_pSerialPort->setParity(QSerialPort::NoParity);
    m_pSerialPort->setStopBits(QSerialPort::OneStop);
    m_pSerialPort->setFlowControl(QSerialPort::NoFlowControl);

    // Create a timer with 1 second intervals.
    m_pTimerCheckConnection = new QTimer( this );
    m_pTimerCheckConnection->setInterval( 1000 );

    // After a time of 1 s, the statusRequest() command is called.
    // This is where the lambda function is used to avoid creating a slot.
    QObject::connect( m_pTimerCheckConnection, &QTimer::timeout, [this]() {
        statusRequest();
    });
    QObject::connect( m_pSerialPort, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
                      this, &RTC::handleError );

    // Connect the serial port.
    connectToRTC();
    m_pTimerCheckConnection->start();
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
    m_pSerialPort = new QSerialPort( this );
    m_pSerialPort->setPortName( portSettings.name );
    m_pSerialPort->setBaudRate( portSettings.baudRate );
    m_pSerialPort->setDataBits( portSettings.dataBits );
    m_pSerialPort->setParity( portSettings.parity );
    m_pSerialPort->setStopBits( portSettings.stopBits );
    m_pSerialPort->setFlowControl( portSettings.flowControl );

    // Create a timer with 1 second intervals.
    m_pTimerCheckConnection = new QTimer( this );
    m_pTimerCheckConnection->setInterval( 1000 );

    // After a time of 1 s, the statusRequest() command is called.
    // This is where the lambda function is used to avoid creating a slot.
    QObject::connect( m_pTimerCheckConnection, &QTimer::timeout, [this]() {
        statusRequest();
    });
    QObject::connect( m_pSerialPort, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
                      this, &RTC::handleError );

    // Open the serial port.
    m_isConnected = openSerialPort();
    m_pTimerCheckConnection->start();
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
    if ( isConnected() )
    {
        informationRequest();
    }
}

//!
//! \brief RTC::adjustmentRequestSlot
//!
void RTC::adjustmentRequestSlot()
{
    if ( isConnected() )
    {
        adjustmentRequest();
    }
}

//!
//! \brief RTC::calibrationRequestSlot
//!
void RTC::calibrationRequestSlot()
{
    if ( isConnected() )
    {
        calibrationRequest();
    }
}

//!
//! \brief RTC::resetRequestSlot
//!
void RTC::resetRequestSlot()
{
    if ( isConnected() )
    {
        resetRequest();
    }
}

//!
//! \brief RTC::setRegisterRequestSlot
//!
void RTC::setRegisterRequestSlot( const float newValue )
{
    if ( isConnected() )
    {
        setRegisterRequest( newValue );
    }
}

//!
//! \brief RTC::versionRequestSlot
//!
void RTC::statusRequestSlot()
{
    if ( isConnected() )
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
bool RTC::openSerialPort()
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
//! \param protocolData
//! \param request
//! \param size
//! \param data
//! \return
//!
QByteArray RTC::sendRequest( QByteArray &protocolData, Request request, quint8 size, const quint8 *data )
{
    // Data that are sent to the serial interface.
//    protocolData.resize(size + 4);
    protocolData.resize(size + 2);
    protocolData[0] = STARTBYTE;
//    protocolData[1] = DEVICE_ID;

    // Checksum = the sum of all bytes starting from the request command.
    quint8 crc = 0;
    crc += protocolData[1] = static_cast<quint8>( request );
//    crc += protocolData[2] = size;

    // If there are data bytes, then they are also added to the checksum.
    if ( size > 0 && data != nullptr )
    {
        int i;
        for ( i = 0; i < size; ++i)
        {
//            crc += protocolData[i + 3] = data[i];
            crc += protocolData[i + 2] = data[i];
        }
    }

    // The last byte is the checksum.
//    protocolData[size + 3] = crc;

    Q_ASSERT( m_pSerialPort->isOpen() );
    // Send data to the serial port and wait up to 50 ms until the write is done.
    m_pSerialPort->write( protocolData );
    m_pSerialPort->waitForBytesWritten( TIME_WAIT );

    // Sleep 50 ms, waiting for the microcontroller to process the data and respond.
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
    QString output;
    QTextStream out( &output );
    QByteArray requestForInformation;

    quint8 sendData[6];
    const QDateTime local(QDateTime::currentDateTime());
    const qint64 localTimeMSecs = local.toMSecsSinceEpoch();
    const quint32 localTimeSecs = localTimeMSecs/1000;
    const quint16 milliSecs = localTimeMSecs - localTimeSecs * 1000;
    memcpy( sendData, &localTimeSecs, sizeof(localTimeSecs) );
    memcpy( sendData + 4, &milliSecs, sizeof(milliSecs) );

    QByteArray receivedData = sendRequest( requestForInformation, Request::INFO, sizeof( sendData ), sendData );

    out << requestForInformation.left(2) << endl;
    const uint8_t blength = receivedData.size();
    // Check of the received response to the request.
    if ( blength > 5 )
    {
        qint64 numberOfMSec = 0LL;
        quint16 numberOfSec = 0U;
        auto byteBuffer = receivedData.constData();
        memcpy( &numberOfMSec, byteBuffer, sizeof( quint32 ) );
        numberOfMSec *= 1000;
        memcpy( &numberOfSec, byteBuffer + 4, sizeof( numberOfSec ) );
        numberOfMSec += numberOfSec;
        QDateTime time( QDateTime::fromMSecsSinceEpoch( numberOfMSec ) );
        out << "Time from DS3231 \t" << numberOfMSec << " ms: " << time.toString("dd.MM.yyyy hh:mm:ss.zzz") << endl;
        out << "System local time\t" << localTimeMSecs << " ms: " << local.toString("dd.MM.yyyy hh:mm:ss.zzz") << endl;
        out << "Difference between\t" << numberOfMSec - localTimeMSecs << " ms" << endl;
        if ( blength > 6 ) {
            const float offset_reg = float( byteBuffer[6] )/10;
            out << "Offset reg. in ppm\t" << offset_reg << " ppm" << endl;
            if ( blength > 10 ) {
                float drift_in_ppm = 0;
                memcpy( &drift_in_ppm, byteBuffer + 7, sizeof( drift_in_ppm ) );
                out << "Time drift in ppm\t" << drift_in_ppm << " ppm" << endl;
                if ( blength > 14 ) {
                    qint64 lastAdjustOfTimeMSec = 0LL;
                    memcpy( &lastAdjustOfTimeMSec, byteBuffer + 11, sizeof( quint32 ) );
                    if ( lastAdjustOfTimeMSec < 0xFFFFFFFF ) {
                        lastAdjustOfTimeMSec *= 1000;
                        time = QDateTime::fromMSecsSinceEpoch( lastAdjustOfTimeMSec );
                        out << "LastTime adjustment\t" << lastAdjustOfTimeMSec << " ms: " << time.toString("dd.MM.yyyy hh:mm:ss.zzz") << endl;
//                        out << "Time drift in ppm\t" << float(numberOfMSec - localTimeMSecs)*1000000/(localTimeMSecs - lastAdjustOfTimeMSec ) << " ppm" << endl;
                    }
                }
            }
        }
    }
    emit getData( output.toLocal8Bit() );
}

//!
//! \brief RTC::adjustmentRequest
//!
void RTC::adjustmentRequest()
{
    QByteArray requestForAdjustment;
    sendRequest( requestForAdjustment, Request::ADJUST );
    emit getData( requestForAdjustment );
    //! \todo
}

//!
//! \brief RTC::calibrationRequest
//!
void RTC::calibrationRequest()
{
    QByteArray requestForCalibration;
    sendRequest( requestForCalibration, Request::CALIBR );
    emit getData( requestForCalibration );
    //! \todo
}

//!
//! \brief RTC::resetRequest
//!
void RTC::resetRequest()
{
    QByteArray requestForReset;
    sendRequest( requestForReset, Request::RESET );
    emit getData( requestForReset );
    //! \todo
}

//!
//! \brief RTC::setRegisterRequest
//!
void RTC::setRegisterRequest( const float newValue )
{
    QByteArray requestForSetRegister;
    sendRequest( requestForSetRegister, Request::SETREG );
    emit getData( requestForSetRegister );
    qDebug() << newValue;
    //! \todo
}

//!
//! \brief RTC::statusRequest
//! \return
//!
bool RTC::statusRequest()
{
    QByteArray requestForStatus;
    QByteArray receivedData = sendRequest( requestForStatus, Request::STATUS );

    if ( receivedData.size() == 1 && receivedData.at(0) == 0x00 )
    {
        return true;
    }
    else
    {
        qDebug() << QStringLiteral( "Status Request failed" );
        emit portError( QStringLiteral( "Another device is connected to the RTC serial port! " ) + m_pSerialPort->errorString() );
        m_pSerialPort->blockSignals( true );
    }
    //! \todo

    return false;
}

