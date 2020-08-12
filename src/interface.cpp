//------------------------------------------------------------------------------
//  Home Office
//  Nürnberg, Germany
//  E-Mail: sergej1@email.ua
//
//  Copyright (C) 2020 free Project SynchroTime. All rights reserved.
//------------------------------------------------------------------------------
//  Project SynchroTime: Time synchronization via Serial Port (UART)
//
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include "interface.h"
//#include <QCoreApplication>
#include <QLoggingCategory>
#include <QDebug>
#include <QTime>

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
Q_LOGGING_CATEGORY(logInter, "Interface")

//------------------------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------------------------

//! \brief Interface::Interface
//! Constructor of the class Interface for creating objects with default values.
//!
//! \details
//! Create a instance of the class and call following init methods:
//!  -# Interface::init()
//!  .
//!
//! \param[in] parent Pointer to the parent object used for QObject
//!
Interface::Interface( QObject *parent )
    : Base( parent )
    , blockSize( 0U )
{
    this->receivedData.clear();
}

//! \brief Interface::setBlockSize
//!
//! \details
//! Sets the size of the data block for the communication protocol.
//!
//! \param size of typ const quint16
//!
void Interface::setBlockSize( const quint16 size)
{
    this->blockSize = size;
}

//! \brief Interface::getBlockSize
//!
//! \details
//! Gets the size of the data block of the communication protocol.
//!
//! \return blockSize of the type quint16.
//!
quint16 Interface::getBlockSize( void ) const
{
    return this->blockSize;
}

//! \brief Interface::getReceivedBytes
//!
//! \details
//! Gets the number of the received bytes.
//!
//! \return receivedBytes of the type quint32.
//!
quint32 Interface::getReceivedBytes( void ) const
{
    if ( this->receivedData.isNull() )
    {
        return 0U;
    }
    return this->receivedData.size();
}

//! \brief Interface::setTimer
//!
//! \details
//! Set the Pointer of new object Timer.
//!
//! \param *timer of the type QTimer*.
//!
void Interface::setTimer( QTimer *timer )
{
    this->timer = timer;
}

//! \brief Interface::getTimer
//!
//! \details
//! Returns the Pointer of object Timer.
//!
//! \return *timer of the type QTimer.
//!
QTimer* Interface::getTimer( void )
{
    Q_ASSERT( this->timer != NULL );
    return this->timer;
}

//! \brief Interface::setTimeout
//!
//! \details
//! Set the Interval for a time-out in milliseconds.
//!
//! \param interval of the type quint32.
//!
void Interface::setTimeout( quint32 interval )
{
    this->timeout = interval;
}

//! \brief Interface::getTimeout
//!
//! \details
//! Returns the Interval for a time-out in milliseconds.
//!
//! \return timeout of the type quint32.
//!
quint32 Interface::getTimeout( void ) const
{
    return this->timeout;
}

//! \brief Interface::getReceivedData
//!
//! \details
//! Returns the received Data from the communication port.
//!
//! \return receivedData of the type QByteArray&.
//!
QByteArray& Interface::getReceivedData( void )
{
    return this->receivedData;
}

//! \brief InterfaceSP::InterfaceSP
//! Constructor of the class InterfaceSP for creating objects with default values.
//!
//! \details
//! Create a instance of the class and call following init methods:
//!  -# InterfaceSP::init()
//!  .
//!
//! \param[in] parent Pointer to the parent object used for QObject
//!
InterfaceSP::InterfaceSP( QObject *parent )
    : Interface( parent )
    , serialPort( NULL )
{
    init();
}

//! \brief InterfaceSP::InterfaceSP
//! Constructor of the class InterfaceSP for creating objects with default values.
//!
//! \details
//! Create a instance of the class and call following init methods:
//!  -# InterfaceSP::init()
//!  -# InterfaceSP::searchSerialPort( const QString & portName )
//!  -# InterfaceSP::initSerialPort()
//!  .
//!
//! \param[in] parent Pointer to the parent object used for QObject.
//!
//! \param[in] portName a Serial Port Name of the type const QString.
//!
InterfaceSP::InterfaceSP( QObject *parent, const QString & portName )
    : Interface( parent )
    , serialPort( NULL )
{
    init();

    if ( searchSerialPort( portName ) )
    {
        initSerialPort();
    }
}

//! \brief InterfaceSP::InterfaceSP
//! Constructor of the class InterfaceSP for creating objects with default values.
//!
//! \details
//! Create a instance of the class and call following init methods:
//!  -# InterfaceSP::init()
//!  .
//!
//! \param[in] parent Pointer to the parent object used for QObject.
//!
//! \param[in] *port a pointer an the Serial Port Name of the type QSerialPort*.
//!
InterfaceSP::InterfaceSP(QObject *parent, QSerialPort * port ) :
    Interface(parent)
  , serialPort(port)
{
    init();

    QObject::connect( this->serialPort, &QSerialPort::readyRead, this, &InterfaceSP::handleReadyRead );
    QObject::connect( this->serialPort, static_cast <void (QSerialPort::*)(QSerialPort::SerialPortError)> (&QSerialPort::error),
                     this, &InterfaceSP::handleError );
}

//! \brief
//! Destructor for the class InterfaceSP
//!
//! \details
//!
//!
InterfaceSP::~InterfaceSP()
{

}

//! \brief InterfaceSP::init
//!
//! \details
//! This method establishes the necessary fields of the class to read the data,
//! and starts a timeout.
//!
void InterfaceSP::init( void )
{
    this->readBufferSize = 528U;
    this->bytesAvailable = 0U;

    // Instance a Timer object
    this->setTimer( new QTimer( this ) );
    this->getTimer()->setSingleShot( true );
    // Set Interval for Time-out in milliseconds.
    this->setTimeout( 1000 );

    QObject::connect( this->getTimer(), &QTimer::timeout, this, &InterfaceSP::handleTimeout );

    this->getTimer()->start( this->getTimeout() );
}

//! \brief InterfaceSP::initSerialPort
//! Prepare a instance of the Serial Port
//!
//! \details
//!
void InterfaceSP::initSerialPort( void )
{
    this->serialPort = new QSerialPort(this);
    this->serialPort->setPort( this->serialPortInfo );

    // set the default Baud Rate to 115200 baud.
    if ( this->serialPort->baudRate(QSerialPort::AllDirections) < QSerialPort::Baud115200 )
    {
        foreach ( const qint32 & standard, serialPortInfo.standardBaudRates() )
        {
            if ( standard == QSerialPort::Baud115200 )
            {
                this->serialPort->setBaudRate(QSerialPort::Baud115200, QSerialPort::AllDirections);
                break;
            }
        }
    }

    // Set the size of the internal read buffer.
    // If the buffer size is limited to a certain size,
    // QSerialPort will not buffer more than this size of data.
    // The special case of a buffer size of 0 means that
    // the read buffer is unlimited and all incoming data is buffered.
    this->serialPort->setReadBufferSize( readBufferSize );

    QObject::connect( this->serialPort, &QSerialPort::readyRead, this, &InterfaceSP::handleReadyRead );
    QObject::connect( this->serialPort, static_cast <void (QSerialPort::*)(QSerialPort::SerialPortError)> (&QSerialPort::error),
                     this, &InterfaceSP::handleError );
}

//! \brief InterfaceSP::searchAllSerialPort
//! Info about all available serial ports
//!
//! \details
//!
void InterfaceSP::searchAllSerialPort( void )
{
    int n = 0;
    // Info about all available in system serial ports.
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        stdOutput() << "Serial Port : " << info.portName() << endl;
//        stdOutput() << "Serial number: " << info.serialNumber() << endl;
        stdOutput() << "Description : " << info.description() << endl;
        stdOutput() << "Manufacturer: " << info.manufacturer() << endl;
        stdOutput() << "Vendor ID   : " << (info.hasVendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : QString()) << endl;
        stdOutput() << "Product ID  : " << (info.hasProductIdentifier() ? QString::number(info.productIdentifier(), 16) : QString()) << endl;
        stdOutput() << "System Locat: " << info.systemLocation() << endl;
        stdOutput() << "Busy        : " << (info.isBusy() ? QObject::tr("Yes") : QObject::tr("No")) << endl;
        n++;
    }

    if ( n > 0 )
    {
        stdOutput() << QObject::tr( "A total of %1 Serial Ports have been found." ).arg( n ) << endl;
    }
    else
    {
        stdOutput() << "The serial ports are not present in the system." << endl;
    }
}

//! \brief InterfaceSP::searchSerialPort
//! This method checks a serial port for existence.
//!
//! \details
//!
bool InterfaceSP::searchSerialPort( const QString & portName )
{
    this->serialPortInfo = QSerialPortInfo( portName );

    if ( serialPortInfo.isNull() )
    {
        qCritical( logInter ) << QObject::tr( "Serial Port %1 does not exist in the system." ).arg( portName );
        return false;
    }
    return true;
}

//! \brief InterfaceSP::availableSerialPorts
//! Return a list with all available serial port names.
//!
//! \details
//!
//! \return serialPorts of the type QStringList
//!
QStringList InterfaceSP::availableSerialPorts( void )
{
    QStringList serialPorts;

    // Info about all available in system serial ports.
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        serialPorts << info.portName();
    }
    return serialPorts;
}

//!
//! \brief InterfaceSP::writeTheData
//! Writes the content of byteArray to the Serial Port device.
//!
//! \details
//! Send the data to a UART device.
//!
//! \param data of type const QByteArray.
//!
//! \return bytesWritten of the type qint64.
//!
qint64 InterfaceSP::writeTheData( const QByteArray & data )
{
    Q_ASSERT( this->serialPort != NULL );
    Q_ASSERT( this->serialPort->isOpen() && this->serialPort->isWritable() );
    qDebug() << "block size" << this->getBlockSize() << "data size" << data.size();
    Q_ASSERT( ( this->getBlockSize() == 0U ) || ( this->getBlockSize() >= data.size() ) );

    qint64 bytesWritten = -1;
    this->getTimer()->start( this->getTimeout() );

    if ( this->serialPort->isOpen() && this->serialPort->isWritable() )
    {
#if 0
        // Check if the file size does not exceed the buffer size.
        if ( this->getBlockSize() > 0U && this->getBlockSize() < data.size() )
        {
            qDebug() << "block size" << this->getBlockSize();
            int pos = 0;
            QByteArrayList dataList;
            while( pos < data.size() )
            {
                QByteArray tmp = data.mid( pos, this->getBlockSize() );
                dataList << tmp;
                pos += tmp.size();
            }

            foreach ( const QByteArray &block, dataList)
            {
                int bytes = 0;
                if ( ( bytes = this->writeTheData( block ) ) != block.size() )
                {
                    qDebug() << "block size" << block.size();
                    break;
                }

                this->readTheData( 100 );
                qDebug() << "Received bytes w: " << this->getReceivedData().size();

                if ( this->getReceivedData().data()[0] != 0x00 )
                {
                    qDebug() << "break" << this->getReceivedData().data()[0];
                    break;
                }
            }
        }
#endif
        // Writes the content of byteArray to the device.
        // Returns the number of bytes that were actually written, or -1 if an error occurred.
        bytesWritten = this->serialPort->write( data );

        // Returns true if a payload of data was written to the device;
        //  returns false if the operation timed out, or if an error occurred.
        if ( serialPort->waitForBytesWritten( this->getTimeout() ) )
        {
            if ( bytesWritten == -1 )
            {
                stdOutput() << QObject::tr( "Failed to write the data to port %1, error: %2" ).arg( serialPort->portName() ).arg( serialPort->errorString() ).toLocal8Bit() << endl;
            }
            else if ( bytesWritten != data.size() )
            {
                stdOutput() << QObject::tr( "Failed to write all the data to port %1, error: %2" ).arg( serialPort->portName() ).arg( serialPort->errorString() ).toLocal8Bit() << endl;
            }
        }
        else
        {
            stdOutput() << QObject::tr( "Failed to write the data to port %1, error: %2" ).arg( serialPort->portName() ).arg( serialPort->errorString() ).toLocal8Bit() << endl;
        }
    }
    else
    {
        stdOutput() << QObject::tr( "Failed to open port %1, error: %2" ).arg( serialPort->portName() ).arg( serialPort->errorString() ).toLocal8Bit() << endl;
    }
    return bytesWritten;
}

//! \brief InterfaceSP::readTheData
//! Reads all remaining data from the UART device
//!
//! \details
//! Set a time-out for the read the data from a device.
//!
//! \param timewait of the type quint32.
//!
bool InterfaceSP::readTheData( const quint32 timewait, const quint32 bytes )
{
    Q_ASSERT( this->serialPort->isOpen() && this->serialPort->isReadable() );

    if ( this->getReceivedData().size() > 0 )
    {
        this->getReceivedData().clear();
    }

    bool wait = false;

    if ( this->serialPort->isOpen() && this->serialPort->isReadable() )
    {
        QTime time;
        time.start();
        wait = true;
        while ( wait )
        {
            if ( serialPort->waitForReadyRead( timewait ) )
            {
                if ( bytes > 0 && this->getReceivedBytes() >= bytes )
                {
                    break;
                }
            }
            else
            {
                wait = false;
                stdOutput() << QObject::tr( "Serial port %1 wait %2ms for Ready Read: %3." ).arg( serialPort->portName() ).arg( timewait ).arg( serialPort->errorString() ).toLocal8Bit() << endl;
            }
        }
        qDebug() << QObject::tr( "Wait for ReadyRead signal %1 ms" ).arg( time.elapsed() );
    }
    else
    {
        // Error handling...
    }
    return wait;
}

//! \todo
void InterfaceSP::handleReadyRead( void )
{

    this->bytesAvailable += this->serialPort->bytesAvailable();

    // This function has no way of reporting errors;
    // returning an empty QByteArray can mean either that no data was currently available for reading,
    // or that an error occurred.
    this->getReceivedData().append( serialPort->readAll() );

    if ( !this->getTimer()->isActive() )
    {
        this->getTimer()->start( getTimeout() );
    }

#if 0
    while ( serialPort->bytesAvailable() )
    {
        if ( bytesAvailable + serialPort->bytesAvailable() < readBufferSize )
        {
            bytesAvailable += serialPort->bytesAvailable();
            this->getReceivedData().append( serialPort->readAll() );
        }
        else
        {
            this->getReceivedData().append( serialPort->read( readBufferSize - bytesAvailable ) );
            qDebug() << "size of the block: " << this->getReceivedData().size();
            this->getReceivedData().clear();
            bytesAvailable = 0U;
        }
    }

    if ( !this->getTimer()->isActive() )
    {
        this->getTimer()->start( getTimeout() );
    }
#endif
}


//! \brief InterfaceSP::handleTimeout
//! A slot for treatment of the signal Time-Out
//!
//! \details
//! The slot terminates the console application
//! or sends another signal to GUI application.
//!
void InterfaceSP::handleTimeout()
{
    if ( this->serialPort != NULL )
    {
       stdOutput() << QObject::tr( "Timeout for port %1: %2" ).arg( this->serialPort->portName() ).arg( this->serialPort->errorString() ).toLocal8Bit() << endl;

       if ( this->serialPort->isOpen() )
       {
           this->serialPort->close();
       }
    }
    else
    {
        stdOutput() << QObject::tr( "Timeout after %1 milliseconds." ).arg( this->getTimeout() ).toLocal8Bit() << endl;
    }
#ifdef GUI_APP
    if ( this->serialPort != NULL )
    {
        emit errorMessage( QObject::tr( "Timeout for port %1, error: %2" ).arg( this->serialPort->portName() ).arg( this->serialPort->errorString() ) );
    }
    else
    {
        emit errorMessage( QObject::tr( "Timeout after %1 milliseconds." ).arg( this->getTimeout() ) );
    }
#else
//    QCoreApplication::exit( 1 );
    qFatal( QObject::tr( "Timeout for port %1, error: %2" ).arg( this->serialPort->portName() ).arg( this->serialPort->errorString() ).toLocal8Bit() );

#endif
}

//! \todo
void InterfaceSP::handleError( QSerialPort::SerialPortError error )
{
    if ( error == QSerialPort::ReadError )
    {
        stdOutput() << QObject::tr( "An I/O error occurred while reading the data from port %1, error: %2" ).arg( serialPort->portName() ).arg( serialPort->errorString() ) << endl;
    }
}

//! \brief InterfaceSP::getSerialPortInfo
//!
//! \details
//! Returns the Information of the serial port.
//!
//! \return serialPortInfo of the type QSerialPortInfo.
//!
QSerialPortInfo InterfaceSP::getSerialPortInfo( void ) const
{
    return this->serialPortInfo;
}

//! \brief InterfaceSP::getSerialPort
//!
//! \details
//! Returns the Information of the serial port.
//!
//! \return serialPort of the type QSerialPort*.
//!
QSerialPort* InterfaceSP::getSerialPort( void )
{
    return this->serialPort;
}

//! \brief InterfaceSP::getSocket
//!
//! \details
//! Returns the Information of the serial port.
//!
//! \return serialPort of the type QSerialPort*.
//!
QSerialPort* InterfaceSP::getSocket( void )
{
    return this->serialPort;
}

//! \brief InterfaceSP::initSocket
//!
//! \details
//!
void InterfaceSP::initSocket( void )
{

}

//! \brief InterfaceSP::openSocket
//!
//! \details
//!
bool InterfaceSP::openSocket( void )
{
    Q_ASSERT( this->serialPort != NULL );
    Q_ASSERT( !this->serialPort->isOpen() );

    if ( this->serialPort == NULL )
    {
        qCritical( logInter ) << QObject::tr( "Serial Port is not specified." );
        return false;
    }

    this->serialPort->setFlowControl(QSerialPort::HardwareControl);

    // open the Serial Port for the read and the write
    if ( !this->serialPort->open( QIODevice::ReadWrite ) )
    {
        qCritical( logInter ) << QObject::tr( "Failed to open port %1, error: %2.").arg( serialPort->portName() ).arg( serialPort->errorString() );
        return false;
    }

    // the timer is needed, since the microcontroller after the DTR (Data Terminal Ready) signal goes into reboot.
    QTime time;
    time.start();
    while ( time.elapsed() < REBOOT_WAIT );

    if ( this->serialPort->isDataTerminalReady() )
    {
        qDebug() << QObject::tr( "The data terminal is ready. Hardware flow control=%1" ).arg( serialPort->flowControl() );
    }
    else
    {
        qDebug() << QObject::tr( "The data terminal is not ready!" );
    }

    return true;
}

//! \brief InterfaceSP::closeSocket
//!
//! \details
//!
void InterfaceSP::closeSocket( void )
{
    if ( this->serialPort->isOpen() )
    {
        this->serialPort->close();
    }
}

//! \brief InterfaceSP::getPortName
//!
//! \details
//! Returns the name of the serial port.
//!
//! \return portName of the type QString.
//!
QString InterfaceSP::getPortName( void ) const
{
    return this->serialPort->portName();
}

//! \brief InterfaceSP::getPortBaudRate
//! Returns the baud rate of the serial port.
//!
//! \details
//! Warning: Setting the AllDirections flag is supported on all platforms.
//! Windows and Windows CE support only this mode.
//! Warning: Returns equal baud rate in any direction on Windows,
//! Windows CE.
//! The default value is Baud9600, i.e. 9600 bits per second.
//!
//! \return portBaudRate of the type int.
//!
//! \note
//! If the setting is set before opening the port,
//! the actual serial port setting is done automatically
//! in the QSerialPort::open() method right after that
//! the opening of the port succeeds.
qint32 InterfaceSP::getPortBaudRate( void ) const
{
    return this->serialPort->baudRate(QSerialPort::AllDirections);
}

//! \brief InterfaceSP::getDescription
//!
//! \details
//! Returns the description string of the serial port.
//!
//! \return description of the type QString.
//!
QString InterfaceSP::getDescription( void ) const
{
    return this->serialPortInfo.description();
}

//! \brief InterfaceSP::getManufacturer
//!
//! \details
//! Returns the manufacturer string of the serial port.
//!
//! \return manufacturer of the type QString.
//!
QString InterfaceSP::getManufacturer( void ) const
{
    return this->serialPortInfo.manufacturer();
}

//! \brief InterfaceSP::getSerialNumber
//!
//! \details
//! Returns the serial number string of the serial port.
//!
//! \return serialNumber of the type QString.
//!
QString InterfaceSP::getSerialNumber( void ) const
{
    return this->serialPortInfo.serialNumber();
}

//! \brief InterfaceSP::getProductIdentifier
//!
//! \details
//! Returns the 16-bit product number for the serial port.
//!
//! \return productIdentifier of the type quint16.
//!
quint16 InterfaceSP::getProductIdentifier( void ) const
{
    return this->serialPortInfo.productIdentifier();
}
