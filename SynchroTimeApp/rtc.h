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
//! \details
//! Requests to be sent to the RTC device. Sent as the second byte in the protocol.
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

//!
//! \enum StatusMessages
//!
//! \details
//! Contains all possible error codes in response to a status request.
enum class StatusMessages : quint8
{
    STATUS_SUCCESS = 0x00,           //!< Data processing has been successful.
    STATUS_ERROR = 0x01,             //!< Processing the data failed.
    STATUS_INVALID_PARAMETER = 0x02, //!< Received parameter(s) are invalid.
    STATUS_INPUT_DATA_TOLONG = 0x03, //!< Input data too long.
    STATUS_NOT_SUPPORTED = 0x04,     //!< The state of the device is undefined.
    STATUS_UNKNOWN_ERROR = 0x05,     //!< Unexpected error.
    STATUS_DISCONNECTION = 0x06      //!< No confirmation of connection received.
};

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
class QSerialPort;
class QTimer;

//! \class RTC
//! \brief The RTC class
//! The class is responsible for communication with the RTC device.
class RTC : public QObject
{
    Q_OBJECT

public:
    explicit RTC( const QString &portName, QObject *parent = 0 );
    explicit RTC( const Settings_t &portSettings, QObject *parent = 0 );
    ~RTC();

    // Connection check function.
    bool isConnected() const;

signals:
    void getData( const QString &data );
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
    // Open the serial port
    bool openSerialPort() const;
    // The connection function.
    void connectToRTC();
    // The function send a request to the RTC.
    const QByteArray sendRequest( Request request, quint8 size = 0, const quint8 *const data = nullptr );
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

    QSerialPort *m_pSerialPort;
    bool m_isConnected;
    bool m_isBusy;
    QTimer *m_pTimerCheckConnection;
};

#endif // RTC_H
