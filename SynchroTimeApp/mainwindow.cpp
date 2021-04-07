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
//! \file mainwindow.cpp
//!
//! \brief The file contains the definition of the constructor and methods of the MainWindow class.
//!
//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "serialportsettings.h"
#include "settingsdialog.h"
#include "rtc.h"
#include <QMessageBox>
#include <QLCDNumber>
#include <QLabel>
#include <QTime>
#include <QTimer>
#include <QSettings>
#include <QThread>
#include <QInputDialog>
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
#define SETTINGS_FILE QStringLiteral( "synchroTimeApp.ini" ) //!< The name of the settings file.
//------------------------------------------------------------------------------
// Function definitions
//------------------------------------------------------------------------------

//!
//! \brief MainWindow::MainWindow
//! Constructor of the main application window.
//! \param parent of the type QWidget
//!
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
    // Here, the settings of the serial interface is retrieved from the configuration file.
    m_pSettingsDialog->fillSettingsUi();

    rate = new QLabel;
    ui->statusBar->addPermanentWidget( rate, 0 );
    clock = new QLCDNumber;
    clock->setDigitCount(8);
    clock->setPalette( Qt::green );
    clock->setStyleSheet( QStringLiteral("background: black") );
    clock->display( QTime::currentTime().toString() );
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
    QObject::connect(ui->actionAbout_Qt, &QAction::triggered, this, &MainWindow::aboutQt);
    QObject::connect(ui->actionContents, &QAction::triggered, this, &MainWindow::help);
    QObject::connect(ui->actionSetRegister, &QAction::triggered, this, &MainWindow::setRegisterSlot );
}

//!
//! \brief MainWindow::~MainWindow
//! Destructor in which the completion of a separate thread is checked.
//!
MainWindow::~MainWindow()
{
    // Wait 1s for the stream to complete before destroy the main window.
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

    settings.beginGroup( QStringLiteral( "Geometry" ));
    QPoint pos = settings.value( QStringLiteral( "pos" ), QPoint(200, 200) ).toPoint();
    QSize size = settings.value( QStringLiteral( "size"), QSize(640, 400) ).toSize();
    this->resize( size );
    this->move( pos );
    settings.endGroup();

    settings.beginGroup( QStringLiteral( "Font" ));
    QFont font;
    font.fromString( settings.value( QStringLiteral( "font" ), QFont( QStringLiteral( "Monospace" ), 10) ).toString() );
    Q_ASSERT( m_pConsole != nullptr );
    m_pConsole->setFont( font );
    settings.endGroup();

    settings.beginGroup( QStringLiteral( "ULayout" ));
    settings.endGroup();

    Q_ASSERT( m_pSettingsDialog != nullptr );
    Settings_t *const p = m_pSettingsDialog->serialPortSettings();
    settings.beginGroup( QStringLiteral( "SerialPort" ));
    p->name = settings.value( QStringLiteral( "portName" ), QStringLiteral( "ttyUSB0" )).toString();
    p->baudRate = settings.value( QStringLiteral( "baudRate" ), 115200 ).toUInt();
    p->stringBaudRate = settings.value( QStringLiteral( "baudRate" ), 115200 ).toString();
    p->stringDataBits = settings.value( QStringLiteral( "dataBits" ), 8 ).toString();
    p->stringParity = settings.value( QStringLiteral( "parity" ), QStringLiteral( "NoParity" )).toString();
    p->stringStopBits = settings.value( QStringLiteral( "stopBits" ), 1 ).toString();
    p->stringFlowControl = settings.value( QStringLiteral( "flowControl" ), QStringLiteral( "None" )).toString();
    settings.endGroup();

    settings.beginGroup( QStringLiteral( "AdditionalOptions" ));
    p->correctionFactor = settings.value( QStringLiteral( "correctionFactor" ), -12.8 ).toFloat();
    p->accessRateEnabled = settings.value( QStringLiteral( "accessRateEnabled" ), true ).toBool();
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

    settings.beginGroup( QStringLiteral( "Geometry" ));
    settings.setValue( QStringLiteral( "pos" ), pos() );
    settings.setValue( QStringLiteral( "size" ), size() );
    settings.endGroup();

    settings.beginGroup( QStringLiteral( "Font" ));
    Q_ASSERT( m_pConsole != nullptr );
    settings.setValue( QStringLiteral( "font" ), m_pConsole->font().toString() );
    settings.endGroup();

    settings.beginGroup( QStringLiteral( "ULayot" ));
    settings.endGroup();

    Q_ASSERT( m_pSettingsDialog != nullptr );
    const Settings p = m_pSettingsDialog->settings();
    settings.beginGroup( QStringLiteral( "SerialPort" ));
    if ( p.isChanged ) {
        settings.setValue( QStringLiteral( "PortName" ), p.name );
        settings.setValue( QStringLiteral( "baudRate" ), p.baudRate );
        settings.setValue( QStringLiteral( "dataBits" ), p.stringDataBits );
        settings.setValue( QStringLiteral( "parity" ), p.stringParity );
        settings.setValue( QStringLiteral( "stopBits" ), p.stringStopBits );
        settings.setValue( QStringLiteral( "flowControl" ), p.stringFlowControl );
    }
    settings.endGroup();

    settings.beginGroup( QStringLiteral( "AdditionalOptions" ));
    if ( p.isChanged ) {
        settings.setValue( QStringLiteral( "correctionFactor" ), QString::number( p.correctionFactor ) );
        settings.setValue( QStringLiteral( "accessRateEnabled" ), QString::number( p.accessRateEnabled ) );
    }
    settings.endGroup();
}

//!
//! \brief MainWindow::about
//! About the app
//!
void MainWindow::about()
{
    QMessageBox::about(this, QObject::tr("About SynchroTime App"),
                       QObject::tr("The <b>SynchroTime</b> application is used for fine tuning "
                                   "and calibration of the <b>RTC DS3231</b> module."
                                   "<br /><b>Version</b> %1"
                                   "<br /><b>Copyright</b> © 2021 sergej1@email.ua"
                                   "<br /><br />For more information follow the link to the "
                                   "<a href=\"https://github.com/SergejBre/SynchroTime\">project page</a>.").arg(qApp->applicationVersion()));
}

//!
//! \brief MainWindow::putRate
//! Displays the time of access to the device through the serial port.
//! \param rate of the type const float
//!
void MainWindow::putRate( const float rate )
{
    Q_ASSERT( this->m_pSettingsDialog != nullptr );
    Q_ASSERT( this->rate != nullptr );
    if ( this->m_pSettingsDialog->settings().accessRateEnabled )
        this->rate->setText( QString::number( rate, 'f', 3 ).prepend( QStringLiteral("Access rate, ms ")) );
}

//!
//! \brief MainWindow::handleError
//! Slot for handling errors when communicating with a remote device.
//! \param error of the type QString
//!
void MainWindow::handleError( const QString &error )
{
//    QObject::disconnect(m_pRTC, &RTC::portError, this, &MainWindow::handleError);
    disconnectRTC();
    QMessageBox::critical( this, QObject::tr( "Serial Port Error" ), error, QMessageBox::Ok );
}

//!
//! \brief MainWindow::connectRTC
//! Procedure for creating a separate thread for communication with a device.
//!
void MainWindow::connectRTC()
{
    m_pThread = ::new( std::nothrow ) QThread( this );
    auto p = m_pSettingsDialog->settings();
    // There is no need to specify the parent. The parent will be a thread when we move our RTC object into it.
    m_pRTC = ::new( std::nothrow ) RTC( p );
    if ( m_pThread != nullptr && m_pRTC != nullptr ) {
        // We move the RTC object to a separate thread so that synchronous pending operations do not block the main GUI thread.
        // Create a connection: Delete the RTC object when the stream ends. start the thread.
        m_pRTC->moveToThread( m_pThread );
        QObject::connect( m_pThread, SIGNAL( finished() ), m_pRTC, SLOT( deleteLater() ) );
        m_pThread->start();

        // Checking the connection.
        if ( m_pRTC->isConnected() ) {
            Q_ASSERT( m_pConsole != nullptr );
            m_pConsole->setEnabled( true );
            actionsTrigger( true );

            QObject::connect(ui->actionInformation, &QAction::triggered, m_pRTC, &RTC::informationRequestSlot);
            QObject::connect(ui->actionAdjustment, &QAction::triggered, m_pRTC, &RTC::adjustmentRequestSlot);
            QObject::connect(ui->actionCalibration, &QAction::triggered, m_pRTC, &RTC::calibrationRequestSlot);
            QObject::connect(ui->actionReset, &QAction::triggered, m_pRTC, &RTC::resetRequestSlot);
            QObject::connect(this, &MainWindow::setRegister, m_pRTC, &RTC::setRegisterRequestSlot);

            QObject::connect(m_pRTC, &RTC::getData, m_pConsole, &Console::putData);
            QObject::connect(m_pRTC, &RTC::getRate, this, &MainWindow::putRate);
            QObject::connect(m_pRTC, &RTC::portError, this, &MainWindow::handleError);

            showStatusMessage( QObject::tr( "Connected to %1 port, baud rate %2 / %3–%4–%5" )
                               .arg(p.name)
                               .arg(p.stringBaudRate)
                               .arg(p.dataBits)
                               .arg(p.parity == QSerialPort::NoParity ? 'N' : p.parity == QSerialPort::EvenParity ? 'E' : p.parity == QSerialPort::OddParity ? 'O' : p.parity == QSerialPort::SpaceParity ? 'S' : 'M')
                               .arg(p.stopBits) );
        }
        else {
            m_pThread->quit();
            m_pThread->wait( WAIT_FOR_STREAM );

            showStatusMessage( QObject::tr( "Connection error" ));
            QMessageBox::critical(this, QObject::tr( "Connection error" ),
                                  QObject::tr( "Connect the RTC device to the correct serial port, "
                                               "or set the serial port name in the port settings." ),
                                  QMessageBox::Ok);
        }
    }
    else {
        showStatusMessage( QObject::tr( "Bad allocation memory" ));
        QMessageBox::critical(this, QObject::tr( "Bad allocation memory" ),
                              QObject::tr( "Bad allocation memory, execution terminating.\n"
                                           "Advice: terminate unnecessary applications!" ),
                              QMessageBox::Ok);
    }
}

//!
//! \brief MainWindow::disconnectRTC
//! The procedure performs a correct disconnection from the device.
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
    rate->clear();
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
//! Slot for displaying the time of day.
//!
void MainWindow::tickClock()
{
    Q_ASSERT( clock != nullptr );
    clock->display( QTime::currentTime().toString() );
}

//!
//! \brief MainWindow::help slot
//! Brief description of how to use the application.
//!
void MainWindow::help()
{
    QMessageBox::information( this, QObject::tr( "Help" ),
                              QObject::tr("<h4>The Application is used for fine tuning "
                                          "and calibration of the RTC DS3231 module</h4>"
                                          "<ol><li>To select the correct <b>serial port</b>, "
                                          "you need to go to the Port Settings and select its name and parameters.</li>"
                                          "<li>Use the <b>information request</b> to get the information from DS3231 module. "
                                          "If everything is connected correctly, "
                                          "then you will get the current time of both clocks, "
                                          "the difference between the clocks in milliseconds "
                                          "(with an accuracy of ±2 ms), the value written in the offset register "
                                          "and the calculated time drift value in ppm. "
                                          "If the offset register and time drift are zero, "
                                          "then the DS3231 module has not yet been calibrated (see step 4).</li>"
                                          "<li>To set the exact time, use the <b>adjustment request</b>. "
                                          "The module clock will be synchronized with the computer time "
                                          "with an accuracy of ±1 ms. After updating the time, "
                                          "the date of the time setting will be recorded in the module's memory, "
                                          "which will allow later to determine the exact drift of the clock time.</li>"
                                          "<li>To calibrate the clock of the DS3231 module, "
                                          "use the <b>calibration request</b>. "
                                          "For the successful execution of this procedure, "
                                          "the module must be activated (see step 3) and "
                                          "it is necessary that enough time has passed so that the calculated value "
                                          "of the clock drift is well distinguishable from the rounding error. "
                                          "The algorithm of the program will calculate the amount of drift of the clock time "
                                          "and the correction factor, which will be written into the offset register. "
                                          "The clock time will also be updated. If the calibration is successful, "
                                          "the current time, drift and correction factor will be displayed.</li>"
                                          "<li>To reset the offset register to its default value and "
                                          "clear the module's memory of calibration data, use the <b>reset request</b>.</li></ol>"
                                          "For more information follow the link to the <a href=\"https://github.com/SergejBre/SynchroTime\">project page</a>.") );
}

//!
//! \brief MainWindow::aboutQt
//! A slot for issuing information about the Qt-Framework used.
void MainWindow::aboutQt()
{
    qApp->aboutQt();
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

//!
//! \brief MainWindow::setRegisterSlot
//! Slot for writing to the shift register.
//!
void MainWindow::setRegisterSlot()
{
    bool ok;
    const float value = QInputDialog::getDouble( this, QObject::tr( "Offset register modification" ),
                                         QObject::tr( "Enter a new value new value in the Offset Register:" ),
                                         0, -12.8, 12.7, 1, &ok );
    Q_ASSERT( m_pRTC != nullptr );
    if ( ok )
    {
        emit this->setRegister( value );
    }
}
