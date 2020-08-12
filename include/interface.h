//------------------------------------------------------------------------------
//  Home Office
//  Nürnberg, Germany
//  E-Mail: sergej1@email.ua
//
//  Copyright (C) 2020 free Project SynchroTime. All rights reserved.
//------------------------------------------------------------------------------
//  Project SynchroTime: Time synchronization via Serial Port (UART)
//  with a DS3231 Precision RTC module.
//
//------------------------------------------------------------------------------
#ifndef INTERFACE_H
#define INTERFACE_H

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTimer>
#include <QByteArray>
#include "base.h"

//------------------------------------------------------------------------------
// Preprocessor
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Enums
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
// Interval for a time-out in milliseconds
#define REBOOT_WAIT 2000

//! \brief Interface
//! The base class Interface provides for the communication between
//! Bootloader Client and Host Device.
//!
//! \details
//! Base class interface provides two virtual methods for sending and
//! receiving the instructions and the data.
//! ( Interface::writeTheData Interface::readTheData )
//! These virtual methods are implemented in the other derived classes.
//!
class Interface : public Base
{
    Q_OBJECT
    Q_CLASSINFO("className", "Interface")

public:
    explicit Interface(QObject *parent = 0);

    // Virtual communicate Methods
    virtual qint64 writeTheData( const QByteArray & ) = 0;
    virtual bool readTheData( const quint32, const quint32 bytes = 0U ) = 0;
    virtual QIODevice *getSocket( void ) = 0;
    virtual void initSocket( void ) = 0;
    virtual bool openSocket( void ) = 0;
    virtual void closeSocket( void ) = 0;

    void setBlockSize( const quint16 size);
    quint16 getBlockSize( void ) const;
    quint32 getReceivedBytes( void ) const;
    void setTimer( QTimer *timer );
    QTimer *getTimer( void );
    void setTimeout( quint32 time );
    quint32 getTimeout( void ) const;
    QByteArray& getReceivedData( void );

signals:
    void infoMessage( const QVariant & message ) const;
    void debugMessage( const QVariant & message ) const;
    void errorMessage( const QVariant & message ) const;

private:

    //! Timer for Time-out method
    QTimer *timer;

    //! The timeout interval in milliseconds.
    quint32 timeout;

    //! Size of the data block for the communication protocol
    quint16 blockSize;

    //! The received data
    QByteArray receivedData;
};

//! \brief
//! The class InterfaceSP provides methods to access serial ports.
//!
//! \details
//! You can get information about the available serial ports using the QSerialPortInfo helper class,
//! which allows an enumeration of all the serial ports in the system.
//! This is useful to obtain the correct name of the serial port you want to use.
//! You can pass an object of the helper class as an argument to the setPort() or
//! setPortName() methods to assign the desired serial device.
//! After setting the port, you can open it in read-only (r/o), write-only (w/o),
//! or read-write (r/w) mode using the open() method.
//!
//! \note
//! The serial port is always opened with exclusive access (that is, no other process or
//! thread can access an already opened serial port).
//!
class InterfaceSP : public Interface
{
    Q_OBJECT
    Q_CLASSINFO("className", "InterfaceSP")

public:
    InterfaceSP(QObject *parent = 0);
    InterfaceSP(QObject *parent, const QString & portName );
    InterfaceSP(QObject *parent, QSerialPort * port );
    ~InterfaceSP();

    // Init Method
    void init( void );

    // Init a Serial Port
    void initSerialPort( void );
    void searchAllSerialPort( void );
    bool searchSerialPort( const QString & portName );
    QStringList availableSerialPorts( void );

    // Virtual communicate Methods
    qint64 writeTheData( const QByteArray & data );
    bool readTheData( const quint32 timewait, const quint32 bytes = 0U );
    QSerialPort* getSocket( void );
    void initSocket( void );
    bool openSocket( void );
    void closeSocket( void );

    // Get methods
    QString getPortName( void ) const;
    qint32 getPortBaudRate( void ) const;
    QString getDescription( void ) const;
    QString getManufacturer( void ) const;
    QString getSerialNumber( void ) const;
    quint16 getProductIdentifier( void ) const;
    QSerialPortInfo getSerialPortInfo( void ) const;
    QSerialPort* getSerialPort( void );

private slots:
    void handleReadyRead();
    void handleTimeout();
    void handleError( QSerialPort::SerialPortError error );

private:

    //! Serial Port
    QSerialPort *serialPort;

    //! It provides information on this serial ports:
    //! - The name of serial port
    //! - The description string of the serial port
    //! - The manufacturer string of the serial port
    //! - The serial number string of the serial port
    //! - The 16-bit product number for the serial port
    //! - The 16-bit vendor number for the serial port
    //! - The list of available standard baud rates supported by the current serial port
    QSerialPortInfo serialPortInfo;

    //! The size of the internal read buffer.
    //! This limits the amount of data that the client can receive before calling the read()
    //! or readAll() methods.
    qint64 readBufferSize;

    //! The number of bytes that are in the buffer for reading
    qint64 bytesAvailable;

};

#endif // INTERFACE_H
