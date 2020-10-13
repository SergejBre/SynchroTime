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
#include <QMessageBox>
#include <QLabel>

//------------------------------------------------------------------------------
// Preprocessor
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Enums
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionSelect_Port->setEnabled(true);
    ui->actionQuit->setEnabled(true);

    ui->actionInformation->setEnabled(false);
    ui->actionAdjustment->setEnabled(false);
    ui->actionCalibration->setEnabled(false);
    ui->actionReset->setEnabled(false);
    ui->actionSetRegister->setEnabled(false);

    ui->actionSelect_Port->setEnabled(true);

    status = new QLabel;
    ui->statusBar->addWidget(status);

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
}

MainWindow::~MainWindow()
{
    delete ui;
}

//!
//! \brief MainWindow::readSettings
//!
void MainWindow::readSettings()
{

}

//!
//! \brief MainWindow::writeSettings
//!
void MainWindow::writeSettings() const
{

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
//! \brief MainWindow::initActionsConnections
//!
void MainWindow::initActionsConnections()
{
    QObject::connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    QObject::connect(ui->actionAbout_App, &QAction::triggered, this, &MainWindow::about);
    QObject::connect(ui->actionAbout_Qt, &QAction::triggered, qApp, &QApplication::aboutQt);
}

//!
//! \brief MainWindow::showStatusMessage
//! \param message
//!
void MainWindow::showStatusMessage(const QString &message) const
{
    status->setText(message);
}
