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
#include "serialportsettings.h"
#include <QDebug>
#include <QThread>
#include <QDateTime>
#include <QElapsedTimer>
#include <QTimer>
#include <QtEndian>
#include <cmath>

//------------------------------------------------------------------------------
// Preprocessor
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Enums
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
Q_DECLARE_METATYPE(QSerialPort::SerialPortError)
#define STARTBYTE 0x40   //!< The starting byte of the data set from the communication protocol.
#define DEVICE_ID 0x00   //!< ID of the RTC device.
#define WAIT_REBOOT 2500 //!< Interval for a time-out in milliseconds
#define WAIT_TIME 100    //!< The waiting time for the reading of data in milliseconds.
#define ESC_RED QStringLiteral("\x1b[31m")    //!< ESCAPE sequence for red.
#define ESC_YELLOW QStringLiteral("\x1b[33m") //!< ESCAPE sequence for yellow.
#define ESC_BLUE QStringLiteral("\x1b[36m")   //!< ESCAPE sequence for blue.
#define ESC_WHITE QStringLiteral("\x1b[37m")  //!< ESCAPE sequence for white.
#define ESC_RESET QStringLiteral("\x1b[0m")   //!< Reset color.

//------------------------------------------------------------------------------
// Function definitions
//------------------------------------------------------------------------------

//! \brief RTC::RTC
//! Constructor for test purposes.
//! \param portName of the type const QString. Name of the port in the system.
//! \param parent of the type *QObject - pThread.
//! \note There is no need to specify the parent.
//! The parent will be a thread when we move our RTC object into it.
RTC::RTC( const QString &portName, QObject *parent )
    : QObject( parent ),
      m_pSerialPort( nullptr ),
      m_isBusy( false ),
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

        QObject::connect( m_pSerialPort, SIGNAL( error( QSerialPort::SerialPortError )),
                          this, SLOT( handleError( QSerialPort::SerialPortError )));
        // Connect the serial port.
        connectToRTC();

        // Create a timer with 1 second intervals.
        m_pTimerCheckConnection = ::new( std::nothrow ) QTimer( this );
        if ( m_pTimerCheckConnection != nullptr ) {
            m_pTimerCheckConnection->setInterval( 500 );

            // After a time of 1 s, the statusRequest() command is called.
            // This is where the lambda function is used to avoid creating a slot.
            QObject::connect( m_pTimerCheckConnection, &QTimer::timeout, [this]() {
                statusRequestSlot();
            });
            m_pTimerCheckConnection->start();
        }
    }
}

//! \brief RTC::RTC
//! Default Constructor.
//! \param portSettings of the type const Settings *const. Reference to an parameters of the serial port.
//! \param parent of the type QObject* - pThread.
//! \note There is no need to specify the parent.
//! The parent will be a thread when we move our RTC object into it.
RTC::RTC( const Settings *const portSettings, QObject *parent )
    : QObject( parent ),
      m_pSerialPort( nullptr ),
      m_isConnected( false ),
      m_isBusy( false ),
      m_isDetectDelayEnabled( false ),
      m_pTimerCheckConnection( nullptr ),
      m_correctionFactor( portSettings->correctionFactor )
{
    // Initialization of the serial interface.
    m_pSerialPort = ::new( std::nothrow ) QSerialPort( this );
    if ( m_pSerialPort != nullptr ) {
        m_pSerialPort->setPortName( portSettings->name );
        m_pSerialPort->setBaudRate( portSettings->baudRate );
        m_pSerialPort->setDataBits( portSettings->dataBits );
        m_pSerialPort->setParity( portSettings->parity );
        m_pSerialPort->setStopBits( portSettings->stopBits );
        m_pSerialPort->setFlowControl( portSettings->flowControl );

        qRegisterMetaType<QSerialPort::SerialPortError>( "QSerialPort::SerialPortError" );
        QObject::connect( m_pSerialPort, SIGNAL( error( QSerialPort::SerialPortError )),
                          this, SLOT( handleError( QSerialPort::SerialPortError )));
        // Open the serial port.
        m_isConnected = openSerialPort();

        // Create a timer with 1 second intervals.
        m_pTimerCheckConnection = ::new( std::nothrow ) QTimer( this );
        if ( m_pTimerCheckConnection != nullptr && portSettings->statusControlEnabled ) {
            m_isDetectDelayEnabled = portSettings->detectDelayEnabled;
            m_pTimerCheckConnection->setInterval( portSettings->requestRate );

            // After a time of 1 s, the statusRequest() command is called.
            // This is where the lambda function is used to avoid creating a slot.
            QObject::connect( m_pTimerCheckConnection, &QTimer::timeout, [this]() {
                statusRequestSlot();
            });
            m_pTimerCheckConnection->start();
        }
    }
}

//!
//! \brief RTC::~RTC
//! Default destructor
RTC::~RTC()
{
    if ( m_pTimerCheckConnection != nullptr ) {
        delete m_pTimerCheckConnection;
    }
    if ( m_pSerialPort != nullptr ) {
        if ( m_pSerialPort->isOpen() ) {
            m_pSerialPort->close();
        }
        delete m_pSerialPort;
    }
}

//!
//! \brief RTC::isConnected
//! \return True if the RTC device is connected, otherwise false.
//!
bool RTC::isConnected() const
{
    return m_isConnected;
}

//!
//! \brief RTC::isBusy
//! \return  True if the Serial Port is busy, otherwise false.
//!
bool RTC::isBusy() const
{
    return m_isBusy;
}

//!
//! \brief RTC::informationRequestSlot
//!
void RTC::informationRequestSlot()
{
    if ( m_isConnected && !m_isBusy )
    {
        informationRequest();
    }
}

//!
//! \brief RTC::adjustmentRequestSlot
//!
void RTC::adjustmentRequestSlot()
{
    if ( m_isConnected && !m_isBusy )
    {
        adjustmentRequest();
    }
}

//!
//! \brief RTC::calibrationRequestSlot
//!
void RTC::calibrationRequestSlot()
{
    if ( m_isConnected && !m_isBusy )
    {
        calibrationRequest();
    }
}

//!
//! \brief RTC::resetRequestSlot
//!
void RTC::resetRequestSlot()
{
    if ( m_isConnected && !m_isBusy )
    {
        resetRequest();
    }
}

//!
//! \brief RTC::setRegisterRequestSlot
//! \param newValue of the type const float.
//!
void RTC::setRegisterRequestSlot( const float newValue )
{
    if ( m_isConnected && !m_isBusy )
    {
        setRegisterRequest( newValue );
    }
}

//!
//! \brief RTC::versionRequestSlot
//!
void RTC::statusRequestSlot()
{
    if ( m_isConnected && !m_isBusy )
    {
        statusRequest();
    }
}

//!
//! \brief RTC::infoFromDevice
//!
void RTC::infoFromDevice()
{
    Q_ASSERT( m_pSerialPort != nullptr );
    if ( m_isConnected && !m_isBusy )
    {
        m_pSerialPort->waitForReadyRead( WAIT_TIME );
        emit getData( ESC_WHITE + m_pSerialPort->readAll() + ESC_RESET );
    }
}

//!
//! \brief RTC::handleError
//! \param error of the type enum QSerialPort::SerialPortError.
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
//! \return True if the serial port is open, otherwise false.
//!
bool RTC::openSerialPort() const
{
    Q_ASSERT( m_pSerialPort != nullptr );
    if ( m_pSerialPort->open( QSerialPort::ReadWrite ) && m_pSerialPort->clear( QSerialPort::AllDirections ) )
    {
        thread()->msleep( WAIT_REBOOT );
        return true;
    }
    else
    {
        qCritical() << QStringLiteral( "Failed to open the serial interface" );
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
        thread()->msleep( WAIT_REBOOT );
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
//! \param request of the type enum Request.
//! \param size of the type quint8.
//! \param data Sent data of type byte array.
//! \return Data array of the type const QByteArray.
//!
const QByteArray RTC::sendRequest( Request request, quint8 size, const quint8 *const data )
{
    Q_ASSERT( m_pSerialPort->isOpen() );
    m_isBusy = true;

    // Data that are sent to the serial port.
    QByteArray sentData;
    sentData.resize(size + 3);
    sentData[0] = STARTBYTE;
//    sentData[1] = DEVICE_ID;

    // Checksum = the sum of all bytes starting from the request command.
    quint8 crc = 0;
    crc += sentData[1] = static_cast<quint8>( request );

    // If there are data bytes, then they are also added to the checksum.
    if ( size > 0 && data != nullptr )
    {
        for ( quint8 i = 0U; i < size; ++i )
        {
            crc += sentData[i + 2] = data[i];
        }
    }

    // The last byte is the checksum.
    sentData[size + 2] = crc;

    // Send data to the serial port
    m_pSerialPort->write( sentData );
    bool ready = m_pSerialPort->waitForBytesWritten( WAIT_TIME );

    // Reading data from RTC.
    if ( ready ) {
        ready = m_pSerialPort->waitForReadyRead( WAIT_TIME );
        if ( !ready ) {
            qCritical() << QStringLiteral( "Failed to received a response from device: " ) + m_pSerialPort->errorString();
        }
    }
    else {
        qCritical() << QStringLiteral( "Failed to send data to device: " ) + m_pSerialPort->errorString();
    }
    m_isBusy = false;
    return m_pSerialPort->readAll();
}

//!
//! \brief RTC::informationRequest
//!
void RTC::informationRequest()
{
    QString output;
    QTextStream out( &output );
    out << ESC_WHITE << QStringLiteral( "Information from Device:" ) << ESC_RESET;

    // Data that are sent to the serial interface.
    quint8 sentData[6];
    const QDateTime local = QDateTime::currentDateTime();
    const qint64 localTimeMSecs = local.toMSecsSinceEpoch();
    {
        quint32 localTimeSecs = localTimeMSecs/1000;
        quint16 milliSecs = localTimeMSecs % 1000;
        localTimeSecs = qToLittleEndian<quint32>( localTimeSecs );
        memcpy( sentData, &localTimeSecs, sizeof(localTimeSecs) );
        milliSecs = qToLittleEndian<quint16>( milliSecs );
        memcpy( sentData + sizeof(localTimeSecs), &milliSecs, sizeof(milliSecs) );
    }
    // Send a request to the RTC device
    auto receivedData = sendRequest( Request::INFO, sizeof( sentData ), sentData );
    const qint8 blength = receivedData.size();

    // Check of the received response to the request
    if ( blength > 6 && receivedData.at(0) == STARTBYTE && checkCRC( receivedData ) )
    {
        qint64 numberOfMSec = 0LL;
        quint16 numberOfSec = 0U;
        auto p_byteBuffer = receivedData.constData();
        memcpy( &numberOfMSec, p_byteBuffer + 1, sizeof( quint32 ) );
        numberOfMSec = qFromLittleEndian( numberOfMSec );
        numberOfMSec *= 1000;
        memcpy( &numberOfSec, p_byteBuffer + 5, sizeof( numberOfSec ) );
        numberOfSec = qFromLittleEndian( numberOfSec );
        numberOfMSec += numberOfSec;
        QDateTime time( QDateTime::fromMSecsSinceEpoch( numberOfMSec ) );
        out << "Time from Device  \t" << numberOfMSec << " ms: " << time.toString("dd.MM.yyyy hh:mm:ss.zzz") << endl;
        out << "Local system time \t" << localTimeMSecs << " ms: " << local.toString("dd.MM.yyyy hh:mm:ss.zzz") << endl;
        out << "Difference between\t" << numberOfMSec - localTimeMSecs << " ms" << endl;
        if ( blength > 7 ) {
            const float offset_reg = static_cast<float>( p_byteBuffer[7] )/10;
            out << "Offset reg. value \t" << offset_reg << " ppm" << endl;
            if ( blength > 11 ) {
                float drift_in_ppm = 0;
                memcpy( &drift_in_ppm, p_byteBuffer + 8, sizeof( drift_in_ppm ) );
                out << "Frequency drift   \t" << drift_in_ppm << " ppm" << endl;
                out << "Corrected value***\t" << std::abs(m_correctionFactor) * drift_in_ppm/10 << " ppm for correction faktor " << m_correctionFactor << endl;
                if ( blength > 15 ) {
                    quint32 lastAdjustOfTimeSec = 0L;
                    memcpy( &lastAdjustOfTimeSec, p_byteBuffer + 12, sizeof( lastAdjustOfTimeSec ) );
                    lastAdjustOfTimeSec = qFromLittleEndian( lastAdjustOfTimeSec );
                    if ( lastAdjustOfTimeSec < 0xFFFFFFFF ) {
                        const qint64 lastAdjustOfTimeMSec = qint64(lastAdjustOfTimeSec) * 1000;
                        time = QDateTime::fromMSecsSinceEpoch( lastAdjustOfTimeMSec );
//                        out << "Frequency drift*  \t" << static_cast<float>(numberOfMSec - localTimeMSecs)*1000000/(localTimeMSecs - lastAdjustOfTimeMSec ) << " ppm" << endl;
                        out << "Last adjustm. time\t" << lastAdjustOfTimeMSec << " ms: " << time.toString("dd.MM.yyyy hh:mm:ss.zzz") << endl;
                    }
                }
            }
        }
    }
    else
    {
        out << ESC_RED << QStringLiteral( "Request for the information failed. " ) << receivedData << ESC_RESET;
    }
    emit getData( output );
}

//!
//! \brief RTC::adjustmentRequest
//!
void RTC::adjustmentRequest()
{
    QString output;
    QTextStream out( &output );
    out << ESC_WHITE << QStringLiteral( "Adjust clock Date/Time.." ) << ESC_RESET;

    // Data that are sent to the serial interface.
    quint8 sentData[6];
    const qint64 localTimeMSecs = QDateTime::currentDateTime().toMSecsSinceEpoch(); //local.toMSecsSinceEpoch();
    quint32 localTimeSecs = localTimeMSecs/1000;
    const quint16 milliSecs = localTimeMSecs % 1000;
    quint32 le_localTimeSecs = qToLittleEndian<quint32>( ++localTimeSecs );
    memcpy( sentData, &le_localTimeSecs, sizeof(le_localTimeSecs) );
    sentData[4] = sentData[5] = 0U;

    thread()->msleep( 1000UL - milliSecs );

    // Send a request to the RTC device
    auto receivedData = sendRequest( Request::ADJUST, sizeof( sentData ), sentData );

    // Check of the received response to the request.
    if ( !receivedData.isEmpty() && receivedData.at(0) == STARTBYTE && checkCRC( receivedData ) )
    {
        const qint8 blength = receivedData.size();
        const quint8 ret_value = receivedData.at( blength - 1 );
        QDateTime local; //(QDateTime::currentDateTime());
        local.setTime_t( localTimeSecs );
        out << "Local system time \t" << local.toString( "ddd d MMM yyyy hh:mm:ss.zzz" );
        out << ESC_YELLOW << "Request for adjustment " << ( ret_value ? "completed successfully" : "failed" ) << ESC_RESET;
    }
    else
    {
        out << ESC_RED << QStringLiteral( "Request for adjustment failed. " ) << receivedData << ESC_RESET;
    }
    emit getData( output );
}

//!
//! \brief RTC::calibrationRequest
//!
void RTC::calibrationRequest()
{
    QString output;
    QTextStream out( &output );
    out << ESC_WHITE << QStringLiteral( "Calibration the Oscillator.." ) << ESC_RESET;

    // Data that are sent to the serial interface.
    quint8 sentData[6];
    QDateTime local( QDateTime::currentDateTime() );
    const qint64 localTimeMSecs = local.toMSecsSinceEpoch();
    quint32 localTimeSecs = localTimeMSecs/1000;
    const quint16 milliSecs = localTimeMSecs % 1000;
    quint32 le_localTimeSecs = qToLittleEndian<quint32>( ++localTimeSecs );
    memcpy( sentData, &le_localTimeSecs, sizeof( le_localTimeSecs ) );
    sentData[4] = sentData[5] = 0U;

    thread()->msleep( 1000UL - milliSecs );

    // Send a request to the RTC device
    auto receivedData = sendRequest( Request::CALIBR, sizeof( sentData ), sentData );

    // Check of the received response to the request
    if ( !receivedData.isEmpty() && receivedData.at(0) == STARTBYTE && checkCRC( receivedData ) )
    {
        const qint8 blength = receivedData.size();
        auto byteBuffer = receivedData.constData();
        const quint8 ret_value = byteBuffer[ blength-1 ];
        local.setTime_t( localTimeSecs );
        out << "Local system time \t" << local.toString( "ddd d MMM yyyy hh:mm:ss.zzz" ) << endl;
        qint8 offset_reg = byteBuffer[1];
        out << "Offset last value \t" << offset_reg << endl;
        if ( blength > 6 ) {
            float drift_in_ppm = 0;
            memcpy( &drift_in_ppm, byteBuffer + 2, sizeof( drift_in_ppm ) );
            out << "Frequency drift   \t" << drift_in_ppm << " ppm" << endl;
            offset_reg = byteBuffer[6];
            out << "Offset new value  \t" << offset_reg;
        }
        out << ESC_YELLOW << "Request for calibration " << ( ret_value ? "completed successfully" : "failed" ) << ESC_RESET;
    }
    else
    {
        out << ESC_RED << QStringLiteral( "Request for calibration failed. " ) << receivedData << ESC_RESET;
    }
    emit getData( output );
}

//!
//! \brief RTC::resetRequest
//!
void RTC::resetRequest()
{
    QString output;
    QTextStream out( &output );
    out << ESC_WHITE << QStringLiteral( "Reset the calibration Parameters.." ) << ESC_RESET;

    // Send a request to the RTC device
    auto receivedData = sendRequest( Request::RESET );

    // Check of the received response to the request
    if ( !receivedData.isEmpty() && receivedData.at(0) == STARTBYTE && checkCRC( receivedData ) )
    {
        const qint8 blength = receivedData.size();
        const quint8 ret_value = receivedData.at( blength - 1 );
        out << ESC_YELLOW << "Request for reset " << ( ret_value ? "completed successfully" : "failed" ) << ESC_RESET;
    }
    else
    {
        out << ESC_RED << QStringLiteral( "Request for reset failed. " ) << receivedData << ESC_RESET;
    }
    emit getData( output );
}

//! \brief RTC::setRegisterRequest
//! \param newValue of the type const float. The new Value for Offset Register.
void RTC::setRegisterRequest( const float newValue )
{
    QString output;
    QTextStream out( &output );
    out << ESC_WHITE << QStringLiteral( "Set register Value to " ) << newValue << ESC_RESET;

    // Data that are sent to the serial interface.
    quint8 sentData[4];
    memcpy( sentData, &newValue, sizeof( newValue ) );

    // Send a request to the RTC device
    auto receivedData = sendRequest( Request::SETREG, sizeof( sentData ), sentData );

    // Check of the Received request
    if ( !receivedData.isEmpty() && receivedData.at(0) == STARTBYTE && checkCRC( receivedData ) )
    {
        const qint8 blength = receivedData.size();
        const quint8 ret_value = receivedData.at( blength - 1 );
        out << ESC_YELLOW << "Request for SetRegister " << ( ret_value ? "completed successfully" : "failed" ) << ESC_RESET;
    }
    else
    {
        out << ESC_RED << QStringLiteral( "Request for SetRegister failed. " ) << receivedData << ESC_RESET;
    }
    emit getData( output );
}

//!
//! \brief RTC::statusRequest
//! \return True if the RTC status was successful, otherwise false.
//!
bool RTC::statusRequest()
{
    QString output;
    QTextStream out( &output );
    bool result = false;
    QElapsedTimer m_eTimer;
    m_eTimer.start();

    // Send a request to the RTC device
    auto receivedData = sendRequest( Request::STATUS );
    const float delay = m_eTimer.elapsed();

    // Check of the received response to the request
    if ( !receivedData.isEmpty() && receivedData.at(0) == STARTBYTE && checkCRC( receivedData ) )
    {
        StatusMessages status = static_cast<StatusMessages>( receivedData.at(1) );
        switch ( status ) {
        case StatusMessages::STATUS_SUCCESS:
            result =  true;
            break;
        case StatusMessages::STATUS_ERROR:
            out << QStringLiteral( "Processing the data failed" );
            break;
        case StatusMessages::STATUS_INVALID_PARAMETER_:
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
        out << ESC_RED << QStringLiteral( "Status Request failed. " ) << receivedData << ESC_RESET;
        emit portError( QStringLiteral( "Not received a response to the device status request: " ) + m_pSerialPort->errorString() );
    }
    if ( !result ) emit getData( output );
    else if ( m_isDetectDelayEnabled ) emit getDelay( delay );
    return result;
}

//! \brief RTC::sumOfBytes
//!
//! \param bufferArray of the type QByteArray
//!
//! \return crc a checksum of the type quint8 (1 byte)
//!
quint8 RTC::sumOfBytes( const QByteArray &bufferArray ) const
{
    Q_ASSERT( bufferArray  != nullptr );
    quint8 crc = 0U;
    if ( !bufferArray.isEmpty() )
    {
        for ( uint idx = 0U; idx < bufferArray.size() - 1U; ++idx )
        {
            crc += static_cast<quint8>( bufferArray.at(idx) );
        }
    }
    return crc;
}

//! \brief RTC::checkCRC
//!
//! The function compares a calculated and a received hash sums.
//! The received hash sum is in the last place in the array.
//!
//! \param bufferArray of the type QByteArray
//!
//! \retval true if the hash sums match, otherwise false
//! \retval false
//!
bool RTC::checkCRC( const QByteArray &bufferArray ) const
{
    Q_ASSERT( bufferArray  != nullptr );
    if ( bufferArray.isEmpty() )
    {
        return false;
    }
    else
    {
        int last = bufferArray.size() - 1;
        return ( sumOfBytes( bufferArray ) == static_cast<quint8>( bufferArray.at(last)) );
    }
}

