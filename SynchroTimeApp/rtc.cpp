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

//------------------------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------------------------

//!
//! \brief RTC::RTC
//! \param parent
//!
RTC::RTC( QObject *parent )
    : QObject( parent )
{
    // Initialization of the serial interface.
    m_pSerialPort = new QSerialPort( this );
    m_pSerialPort->setPortName("ttyUSB0");
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
//! \brief RTC::informationRequest
//!
void RTC::informationRequest()
{
    //! \todo
}

//!
//! \brief RTC::adjustmentRequest
//!
void RTC::adjustmentRequest()
{
    //! \todo
}

//!
//! \brief RTC::calibrationRequest
//!
void RTC::calibrationRequest()
{
    //! \todo
}

//!
//! \brief RTC::resetRequest
//!
void RTC::resetRequest()
{
    //! \todo
}

//!
//! \brief RTC::setRegisterRequest
//!
void RTC::setRegisterRequest()
{
    //! \todo
}

//!
//! \brief RTC::statusRequest
//! \return
//!
bool RTC::statusRequest()
{
    qDebug() << "status request";
    //! \todo

    return true;
}

