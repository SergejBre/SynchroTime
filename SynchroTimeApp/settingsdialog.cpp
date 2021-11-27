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
//! \file settingsdialog.cpp
//!
//! \brief The file contains the definition of the constructor and methods of the SettingsDialog class.
//!
//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "serialportsettings.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QIntValidator>
#include <QLineEdit>

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

static const char blankString[] = QT_TRANSLATE_NOOP( "SettingsDialog", "N/A" );

//------------------------------------------------------------------------------
// Function definitions
//------------------------------------------------------------------------------

//! \brief SettingsDialog::SettingsDialog
//!
//! Standard constructor for the SettingsDialog class.
//!
//! \param settings of the type Settings *const
//! \param parent of the type QWidget *
SettingsDialog::SettingsDialog( Settings *const settings, QWidget *parent ) :
    QDialog( parent ),
    ui( new Ui::SettingsDialog ),
    m_pSettings( settings ),
    m_ErrorFlag( false )
{
    ui->setupUi( this );

    m_pIntValidator = new QIntValidator( 1200, 4000000, this );

    ui->baudRateBox->setInsertPolicy( QComboBox::NoInsert );

    QObject::connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    QObject::connect(this, &QDialog::accepted, this, &SettingsDialog::apply);
    QObject::connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    QObject::connect(this, &QDialog::rejected, this, &SettingsDialog::cansel);
    QObject::connect(ui->resetButton, &QPushButton::clicked, this, &SettingsDialog::reset);
    QObject::connect(ui->serialPortInfoListBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                     this, &SettingsDialog::showPortInfo);
    QObject::connect(ui->baudRateBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                     this, &SettingsDialog::checkCustomBaudRatePolicy);
    QObject::connect(ui->serialPortInfoListBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                     this, &SettingsDialog::checkCustomDevicePathPolicy);

    fillPortsParameters();
    fillPortsInfo();
    fillSettingsUi();
    updateSettings();
}

//! \brief SettingsDialog::~SettingsDialog
//!
//! Destructor of the SettingsDialog class.
SettingsDialog::~SettingsDialog()
{
    delete ui;
}

//! \brief SettingsDialog::fillSettingsUi
//!
//! The function initializes the ui form using the stored parameters.
void SettingsDialog::fillSettingsUi()
{
    int index = ui->serialPortInfoListBox->findText( m_pSettings->name );
    if ( index > -1 ) {
        ui->serialPortInfoListBox->setCurrentIndex( index );
    }
    index = ui->baudRateBox->findText( m_pSettings->stringBaudRate );
    if ( index > -1 ) {
        ui->baudRateBox->setCurrentIndex( index );
    }
    else
    {
        int pos = 0;
        if ( m_pIntValidator->validate( m_pSettings->stringBaudRate, pos ) == QValidator::Acceptable ) {
            index = ui->baudRateBox->count() - 1;
            ui->baudRateBox->setCurrentIndex( index );
            ui->baudRateBox->setItemText( index, m_pSettings->stringBaudRate );
        }
        else {
            index = ui->baudRateBox->findData( QSerialPort::Baud115200 );
            ui->baudRateBox->setCurrentIndex( index );
        }
    }
    index = ui->dataBitsBox->findText( m_pSettings->stringDataBits );
    if ( index > -1 ) {
        ui->dataBitsBox->setCurrentIndex( index );
    }
    index = ui->parityBox->findText( m_pSettings->stringParity );
    if ( index > -1 ) {
        ui->parityBox->setCurrentIndex( index );
    }
    index = ui->stopBitsBox->findText( m_pSettings->stringStopBits );
    if ( index > -1 ) {
        ui->stopBitsBox->setCurrentIndex( index );
    }
    index = ui->flowControlBox->findText( m_pSettings->stringFlowControl );
    if ( index > -1 ) {
        ui->flowControlBox->setCurrentIndex( index );
    }
    ui->factorDoubleSpinBox->setValue( m_pSettings->correctionFactor );
    ui->detectDelayCheckBox->setChecked( m_pSettings->detectDelayEnabled );
    ui->statusControlCheckBox->setChecked( m_pSettings->statusControlEnabled );
    ui->requestRateSpinBox->setValue( m_pSettings->requestRate );
    ui->detectDelayCheckBox->setEnabled( m_pSettings->statusControlEnabled );
    ui->requestRateSpinBox->setEnabled( m_pSettings->statusControlEnabled );
}

//! \brief SettingsDialog::show
//!
//! Shows the widget and its child widgets.
void SettingsDialog::show()
{
    fillPortsInfo();
    QWidget::show();
}

//! \brief SettingsDialog::showPortInfo
//!
//! \param idx of the type int
void SettingsDialog::showPortInfo(int idx)
{
    if (idx == -1)
        return;

    QStringList list = ui->serialPortInfoListBox->itemData(idx).toStringList();
    ui->descriptionLabel->setText( QObject::tr( "Description: %1" ).arg( list.count() > 1 ? list.at(1) : tr(blankString) ));
    ui->manufacturerLabel->setText( QObject::tr( "Manufacturer: %1" ).arg( list.count() > 2 ? list.at(2) : tr(blankString) ));
    ui->serialNumberLabel->setText( QObject::tr( "Serial number: %1" ).arg( list.count() > 3 ? list.at(3) : tr(blankString) ));
    ui->locationLabel->setText( QObject::tr( "Location: %1" ).arg( list.count() > 4 ? list.at(4) : tr(blankString) ));
    ui->vidLabel->setText( QObject::tr( "Vendor Identifier: %1" ).arg( list.count() > 5 ? list.at(5) : tr(blankString) ));
    ui->pidLabel->setText( QObject::tr( "Product Identifier: %1" ).arg( list.count() > 6 ? list.at(6) : tr(blankString) ));
}

//! \brief SettingsDialog::apply
//!
//! Changes to settings will be accepted.
void SettingsDialog::apply()
{
    updateSettings();
    m_pSettings->isChanged = true;
    if ( m_ErrorFlag ) {
        m_ErrorFlag = false;
    }
    else {
        emit settingsError( QObject::tr( "The settings have been successfully updated" ));
    }
    QWidget::close();
}

//! \brief SettingsDialog::cansel
//!
//! Changes to settings have been reset to previous.
void SettingsDialog::cansel()
{
    fillSettingsUi();
    emit settingsError( QObject::tr( "The settings have not been updated" ) );
    QWidget::close();
}

//!
//! \brief SettingsDialog::reset
//!
//! The function sets the parameters of the UART interface by default:
//! - QSerialPort::BaudRate     Baud115200
//! - QSerialPort::DataBits     Data8
//! - QQSerialPort::Parity      NoParity
//! - QSerialPort::StopBits     OneStop
//! - QSerialPort::FlowControl  NoFlowControl
//! .
void SettingsDialog::reset()
{
    int index = ui->baudRateBox->findData( QSerialPort::Baud115200 );
    ui->baudRateBox->setCurrentIndex( index );

    index = ui->dataBitsBox->findData( QSerialPort::Data8 );
    ui->dataBitsBox->setCurrentIndex( index );

    index = ui->parityBox->findData( QSerialPort::NoParity );
    ui->parityBox->setCurrentIndex( index );

    index = ui->stopBitsBox->findData( QSerialPort::OneStop );
    ui->stopBitsBox->setCurrentIndex( index );

    index = ui->flowControlBox->findData( QSerialPort::NoFlowControl );
    ui->flowControlBox->setCurrentIndex( index );
}

//! \brief SettingsDialog::checkCustomBaudRatePolicy
//!
//! \param idx of the type int
void SettingsDialog::checkCustomBaudRatePolicy(int idx)
{
    int index = ui->baudRateBox->count() - 1;
    bool isCustomBaudRate = !ui->baudRateBox->itemData(idx).isValid();
    ui->baudRateBox->setEditable(isCustomBaudRate);
    if ( isCustomBaudRate && idx == index ) {
        ui->baudRateBox->clearEditText();
        QLineEdit *edit = ui->baudRateBox->lineEdit();
        edit->setValidator(m_pIntValidator);
    }
}

//! \brief SettingsDialog::checkCustomDevicePathPolicy
//!
//! \param idx ot the type int
void SettingsDialog::checkCustomDevicePathPolicy(int idx)
{
    bool isCustomPath = !ui->serialPortInfoListBox->itemData(idx).isValid();
    ui->serialPortInfoListBox->setEditable(isCustomPath);
    if (isCustomPath)
        ui->serialPortInfoListBox->clearEditText();
}

//! \brief SettingsDialog::on_statusControlCheckBox_clicked
//!
//! The slot receives the clicked signal and manages the associated user interface objects.
//!
//! \param checked of the type bool, clicked state?
void SettingsDialog::on_statusControlCheckBox_clicked( bool checked )
{
    ui->detectDelayCheckBox->setEnabled( checked );
    ui->requestRateSpinBox->setEnabled( checked );
}

//! \brief SettingsDialog::fillPortsParameters
//!
//! Filling the lists of parameters of the serial port.
void SettingsDialog::fillPortsParameters()
{
    ui->baudRateBox->addItem( QStringLiteral( "9600" ), QSerialPort::Baud9600 );
    ui->baudRateBox->addItem( QStringLiteral( "19200" ), QSerialPort::Baud19200 );
    ui->baudRateBox->addItem( QStringLiteral( "38400" ), QSerialPort::Baud38400 );
    ui->baudRateBox->addItem( QStringLiteral( "57600" ), QSerialPort::Baud57600 );
    ui->baudRateBox->addItem( QStringLiteral( "115200" ), QSerialPort::Baud115200 );
    ui->baudRateBox->addItem( QObject::tr( "Custom" ) );

    ui->dataBitsBox->addItem( QStringLiteral( "5" ), QSerialPort::Data5 );
    ui->dataBitsBox->addItem( QStringLiteral( "6" ), QSerialPort::Data6 );
    ui->dataBitsBox->addItem( QStringLiteral( "7" ), QSerialPort::Data7 );
    ui->dataBitsBox->addItem( QStringLiteral( "8" ), QSerialPort::Data8 );

    ui->parityBox->addItem( QStringLiteral( "NoParity" ), QSerialPort::NoParity );
    ui->parityBox->addItem( QStringLiteral( "EvenParity" ), QSerialPort::EvenParity );
    ui->parityBox->addItem( QStringLiteral( "OddParity" ), QSerialPort::OddParity );
    ui->parityBox->addItem( QStringLiteral( "SpaceParity" ), QSerialPort::SpaceParity );
    ui->parityBox->addItem( QStringLiteral( "MarkParity" ), QSerialPort::MarkParity );

    ui->stopBitsBox->addItem( QStringLiteral( "1" ), QSerialPort::OneStop );
#ifdef Q_OS_WIN
    ui->stopBitsBox->addItem( QStringLiteral( "1.5" ), QSerialPort::OneAndHalfStop );
#endif
    ui->stopBitsBox->addItem( QStringLiteral( "2" ), QSerialPort::TwoStop );

    ui->flowControlBox->addItem( QStringLiteral( "None" ), QSerialPort::NoFlowControl );
    ui->flowControlBox->addItem( QStringLiteral( "RTS/CTS" ), QSerialPort::HardwareControl );
    ui->flowControlBox->addItem( QStringLiteral( "XON/XOFF" ), QSerialPort::SoftwareControl );
}

//! \brief SettingsDialog::fillPortsInfo
//!
//! Search for all available serial ports on the system.
//! Search for the information about the serial port.
void SettingsDialog::fillPortsInfo()
{
    ui->serialPortInfoListBox->clear();
    QString description;
    QString manufacturer;
    QString serialNumber;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = info.serialNumber();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);

        ui->serialPortInfoListBox->addItem(list.first(), list);
    }

    ui->serialPortInfoListBox->addItem( QObject::tr( "Custom" ));
}

//! \brief SettingsDialog::updateSettings
//!
//! Updates all parameters of the serial port.
void SettingsDialog::updateSettings()
{
    m_pSettings->name = ui->serialPortInfoListBox->currentText();

    int index = ui->baudRateBox->count() - 1;
    if ( ui->baudRateBox->currentIndex() == index ) {
        int pos = 0;
        QString str = ui->baudRateBox->currentText();
        if ( m_pIntValidator->validate( str, pos ) == QValidator::Acceptable ) {
            m_pSettings->baudRate = ui->baudRateBox->currentText().toInt();
        }
        else {
            index = ui->baudRateBox->findData( QSerialPort::Baud115200 );
            ui->baudRateBox->setCurrentIndex( index );
            m_pSettings->baudRate = static_cast<QSerialPort::BaudRate>(
                        ui->baudRateBox->itemData(ui->baudRateBox->currentIndex()).toInt());
            m_ErrorFlag = true;
            emit settingsError( QObject::tr( "Invalid serial port baud rate! The setting is reset to default" ));
        }
    }
    else {
        m_pSettings->baudRate = static_cast<QSerialPort::BaudRate>(
                    ui->baudRateBox->itemData(ui->baudRateBox->currentIndex()).toInt());
    }
    m_pSettings->stringBaudRate = QString::number(m_pSettings->baudRate);

    m_pSettings->dataBits = static_cast<QSerialPort::DataBits>(
                ui->dataBitsBox->itemData(ui->dataBitsBox->currentIndex()).toInt());
    m_pSettings->stringDataBits = ui->dataBitsBox->currentText();

    m_pSettings->parity = static_cast<QSerialPort::Parity>(
                ui->parityBox->itemData(ui->parityBox->currentIndex()).toInt());
    m_pSettings->stringParity = ui->parityBox->currentText();

    m_pSettings->stopBits = static_cast<QSerialPort::StopBits>(
                ui->stopBitsBox->itemData(ui->stopBitsBox->currentIndex()).toInt());
    m_pSettings->stringStopBits = ui->stopBitsBox->currentText();

    m_pSettings->flowControl = static_cast<QSerialPort::FlowControl>(
                ui->flowControlBox->itemData(ui->flowControlBox->currentIndex()).toInt());
    m_pSettings->stringFlowControl = ui->flowControlBox->currentText();

//    m_pSettings->localEchoEnabled = ui->localEchoCheckBox->isChecked();
    m_pSettings->detectDelayEnabled = ui->detectDelayCheckBox->isChecked();
    m_pSettings->statusControlEnabled = ui->statusControlCheckBox->isChecked();
    m_pSettings->requestRate = ui->requestRateSpinBox->value();
    m_pSettings->correctionFactor = static_cast<float>( ui->factorDoubleSpinBox->value() );
}

