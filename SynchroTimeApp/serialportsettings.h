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
//! \file settings.h
//!
//! \brief This file contains the declaration of the structure 'Settings'.
//!
#ifndef SERIALPORTSETTINGS_H
#define SERIALPORTSETTINGS_H

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <QtSerialPort/QSerialPort>

//------------------------------------------------------------------------------
// Preprocessor
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Enums
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
//!
//! \struct Settings
//!
//! \brief The Settings structure and new type Settings_t.
//!
//! The structure contains specific fields for storing Serial Port / UART settings.
//! This structure is necessary for storing user settings of the Serial Port / UART.
//!
typedef struct Settings {
    QString name;                           //!< The name of the serial port.
    qint32 baudRate;                        //!< The data baud rate for the serial port.
    QString stringBaudRate;                 //!< The data baud rate for the UI.
    QSerialPort::DataBits dataBits;         //!< This property holds the data bits in a frame.
    QString stringDataBits;                 //!< description of the parameter data bits in a frame.
    QSerialPort::Parity parity;             //!< This property holds the parity checking mode.
    QString stringParity;                   //!< description of the parameter parity checking mode.
    QSerialPort::StopBits stopBits;         //!< This property holds the number of stop bits in a frame.
    QString stringStopBits;                 //!< description of the parameter number of stop bits in a frame.
    QSerialPort::FlowControl flowControl;   //!< This property holds the desired flow control mode.
    QString stringFlowControl;              //!< description of the parameter flow control mode.
    float correctionFactor;                 //!< Advanced parameter: Correction factor between frequency deviation and Aging register value.
    bool localEchoEnabled;                  //!< Advanced parameter: This is a flag that allows console input.
    bool isChanged = false;                 //!< This is a flag to save changes to the serial port parameters.
} Settings_t;

#endif // SERIALPORTSETTINGS_H

