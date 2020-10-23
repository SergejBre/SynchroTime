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
#ifndef RTC_H
#define RTC_H

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <QObject>
#include <QTimer>
#include <QSerialPort>
#include "serialportsettings.h"

//------------------------------------------------------------------------------
// Preprocessor
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Enums
//------------------------------------------------------------------------------
//!
//! \enum Request
//!
//! \brief The Request enum
//!
//! \details
//! Requests available to us sent by RTC. Sent as the third byte in the protocol.
//!
enum class Request : quint8
{
    INFO   = 'i', //!< Information request.
    ADJUST = 'a', //!< Adjustment request.
    CALIBR = 'c', //!< Calibration request.
    RESET  = 'r', //!< Reset request.
    SETREG = 's', //!< Set register request.
    STATUS = 't'  //!< Status request.
};

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

//!
//! \class RTC
//!
//! \brief The RTC class
//!
class RTC : public QObject
{
    Q_OBJECT

public:
    explicit RTC( const QString & portName, QObject *parent = 0 );
    explicit RTC( const Settings_t &portSettings, QObject *parent = 0 );
    ~RTC();

    // Connection check function.
    bool isConnected() const;

signals:
    void getData( const QByteArray &data );
    void portError( const QString &error );

public slots:
    // Information request slot.
    void informationRequestSlot();
    // Adjustment request slot.
    void adjustmentRequestSlot();
    // Calibration request slot.
    void calibrationRequestSlot();
    // Reset request slot.
    void resetRequestSlot();
    // Set register request slot.
    void setRegisterRequestSlot( const float newValue );
    // Status request slot.
    void statusRequestSlot();

private slots:
    void handleError( QSerialPort::SerialPortError error );

private:
    enum class StatusMessages: quint8;
    // The connection function.
    void connectToRTC();
    // The function realizing protocol with RTC.
    void setProtocol( QByteArray &protocolData, Request request, quint8 size = 0, quint8 const* data = nullptr );
    // Information request.
    void informationRequest();
    // Adjustment request.
    void adjustmentRequest();
    // Calibration request.
    void calibrationRequest();
    // Reset request.
    void resetRequest();
    // Set register request.
    void setRegisterRequest( const float newValue );
    // Status request.
    bool statusRequest();

    // RTC status messages.
    enum class StatusMessages : quint8
    {
        STATUS_SUCCESS = 0x00,           //!< Data processing has been successful.
        STATUS_ERROR = 0x01,             //!< Processing the data failed.
        STATUS_INVALID_PARAMETER = 0x02, //!< Received parameter(s) are invalid.
        STATUS_INPUT_DATA_TOLONG = 0x03, //!< Input data too long.
        STATUS_NOT_SUPPORTED = 0x04,     //!< DEVICE STATUS UNKNOWN.
        STATUS_UNKNOWN_ERROR = 0x05,     //!< Unexpected error.
        STATUS_DISCONNECTION = 0x06      //!< No confirmation of connection
    };

    QSerialPort *m_pSerialPort;
    bool m_isConnected;

    QTimer *m_pTimerCheckConnection;
};

#endif // RTC_H
