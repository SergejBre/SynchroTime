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

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include "rtc.h"
#include <QDebug>

//------------------------------------------------------------------------------
// Preprocessor
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Enums
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
#define STARTBYTE '@';  //!< Start byte of the protocol.
#define DEVICE_ID 0x00; //!< ID of the RTC device.

//------------------------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------------------------

//!
//! \brief RTC::RTC
//! \param portName
//! \param parent
//!
RTC::RTC( const QString & portName, QObject *parent )
    : QObject( parent )
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

    // Connect the serial port.
    connectToRTC();
    m_pTimerCheckConnection->start();
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
void RTC::setRegisterRequestSlot()
{
    if ( isConnected() )
    {
        setRegisterRequest();
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
//! \brief RTC::connectToRTC
//!
void RTC::connectToRTC()
{
    if ( m_pSerialPort->open( QSerialPort::ReadWrite ) )
    {
        // Make sure that the RTC device is connected to the serial port.
        m_isConnected = statusRequest();

        if ( m_isConnected )
        {
            qDebug() << "The RTC is connected.";
        }
        else
        {
            qDebug() << "Another device is connected to the RTC serial port!";
        }
    }
    else
    {
        qDebug() << "The serial port is not connected.";
        m_isConnected = false;
    }
}

//!
//! \brief RTC::setProtocol
//! \param protocolData
//! \param request
//! \param size
//! \param data
//!
void RTC::setProtocol( QByteArray &protocolData, Request request, quint8 size, quint8 const* data )
{
    // Data that are sent to the serial interface.
    protocolData.resize(size + 4);
    protocolData[0] = STARTBYTE;
//    protocolData[1] = DEVICE_ID;

    // Checksum = the sum of all bytes starting from the request command.
    quint8 crc = 0;
    crc += protocolData[1] = static_cast<quint8>( request );
    crc += protocolData[2] = size;

    // If there are data bytes, then they are also added to the checksum.
    if ( size > 0 && data != nullptr )
    {
        int i;
        for ( i = 0; i < size; ++i)
        {
            crc += protocolData[i + 3] = data[i];
        }
    }

    // The last byte is the checksum.
    protocolData[size + 3] = crc;
}

//!
//! \brief RTC::informationRequest
//!
void RTC::informationRequest()
{
    QByteArray requestForInformation;
    setProtocol( requestForInformation, Request::INFO );
    emit getData( requestForInformation );
    //! \todo
}

//!
//! \brief RTC::adjustmentRequest
//!
void RTC::adjustmentRequest()
{
    QByteArray requestForAdjustment;
    setProtocol( requestForAdjustment, Request::ADJUST );
    emit getData( requestForAdjustment );
    //! \todo
}

//!
//! \brief RTC::calibrationRequest
//!
void RTC::calibrationRequest()
{
    QByteArray requestForCalibration;
    setProtocol( requestForCalibration, Request::CALIBR );
    emit getData( requestForCalibration );
    //! \todo
}

//!
//! \brief RTC::resetRequest
//!
void RTC::resetRequest()
{
    QByteArray requestForReset;
    setProtocol( requestForReset, Request::RESET );
    emit getData( requestForReset );
    //! \todo
}

//!
//! \brief RTC::setRegisterRequest
//!
void RTC::setRegisterRequest()
{
    QByteArray requestForSetRegister;
    setProtocol( requestForSetRegister, Request::SETREG );
    emit getData( requestForSetRegister );
    //! \todo
}

//!
//! \brief RTC::statusRequest
//! \return
//!
bool RTC::statusRequest()
{
    QByteArray requestForStatus;
    setProtocol( requestForStatus, Request::STATUS );
    qDebug() << "status request";
    //! \todo

    return true;
}

