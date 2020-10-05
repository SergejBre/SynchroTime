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

#include <QString>
#include <QtTest>
#include <QCoreApplication>
#include "../include/interface.h"
#include "../include/session.h"

class Tests : public QObject
{
    Q_OBJECT

public:
    Tests();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void Case1_data();
    void Case1();
    void Case14();
    void Case15();
    void Case16();
    void Case17();
};

Tests::Tests()
{
}

void Tests::initTestCase()
{
}

void Tests::cleanupTestCase()
{
}

void Tests::Case1_data()
{
    QTest::addColumn<QString>("data");
    QTest::newRow("0") << QString();
}

void Tests::Case1()
{
    QFETCH(QString, data);
    QVERIFY2(true, "Failure");
}

//!
//! \brief Tests::testCase14
//! Info about all available in system serial ports.
//!
//! \details
//!
void Tests::Case14()
{
    int argc = 1;
    char *argv[] = { 0 };
    QCoreApplication app( argc, argv );

    InterfaceSP *interfaceSP = new InterfaceSP( &app );

    // Info about all available in system serial ports.
    interfaceSP->searchAllSerialPort();

    QTimer::singleShot(0, &app, SLOT(quit()));
    QCOMPARE( app.exec(), 0 );
}

//!
//! \brief Tests::testCase15
//! Prepare a new object Serial Port
//!
//! \details
//!
void Tests::Case15()
{
    int argc = 1;
    char *argv[] = { 0 };
    QCoreApplication app( argc, argv );

    if ( QSerialPortInfo::availablePorts().count() > 0 )
    {
        InterfaceSP *interfaceSP = new InterfaceSP( &app, QSerialPortInfo::availablePorts().at(0).portName() );
        QSerialPort* serialPort = interfaceSP->getSerialPort();
        qDebug() << "Port Name: " << serialPort->portName();
        qDebug() << "Baud Rate: " << serialPort->baudRate();
        qDebug() << "Read Buffer Size: " << serialPort->readBufferSize();
    }

    QTimer::singleShot(0, &app, SLOT(quit()));
    QCOMPARE( app.exec(), 0 );
}

//!
//! \brief Tests::testCase16
//!
//! \details
//!
void Tests::Case16()
{
    int argc = 1;
    char *argv[] = { 0 };
    QCoreApplication app(argc, argv);

    if ( QSerialPortInfo::availablePorts().count() > 0 )
    {
       InterfaceSP *interfaceSP = new InterfaceSP( &app, QSerialPortInfo::availablePorts().at(0).portName() );
       QCOMPARE( interfaceSP->searchSerialPort( QSerialPortInfo::availablePorts().at(0).portName() ), true );

        qint64 ret = 0;
        QByteArray data = "@v";

        if ( interfaceSP->getSerialPort()->isOpen() )
        {
            qDebug( "close Serial Port" );
            interfaceSP->getSerialPort()->close();
        }
#if 0
        interfaceSP->getSerialPort()->setBaudRate( QSerialPort::Baud115200, QSerialPort::AllDirections );
        interfaceSP->getSerialPort()->setDataBits( QSerialPort::Data8 );
        interfaceSP->getSerialPort()->setParity( QSerialPort::NoParity );
        interfaceSP->getSerialPort()->setStopBits( QSerialPort::OneStop );
        interfaceSP->getSerialPort()->setFlowControl( QSerialPort::NoFlowControl );
        interfaceSP->getSerialPort()->setReadBufferSize( 1024 );
#endif
        // open the Serial Port for the read and the write
        if ( interfaceSP->getSerialPort()->open( QIODevice::ReadWrite ) )
        {
            ret = interfaceSP->writeTheData( data );
            qDebug() << "Sent bytes: " << ret;

            interfaceSP->readTheData( 100 );

            if ( interfaceSP->getReceivedData().size() > 0 )
            {
                qDebug() << "Received bytes: " << interfaceSP->getReceivedData().size();
                qDebug() << interfaceSP->getReceivedData();
            }
            else
            {
                qDebug() << QObject::tr("Failed to read the data from port %1, error: %2").arg(interfaceSP->getPortName()).arg( interfaceSP->getSerialPort()->errorString() );
            }
            interfaceSP->getSerialPort()->close();
        }
        else
        {
            qDebug() << "Serial Port error " << interfaceSP->getSerialPort()->errorString();
            QCoreApplication::exit(1);
        }
    }

    QTimer::singleShot(0, &app, SLOT(quit()));
    QCOMPARE( app.exec(), 0 );
}

//!
//! \brief Tests::Case17
//!
//! \details
//!
void Tests::Case17()
{
    int argc = 1;
    char *argv[] = { 0 };
    QCoreApplication app(argc, argv);

    if ( QSerialPortInfo::availablePorts().count() > 0 )
    {
        QSerialPort serialPort;
        serialPort.setPortName( QSerialPortInfo::availablePorts().at(0).portName() );

        int serialPortBaudRate = QSerialPort::Baud115200;
        serialPort.setBaudRate( serialPortBaudRate, QSerialPort::AllDirections );

        if (!serialPort.open( QIODevice::ReadWrite ))
        {
            qDebug() << QObject::tr("Failed to open port %1, error: %2").arg( serialPort.portName() ).arg(serialPort.errorString()) << endl;
            QCoreApplication::exit(1);
        }

        QByteArray data = "@v";
        qint64 ret = 0;

        InterfaceSP serialPortInterface( &app, &serialPort );

        ret = serialPortInterface.writeTheData( data );
        qDebug() << "Sent bytes: " << ret;

        serialPortInterface.readTheData( 100 );
        if ( serialPortInterface.getReceivedData().size() > 0 )
        {
            qDebug() << "Received bytes: " << serialPortInterface.getReceivedData().count();
            qDebug() << serialPortInterface.getReceivedData();
        }
        else
        {
            qDebug() << QObject::tr("Failed to read the data from port %1, error: %2").arg(serialPortInterface.getPortName()).arg( serialPortInterface.getSerialPort()->errorString() );
        }
    }

    QTimer::singleShot(0, &app, SLOT(quit()));
    QCOMPARE( app.exec(), 0 );
}

QTEST_APPLESS_MAIN(Tests)

#include "tests.moc"
