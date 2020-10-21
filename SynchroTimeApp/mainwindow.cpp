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
#include "settingsdialog.h"
#include "rtc.h"
#include <QMessageBox>
#include <QLCDNumber>
#include <QLabel>
#include <QDateTime>
#include <QTimer>
#include <QSettings>
#include <QThread>
#include <QFontDialog>

//------------------------------------------------------------------------------
// Preprocessor
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Enums
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
#define WAIT_FOR_STREAM 1000 //!< Wait 1s for the stream
#define SETTINGS_FILE "synchroTimeApp.ini" //!< The name of the settings file.
//------------------------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------------------------

MainWindow::MainWindow( QWidget *parent ) :
    QMainWindow( parent ),
    ui( new Ui::MainWindow ),
    m_pConsole( nullptr ),
    m_pSettingsDialog( nullptr ),
    m_pThread( nullptr ),
    m_pRTC( nullptr )
{
    ui->setupUi( this );
    actionsTrigger( false );
    ui->actionSettings->setEnabled( false );
    ui->actionQuit->setEnabled( true );

    m_pConsole = new Console;
    m_pConsole->setEnabled( false );
    setCentralWidget( m_pConsole );
    m_pSettingsDialog = new SettingsDialog( this );
    this->readSettings();
    m_pSettingsDialog->fillSettingsUi();

    clock = new QLCDNumber;
    clock->setDigitCount(8);
    clock->setPalette( Qt::green );
    clock->setStyleSheet( QStringLiteral( "background: black" ));
    clock->display( QDateTime::currentDateTime().toString("hh:mm:ss") );
    ui->statusBar->addPermanentWidget( clock, 0 );
    status = new QLabel;
    ui->statusBar->addWidget( status );

    // Create a timer with 1 second intervals for the clock.
    m_pTimer = new QTimer( this );
    m_pTimer->setInterval( 1000 );
    QObject::connect( m_pTimer, &QTimer::timeout, this, &MainWindow::tickClock );
    m_pTimer->start();

    // Initialize action connections
    QObject::connect(ui->actionConnect, &QAction::triggered, this, &MainWindow::connectRTC);
    QObject::connect(ui->actionDisconnect, &QAction::triggered, this, &MainWindow::disconnectRTC);
    QObject::connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    QObject::connect(ui->actionClear, &QAction::triggered, m_pConsole, &Console::clear);
    QObject::connect(ui->actionPort_Setting, &QAction::triggered, m_pSettingsDialog, &SettingsDialog::show);
    QObject::connect(ui->actionSelect_Font, &QAction::triggered, this, &MainWindow::selectConsoleFont);
    QObject::connect(ui->actionAbout_App, &QAction::triggered, this, &MainWindow::about);
}

MainWindow::~MainWindow()
{
    // Wait 1s for the stream to complete before deleting the main window.
    if ( m_pThread != nullptr ) {
        m_pThread->quit();
        m_pThread->wait( WAIT_FOR_STREAM );
    }

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
//! - and the serial port options.
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
    font.fromString( settings.value( "font", QFont("Monospace", 10) ).toString() );
    Q_ASSERT( m_pConsole != nullptr );
    m_pConsole->setFont( font );
    settings.endGroup();

    settings.beginGroup( "ULayout" );
    settings.endGroup();

    Q_ASSERT( m_pSettingsDialog != nullptr );
    SettingsDialog::Settings *p = m_pSettingsDialog->serialPortSettings();
    settings.beginGroup( "SerialPort" );
    p->name = settings.value( "portName", "ttyUSB0" ).toString();
    p->baudRate = settings.value( "baudRate", 115200 ).toUInt();
    p->stringBaudRate = settings.value( "baudRate", 115200 ).toString();
    p->stringDataBits = settings.value( "dataBits", 8 ).toString();
    p->stringParity = settings.value( "parity", "None" ).toString();
    p->stringStopBits = settings.value( "stopBits", 1 ).toString();
    p->stringFlowControl = settings.value( "flowControl", "None" ).toString();
    settings.endGroup();
}

//!
//! \brief MainWindow::writeSettings
//! The function saves the user interface parameters that have been changed by the user in the current session.
//!
//! Such parameters will be updated as
//! - the position of the window on the screen and window size,
//! - interface font and its size,
//! - the user interface settings (overwrite of the data, recurse of dir's, etc.)
//! - and the serial port options.
//!
void MainWindow::writeSettings() const
{
    QSettings settings( SETTINGS_FILE, QSettings::IniFormat );

    settings.beginGroup( "Geometry" );
    settings.setValue( "pos", pos() );
    settings.setValue( "size", size() );
    settings.endGroup();

    settings.beginGroup( "Font" );
    Q_ASSERT( m_pConsole != nullptr );
    settings.setValue( "font", m_pConsole->font().toString() );
    settings.endGroup();

    settings.beginGroup( "ULayot" );
    settings.endGroup();

    Q_ASSERT( m_pSettingsDialog != nullptr );
    SettingsDialog::Settings p = m_pSettingsDialog->settings();
    settings.beginGroup( "SerialPort" );
    if ( p.isChanged ) {
        settings.setValue( "PortName", p.name );
        settings.setValue( "baudRate", p.baudRate );
        settings.setValue( "dataBits", p.stringDataBits );
        settings.setValue( "parity", p.stringParity );
        settings.setValue( "stopBits", p.stringStopBits );
        settings.setValue( "flowControl", p.stringFlowControl );
    }
    settings.endGroup();
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
//! \brief MainWindow::handleError
//! \param error
//!
void MainWindow::handleError( const QString &error )
{
    //! \todo
    disconnectRTC();
    QMessageBox::critical( this, QObject::tr( "Critical Error" ), error );
}

//!
//! \brief MainWindow::connectRTC
//!
void MainWindow::connectRTC()
{
    SettingsDialog::Settings p = m_pSettingsDialog->settings();
    m_pThread = new QThread(this);
    // There is no need to specify the parent. The parent will be a thread when we move our RTC object into it.
    m_pRTC = new RTC( p.name );
    // We move the RTC object to a separate thread so that synchronous pending operations do not block the main GUI thread.
    // Create a connection: Delete the RTC object when the stream ends. start the thread.
    m_pRTC->moveToThread( m_pThread );
    QObject::connect(m_pThread, SIGNAL( finished()), m_pRTC, SLOT( deleteLater() ) );
    m_pThread->start();

    // Checking the connection.
    if ( m_pRTC->isConnected() )
    {
        m_pConsole->setEnabled( true );
        actionsTrigger( true );

        QObject::connect(ui->actionInformation, &QAction::triggered, m_pRTC, &RTC::informationRequestSlot);
        QObject::connect(ui->actionAdjustment, &QAction::triggered, m_pRTC, &RTC::adjustmentRequestSlot);
        QObject::connect(ui->actionCalibration, &QAction::triggered, m_pRTC, &RTC::calibrationRequestSlot);
        QObject::connect(ui->actionReset, &QAction::triggered, m_pRTC, &RTC::resetRequestSlot);
        QObject::connect(ui->actionSetRegister, &QAction::triggered, m_pRTC, &RTC::setRegisterRequestSlot);

        QObject::connect(m_pRTC, &RTC::getData, m_pConsole, &Console::putData);
        QObject::connect(m_pRTC, &RTC::portError, this, &MainWindow::handleError);

        showStatusMessage( QObject::tr( "Connected to %1 : %2, %3, %4, %5, %6" )
                           .arg( p.name ).arg( p.stringBaudRate ).arg( p.stringDataBits )
                           .arg( p.stringParity ).arg( p.stringStopBits ).arg( p.stringFlowControl ) );
    }
    else
    {
        m_pThread->quit();
        m_pThread->wait( WAIT_FOR_STREAM );

        showStatusMessage( QObject::tr( "Connection error" ));
        QMessageBox::critical(this, "Connection error", "Connect the RTC device to the correct serial port, "
                                                        "or set the serial port name in the port settings.",
                              QMessageBox::Ok);
    }
}

//!
//! \brief MainWindow::disconnectRTC
//!
void MainWindow::disconnectRTC()
{
    Q_ASSERT( m_pConsole != nullptr );
    m_pConsole->setEnabled( false );
    actionsTrigger( false );

    Q_ASSERT( m_pThread != nullptr );
    if ( m_pThread != nullptr ) {
        m_pThread->quit();
        m_pThread->wait( WAIT_FOR_STREAM );
    }

    showStatusMessage( QObject::tr( "Disconnected" ) );
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
    Q_ASSERT( status != nullptr );
    status->setText(message);
}

//!
//! \brief MainWindow::tickClock
//!
void MainWindow::tickClock()
{
    Q_ASSERT( clock != nullptr );
    clock->display( QDateTime::currentDateTime().toString("hh:mm:ss") );
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

//!
//! \brief MainWindow::selectConsoleFont Slot for the font selection dialog.
//!
//! Selection of the console font. The default font is Monospace, 10 points.
//!
//! \note The font is installed only for the console window, i.s. not for all graphic forms!
//!
void MainWindow::selectConsoleFont( void )
{
    Q_ASSERT( m_pConsole != nullptr );
    bool selected;
    QFont font = QFontDialog::getFont( &selected, m_pConsole->font(), this );

    if ( selected )
    {
        m_pConsole->setFont( font );
    }
}
