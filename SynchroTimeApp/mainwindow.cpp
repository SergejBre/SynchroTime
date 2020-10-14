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
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "rtc.h"
#include <QMessageBox>
#include <QLabel>
#include <QDateTime>
#include <QTimer>
#include <QSettings>
#include <QThread>

//------------------------------------------------------------------------------
// Preprocessor
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Enums
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
#define SETTINGS_FILE "synchroTimeApp.ini"
//------------------------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------------------------

MainWindow::MainWindow( QWidget *parent ) :
    QMainWindow( parent ),
    ui(new Ui::MainWindow)
{
    ui->setupUi( this );
    m_pConsole = new Console;
    m_pConsole->setEnabled( false );
    setCentralWidget( m_pConsole );

    actionsTrigger( false );
    ui->actionQuit->setEnabled( true );

    clock = new QLabel;
    clock->setStyleSheet( QString("color: blue") );
    clock->setAlignment( Qt::AlignBottom | Qt::AlignRight );
    clock->setText( QDateTime::currentDateTime().toString("hh:mm:ss") );
    ui->statusBar->addPermanentWidget( clock );
    status = new QLabel;
    ui->statusBar->addWidget( status );
    this->readSettings();

    // Create a timer with 1 second intervals.
    m_pTimer = new QTimer( this );
    m_pTimer->setInterval( 1000 );
    QObject::connect( m_pTimer, &QTimer::timeout, this, &MainWindow::tickClock );
    m_pTimer->start();

    initActionsConnections();

    m_pThread = new QThread(this);

    // There is no need to specify the parent. The parent will be a thread when we move our RTC object into it.
    m_pRTC = new RTC( "ttyUSB0" );
    // We move the RTC object to a separate thread so that synchronous pending operations do not block the main GUI thread.
    // Create a connection: Delete the RTC object when the stream ends. start the thread.
    m_pRTC->moveToThread( m_pThread );
    QObject::connect(m_pThread, SIGNAL( finished()), m_pRTC, SLOT( deleteLater() ) );
    m_pThread->start();

    // Checking the connection.
    if ( m_pRTC->isConnected() )
    {
//        ui->pushButtonTurnOff->setEnabled(false);
    }
    else
    {
//        ui->pushButtonSet->setEnabled(false);
//        ui->pushButtonTurnOn->setEnabled(false);
//        ui->pushButtonTurnOff->setEnabled(false);
        QMessageBox::critical(this, "Connection error", "Connect the RTC device to the correct serial port"
                              "and restart the program.", QMessageBox::Ok);
    }

    QObject::connect(ui->actionInformation, &QAction::triggered, m_pRTC, &RTC::informationRequestSlot);
    QObject::connect(ui->actionAdjustment, &QAction::triggered, m_pRTC, &RTC::adjustmentRequestSlot);
    QObject::connect(ui->actionCalibration, &QAction::triggered, m_pRTC, &RTC::calibrationRequestSlot);
    QObject::connect(ui->actionReset, &QAction::triggered, m_pRTC, &RTC::resetRequestSlot);
    QObject::connect(ui->actionSetRegister, &QAction::triggered, m_pRTC, &RTC::setRegisterRequestSlot);

    QObject::connect(m_pRTC, &RTC::getData, m_pConsole, &Console::putData);
}

MainWindow::~MainWindow()
{
    // Wait 1s for the stream to complete before deleting the main window.
    m_pThread->quit();
    m_pThread->wait( 1000 );

    delete ui;
}

//!
//! \brief MainWindow::readSettings
//! The function reads the parameters necessary for the user interface that were saved in the previous session.
//!
//! Such important parameters will be read as
//! - the position of the window on the screen and window size,
//! - interface font and its size,
//! - the user interface settings (overwrite of the data, recurse of dir's, etc.)
//! - error and event logging options.
//!
void MainWindow::readSettings()
{
    QSettings settings( SETTINGS_FILE, QSettings::IniFormat );

    settings.beginGroup( "Geometry" );
    QPoint pos = settings.value( "pos", QPoint(200, 200) ).toPoint();
    QSize size = settings.value( "size", QSize(640, 400) ).toSize();
    this->resize( size );
    this->move( pos );
    settings.endGroup();

    settings.beginGroup( "Font" );
    QFont font;
    font.fromString(settings.value( "font", QFont()).toString() );
//    this->setFont(font);
    qApp->setFont(font);
    settings.endGroup();

    settings.beginGroup( "ULayout" );
    settings.endGroup();
/*
    settings.beginGroup( "SerialPort" );
    QString portName = settings.value( "portName", "ttyUSB0" ).toString();
    this->m_portName = portName;
    qint32 baudRate = settings.value( "baudRate", 115200 ).toUInt();
    this->m_baudRate = baudRate;
    settings.endGroup();
*/
}

//!
//! \brief MainWindow::writeSettings
//! The function saves the user interface parameters that have been changed by the user in the current session.
//!
//! Such parameters will be updated as
//! - the position of the window on the screen and window size,
//! - interface font and its size,
//! - the user interface settings (overwrite of the data, recurse of dir's, etc.)
//! - error and event logging options.
//!
void MainWindow::writeSettings() const
{
    QSettings settings( SETTINGS_FILE, QSettings::IniFormat );

    settings.beginGroup( "Geometry" );
    settings.setValue( "pos", pos() );
    settings.setValue( "size", size() );
    settings.endGroup();

    settings.beginGroup( "Font" );
    settings.setValue( "font", this->font().toString() );
    settings.endGroup();

    settings.beginGroup( "ULayot" );
    settings.endGroup();
/*
    settings.beginGroup( "SerialPort" );
    settings.setValue( "portName", this->m_portName );
    settings.setValue( "baudRate", this->m_baudRate );
    settings.endGroup();
*/
}

//!
//! \brief MainWindow::about
//!
void MainWindow::about()
{
    QMessageBox::about(this, QObject::tr("About SynchroTime App"),
                       QObject::tr("The <b>SynchroTime</b> application is used for fine tuning "
                                   "and calibration of the <b>RTC DS3231</b> module."
                                   "<br /><b>Version</b> %1"
                                   "<br /><b>Copyright</b> © 2020 sergej1@email.ua").arg(qApp->applicationVersion()));
}

//!
//! \brief MainWindow::connectRTC
//!
void MainWindow::connectRTC()
{
    m_pConsole->setEnabled( true );
    actionsTrigger( true );

    showStatusMessage( QObject::tr( "Connected to %1 : %2, %3, %4, %5, %6" ) );
}

//!
//! \brief MainWindow::disconnectRTC
//!
void MainWindow::disconnectRTC()
{
    m_pConsole->setEnabled( false );
    actionsTrigger( false );

    showStatusMessage( QObject::tr( "Disconnected" ) );
}

//!
//! \brief MainWindow::initActionsConnections
//!
void MainWindow::initActionsConnections()
{
    QObject::connect(ui->actionConnect, &QAction::triggered, this, &MainWindow::connectRTC);
    QObject::connect(ui->actionDisconnect, &QAction::triggered, this, &MainWindow::disconnectRTC);
    QObject::connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    QObject::connect(ui->actionClear, &QAction::triggered, m_pConsole, &Console::clear);
    QObject::connect(ui->actionAbout_App, &QAction::triggered, this, &MainWindow::about);
}

//!
//! \brief MainWindow::actionsTrigger
//! \param value of the type bool: true if connected, false if disconnected.
//!
void MainWindow::actionsTrigger( bool value ) const
{
    ui->actionConnect->setEnabled( !value );
    ui->actionDisconnect->setEnabled( value );
    ui->actionPort_Setting->setEnabled( !value );

    ui->actionInformation->setEnabled( value );
    ui->actionAdjustment->setEnabled( value );
    ui->actionCalibration->setEnabled( value );
    ui->actionReset->setEnabled( value );
    ui->actionSetRegister->setEnabled( value );
}

//!
//! \brief MainWindow::showStatusMessage
//! \param message
//!
void MainWindow::showStatusMessage(const QString &message) const
{
    status->setText(message);
}

//!
//! \brief MainWindow::tickClock
//!
void MainWindow::tickClock()
{
    clock->setText( QDateTime::currentDateTime().toString("hh:mm:ss") );
}

//!
//! \brief MainWindow::closeEvent Handle program completion event.
//!
//! Before exiting the program, the user interface parameters are automatically saved.
//!
//! \param event of type QCloseEvent*
//!
void MainWindow::closeEvent( QCloseEvent *event )
{
    this->writeSettings();
    event->accept();
}
