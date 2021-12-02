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
#include "../../qcustomplot/qcustomplot.h"
#include <QMessageBox>
#include <QLCDNumber>
#include <QLabel>
#include <QTime>
#include <QTimer>
#include <QSettings>
#include <QThread>
#include <QInputDialog>
#include <QFontDialog>
#include <QTranslator>
#include <QCloseEvent>

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
#define  NUMBER_OF_REPRESENT 5
static float m_Stack[NUMBER_OF_REPRESENT];  //!< FIFO working stack of N representatives.

//------------------------------------------------------------------------------
// Function definitions
//------------------------------------------------------------------------------
//! \brief MainWindow::push
//!
//! Function for pushing a new value onto the FIFO stack
//!
//! \param stack a Container (of the type float Array, vector<float>, ect.)
//!
//! \param value of the type const float
template <typename Container>
void MainWindow::push( Container &stack, const float value )
{
    for ( auto it = std::begin(stack); it != std::end(stack) - 1; ++it ) {
        *(it) = *(it+1);
    }
    *(std::end(stack) - 1) = value;
}

//! \brief MainWindow::fill
//!
//! The function fills the stack with a value.
//!
//! \param stack a Container (of the type float Array, vector<float>, ect.)
//!
//! \param value of the type const float
template <typename Container>
void MainWindow::fill( Container &stack, const float value )
{
    for ( auto it = std::begin(stack); it != std::end(stack); ++it ) {
        *(it) = value;
    }
}

//! \brief MainWindow::mean - Moving average method.
//!
//! The essence of the method is to calculate the averaged data over a certain period of time.
//!
//! \param stack a Container (of the type float Array, vector<float>, ect.)
//!
//! \return mean of the type float
template <typename Container>
float MainWindow::mean( const Container &stack )
{
    return std::accumulate(std::begin(stack), std::end(stack), .0) / (std::end(stack) - std::begin(stack));
}

//!
//! \brief MainWindow::MainWindow
//! Constructor of the main application window.
//! \param parent of the type QWidget
//!
MainWindow::MainWindow( QWidget *parent ) :
    QMainWindow( parent ),
    ui( new Ui::MainWindow ),
    m_pConsole( nullptr ),
    m_pSettings( nullptr ),
    m_pSettingsDialog( nullptr ),
    m_pThread( nullptr ),
    m_pRTC( nullptr ),
    m_pCPBars( nullptr ),
    m_detectDelayFlag( true ),
    m_pTranslator( nullptr )
{
    ui->setupUi( this );
    this->actionsTrigger( false );
    ui->actionQuit->setEnabled( true );

    m_pConsole = ui->console;
    m_pConsole->setEnabled( false );
    m_pSettings = new Settings();
    this->readSettings();
    this->changeTranslator( postfix );
    m_pSettingsDialog = new SettingsDialog( m_pSettings, this );
    QObject::connect( m_pSettingsDialog, &SettingsDialog::settingsError, this, &MainWindow::handleSettingsError );

    m_pCPBars = new QCPBars(ui->customPlotBars->xAxis, ui->customPlotBars->yAxis);
    initBars();
    if ( !this->m_pSettings->detectDelayEnabled ) {
        ui->customPlotBars->hide();
    }

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
    QObject::connect(ui->actionClean_Up, &QAction::triggered, m_pConsole, &Console::clear);
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
    if ( m_pTranslator != nullptr ) {
        delete m_pTranslator;
    }
    if ( m_pSettings != nullptr ) {
        delete m_pSettings;
    }

    delete ui;
}

//!
//! \brief MainWindow::readSettings
//! The function reads the parameters necessary for the user interface that were saved in the previous session.
//!
//! The following important parameters will be read as:
//! - the position of the window on the screen and window size,
//! - interface font and its size,
//! - the user interface settings (overwrite of the data, recurse of dir's, etc.),
//! - the serial port options,
//! - and other additional parameters.
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

    settings.beginGroup( QStringLiteral( "translations" ));
    this->postfix = settings.value( QStringLiteral( "postfix" ), QStringLiteral( "en" )).toString();
    settings.endGroup();

    settings.beginGroup( QStringLiteral( "ULayout" ));
    settings.endGroup();

    Q_ASSERT( m_pSettings != nullptr );
    settings.beginGroup( QStringLiteral( "SerialPort" ));
#ifdef Q_OS_WIN
    m_pSettings->name = settings.value( QStringLiteral( "portName" ), QStringLiteral( "COM5" )).toString();
#else
    m_pSettings->name = settings.value( QStringLiteral( "portName" ), QStringLiteral( "ttyUSB0" )).toString();
#endif
    m_pSettings->baudRate = settings.value( QStringLiteral( "baudRate" ), 115200 ).toUInt();
    m_pSettings->stringBaudRate = settings.value( QStringLiteral( "baudRate" ), 115200 ).toString();
    m_pSettings->stringDataBits = settings.value( QStringLiteral( "dataBits" ), 8 ).toString();
    m_pSettings->stringParity = settings.value( QStringLiteral( "parity" ), QStringLiteral( "NoParity" )).toString();
    m_pSettings->stringStopBits = settings.value( QStringLiteral( "stopBits" ), 1 ).toString();
    m_pSettings->stringFlowControl = settings.value( QStringLiteral( "flowControl" ), QStringLiteral( "None" )).toString();
    settings.endGroup();

    settings.beginGroup( QStringLiteral( "AdditionalOptions" ));
    m_pSettings->correctionFactor = settings.value( QStringLiteral( "correctionFactor" ), -12.8 ).toFloat();
    m_pSettings->detectDelayEnabled = settings.value( QStringLiteral( "detectDelayEnabled" ), true ).toBool();
    m_pSettings->statusControlEnabled = settings.value( QStringLiteral( "statusControlEnabled" ), true ).toBool();
    m_pSettings->requestRate = settings.value( QStringLiteral( "requestRate" ), 500 ).toInt();
    settings.endGroup();
}

//!
//! \brief MainWindow::writeSettings
//! The function saves the user interface parameters that have been changed by the user in the current session.
//!
//! The following important parameters will be updated as:
//! - the position of the window on the screen and window size,
//! - interface font and its size,
//! - the user interface settings (overwrite of the data, recurse of dir's, etc.),
//! - the serial port options,
//! - and other additional parameters.
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

    settings.beginGroup( QStringLiteral( "translations" ));
    settings.setValue( QStringLiteral( "postfix" ), this->postfix );
    settings.endGroup();

    settings.beginGroup( QStringLiteral( "ULayot" ));
    settings.endGroup();

    Q_ASSERT( m_pSettings != nullptr );
    settings.beginGroup( QStringLiteral( "SerialPort" ));
    if ( m_pSettings->isChanged ) {
        settings.setValue( QStringLiteral( "PortName" ), m_pSettings->name );
        settings.setValue( QStringLiteral( "baudRate" ), m_pSettings->baudRate );
        settings.setValue( QStringLiteral( "dataBits" ), m_pSettings->stringDataBits );
        settings.setValue( QStringLiteral( "parity" ), m_pSettings->stringParity );
        settings.setValue( QStringLiteral( "stopBits" ), m_pSettings->stringStopBits );
        settings.setValue( QStringLiteral( "flowControl" ), m_pSettings->stringFlowControl );
    }
    settings.endGroup();

    settings.beginGroup( QStringLiteral( "AdditionalOptions" ));
    if ( m_pSettings->isChanged ) {
        settings.setValue( QStringLiteral( "correctionFactor" ), QString::number( m_pSettings->correctionFactor ) );
        settings.setValue( QStringLiteral( "detectDelayEnabled" ), QString::number( m_pSettings->detectDelayEnabled ) );
        settings.setValue( QStringLiteral( "statusControlEnabled" ), QString::number( m_pSettings->statusControlEnabled ) );
        settings.setValue( QStringLiteral( "requestRate" ), QString::number( m_pSettings->requestRate ) );
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
                       QObject::tr("The application is used to adjust the exact time "
                                   "and to calibrate the <b>RTC DS3231</b> modules."
                                   "<br /><b>Version</b> %1"
                                   "<br /><b>Copyright</b> © 2021 sergej1@email.ua"
                                   "<br /><br />For more information follow the link to the "
                                   "<a href=\"https://github.com/SergejBre/SynchroTime\">project page</a>.").arg(qApp->applicationVersion()));
}

//! \brief MainWindow::putDelay
//!
//! Displays the delay of access to the device through the serial port.
//!
//! \param delay of the type const float
void MainWindow::putDelay( const float delay )
{
    static float max = 0;
    Q_ASSERT( this->m_pSettings != nullptr );
    if ( m_detectDelayFlag ) {
        fill( m_Stack, 0 );
        m_detectDelayFlag = false;
        max = 0;
    }
    else {
        push( m_Stack, delay );
    }

    m_pCPBars->setData( QVector<double>(1, 1), QVector<double>(1, mean(m_Stack)) );

    if ( delay > max ) {
        max = delay;
        m_pCPBars->rescaleAxes();
        ui->customPlotBars->yAxis->setRange(0, max);
    }
    ui->customPlotBars->replot();
}

//!
//! \brief MainWindow::handleError
//! Slot for handling errors when communicating with a remote device.
//! \param error of the type QString
//!
void MainWindow::handleError( const QString &error )
{
    disconnectRTC();
    QMessageBox::critical( this, QObject::tr( "Serial Port Error" ), error, QMessageBox::Ok );
}

//! \brief MainWindow::handleSettingsError
//!
//! Slot for handling errors that occurred when changing the settings of the serial port interface.
//!
//! \param error of the type QString&
void MainWindow::handleSettingsError( const QString &error )
{
    if ( m_pSettings->detectDelayEnabled && ui->customPlotBars->isHidden() ) {
        ui->customPlotBars->setVisible( true );
    }
    else if ( !m_pSettings->detectDelayEnabled && ui->customPlotBars->isVisible() ) {
        ui->customPlotBars->hide();
    }
    showStatusMessage( error );
}

//! \brief MainWindow::initBars
//!
void MainWindow::initBars()
{
    Q_ASSERT( m_pCPBars != nullptr );
    m_pCPBars->setAntialiased( false );
    m_pCPBars->setPen( QPen( QColor(0, 168, 140).lighter(130)) );
    m_pCPBars->setBrush( QColor(0, 168, 140) );
    m_pCPBars->setData( QVector<double>(1, 1), QVector<double>(1, 0) );
    m_pCPBars->rescaleAxes();

    QLinearGradient plotGradient;
    plotGradient.setStart(0, 128);
    plotGradient.setFinalStop(0, 350);
    plotGradient.setColorAt(1, Qt::white);
    plotGradient.setColorAt(0, QColor(135, 206, 235).lighter(135) );
    ui->customPlotBars->axisRect()->setBackground(plotGradient);

    ui->customPlotBars->plotLayout()->insertRow(0);
    ui->customPlotBars->plotLayout()->addElement(0, 0, new QCPTextElement(ui->customPlotBars, "Max/Average\nAccess delay\n[ms]", QFont(font().family(), 8)));
    ui->customPlotBars->xAxis->setVisible( false );
    ui->customPlotBars->yAxis->grid()->setPen( QPen( Qt::gray, 1, Qt::DotLine ));
    ui->customPlotBars->yAxis->setTickLabelFont( QFont(font().family(), 8) );
    ui->customPlotBars->yAxis->setPadding(-2); // a bit more space to the left border
    ui->customPlotBars->xAxis->setRange(0, 1);
    ui->customPlotBars->yAxis->setRange(0, 10);
    ui->customPlotBars->replot();
}

//!
//! \brief MainWindow::connectRTC
//! Procedure for creating a separate thread for communication with a device.
//!
void MainWindow::connectRTC()
{
    m_pThread = ::new( std::nothrow ) QThread( this );
    // There is no need to specify the parent. The parent will be a thread when we move our RTC object into it.
    Q_ASSERT( m_pSettings != nullptr );
    m_pRTC = ::new( std::nothrow ) RTC( m_pSettings );
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
            QObject::connect(m_pRTC, &RTC::getDelay, this, &MainWindow::putDelay);
            QObject::connect(m_pRTC, &RTC::portError, this, &MainWindow::handleError);

            showStatusMessage( QObject::tr( "Connected to %1 port, baud rate %2 / %3–%4–%5" )
                               .arg(m_pSettings->name)
                               .arg(m_pSettings->stringBaudRate)
                               .arg(m_pSettings->dataBits)
                               .arg(m_pSettings->parity == QSerialPort::NoParity ? 'N' : m_pSettings->parity == QSerialPort::EvenParity ? 'E' : m_pSettings->parity == QSerialPort::OddParity ? 'O' : m_pSettings->parity == QSerialPort::SpaceParity ? 'S' : 'M')
                               .arg(m_pSettings->stopBits) );
            m_pRTC->infoFromDevice();
        }
        else {
            m_pThread->quit();
            m_pThread->wait( WAIT_FOR_STREAM );

            showStatusMessage( QObject::tr( "Connection error with %1 port" ).arg( m_pSettings->name ));
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

//! \brief MainWindow::disconnectRTC
//!
//! The procedure performs a correct disconnection from the device.
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

    showStatusMessage( QObject::tr( "Disconnected from port %1" ).arg( this->m_pSettings->name ));
    m_detectDelayFlag = true;
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
                              QObject::tr("<h4>The Application is used to adjust "
                                          "and calibration the RTC DS3231 module</h4>"
                                          "<ol><li>To select the correct <b>serial port</b>, "
                                          "you need to go to the Port Settings and select its name and parameters.</li>"
                                          "<li>Use the <b>information request</b> to get the information from DS3231 module. "
                                          "If everything is connected correctly, "
                                          "then you will get the current time of both clocks, "
                                          "the difference between the clocks in milliseconds "
                                          "(with an accuracy of ±2 ms), the value written in the Aging register "
                                          "and the calculated time drift value in ppm. "
                                          "If the Aging register and time drift are zero, "
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
                                          "and the correction factor, which will be written into the Aging register. "
                                          "The clock time will also be updated. If the calibration is successful, "
                                          "the current time, drift and correction factor will be displayed.</li>"
                                          "<li>To reset the Aging register to its default value and "
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
    QFont font = QFontDialog::getFont( &selected, m_pConsole->font(), this, QObject::tr( "Select Font" ) );

    if ( selected )
    {
        this->m_pConsole->setFont( font );
    }
}

//!
//! \brief MainWindow::setRegisterSlot
//! Slot for writing to the shift register.
//!
void MainWindow::setRegisterSlot()
{
    bool ok;
    const float value = QInputDialog::getDouble( this, QObject::tr( "Aging register modification" ),
                                         QObject::tr( "Enter a new value new value in the Aging Register:" ),
                                         0, -12.8, 12.7, 1, &ok );
    Q_ASSERT( m_pRTC != nullptr );
    if ( ok )
    {
        emit this->setRegister( value );
    }
}

//!
//! \brief MainWindow::on_actionEnglish_triggered
//! Slot for reacting to the choice of the English interface language
//!
void MainWindow::on_actionEnglish_triggered()
{
    if ( postfix.compare("en") ) {
        this->postfix = QStringLiteral("en");
        changeTranslator( postfix );
    }
    else {
        ui->actionEnglish->setChecked( true );
    }
}

//!
//! \brief MainWindow::on_actionGerman_triggered
//! Slot for reacting to the choice of the German interface language
//!
void MainWindow::on_actionGerman_triggered()
{
    if ( postfix.compare("de_DE") ) {
        this->postfix = QStringLiteral("de_DE");
        changeTranslator( postfix );
    }
    else {
        ui->actionGerman->setChecked( true );
    }
}

//!
//! \brief MainWindow::on_actionRussian_triggered
//! Slot for reacting to the choice of the Russian interface language
//!
void MainWindow::on_actionRussian_triggered()
{
    if ( postfix.compare("ru_RU") ) {
        this->postfix = QStringLiteral("ru_RU");
        changeTranslator( postfix );
    }
    else {
        ui->actionRussian->setChecked( true );
    }
}

//!
//! \brief MainWindow::changeTranslator
//! The Function creates an instance for the given localization.
//!
//! \param postfix of type QString (Suffix denoting language localization: "en", "de_DE", "ru_RU", ect)
//!
void MainWindow::changeTranslator( const QString &postfix )
{
    if ( m_pTranslator == nullptr ) {
        m_pTranslator = new QTranslator( this );
    }
    else {
        qApp->removeTranslator( m_pTranslator );
    }

    if ( m_pTranslator->load( QStringLiteral(":/i18n/") + qApp->applicationName() + QLatin1String("_") + postfix) \
         || !postfix.compare( "en" ) )
    {
        qApp->installTranslator( m_pTranslator );
    }
    this->setLanguage( postfix );
}

//!
//! \brief MainWindow::setLanguage
//! The Function checks localization.
//!
//! \param postfix of type QString (Suffix denoting language localization: "en", "de_DE", "ru_RU", ect)
//!
void MainWindow::setLanguage(const QString &postfix)
{
    uncheck();
    if ( !postfix.compare( "en" ) ) {
        ui->actionEnglish->setChecked( true );
    }
    else if ( !postfix.compare( "de_DE" ) ) {
        ui->actionGerman->setChecked( true );
    }
    else if ( !postfix.compare( "ru_RU" ) ) {
        ui->actionRussian->setChecked( true );
    }
}

//!
//! \brief MainWindow::changeEvent
//! This function intercepts events when the user interface language is changed.
//!
//! \param event of type QEvent*
//!
void MainWindow::changeEvent(QEvent *event)
{
    if ( event->type() == QEvent::LanguageChange ) {
        ui->menuConnect->setTitle( QObject::tr( "Connect" ));
        ui->actionConnect->setText( QObject::tr( "C&onnect" ));
        ui->actionConnect->setToolTip( QObject::tr( "Connect to serial port" ));
        ui->actionDisconnect->setText( QObject::tr( "&Disconnect" ));
        ui->actionDisconnect->setToolTip( QObject::tr( "Disconnect from serial port" ));
        ui->actionQuit->setText( QObject::tr( "&Quit" ));
        ui->menuRequest->setTitle( QObject::tr( "Request" ) );
        ui->actionInformation->setText( QObject::tr( "&Information" ));
        ui->actionInformation->setToolTip( QObject::tr( "Read information from RTC" ));
        ui->actionAdjustment->setText( QObject::tr( "Adjustment" ));
        ui->actionCalibration->setText( QObject::tr( "Calibration" ));
        ui->actionReset->setText( QObject::tr( "Reset" ));
        ui->actionSetRegister->setText( QObject::tr( "Set Register" ));
        ui->menuTools->setTitle( QObject::tr( "Tools" ));
        ui->actionPort_Setting->setText( QObject::tr( "&Port Setting" ));
        ui->actionPort_Setting->setToolTip( QObject::tr( "Configure serial port" ));
        ui->actionSelect_Font->setText( QObject::tr( "Select Font" ));
        ui->actionClean_Up->setText( QObject::tr( "C&lean Up" ));
        ui->actionClean_Up->setToolTip( QObject::tr( "Clean up data" ));
        ui->menuSelect_Language->setTitle( QObject::tr( "Select Language" ));
        ui->actionEnglish->setText( QObject::tr( "English" ));
        ui->actionGerman->setText( QObject::tr( "German" ));
        ui->actionRussian->setText( QObject::tr( "Russian" ));
        ui->menuHelp->setTitle( QObject::tr( "Help" ));
        ui->actionContents->setText( QObject::tr( "Help" ));
        ui->actionAbout_Qt->setText( QObject::tr( "Qt-Framework" ));
        ui->actionAbout_App->setText( QObject::tr( "About App" ));
    }
    else {
        QMainWindow::changeEvent( event );
    }
}

//!
//! \brief MainWindow::uncheck
//! The help function that removes the check mark from all checkboxes.
//!
void MainWindow::uncheck()
{
    ui->actionEnglish->setChecked( false );
    ui->actionGerman->setChecked( false );
    ui->actionRussian->setChecked( false );
}

