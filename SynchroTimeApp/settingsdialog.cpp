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
// Function Prototypes
//------------------------------------------------------------------------------

//!
//! \brief SettingsDialog::SettingsDialog
//! \param parent
//!
SettingsDialog::SettingsDialog( QWidget *parent ) :
    QDialog( parent ),
    ui( new Ui::SettingsDialog )
{
    ui->setupUi( this );

    intValidator = new QIntValidator( 0, 4000000, this );

    ui->baudRateBox->setInsertPolicy( QComboBox::NoInsert );

    QObject::connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::apply);
    QObject::connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::cansel);
    QObject::connect(ui->resetButton, &QPushButton::clicked, this, &SettingsDialog::reset);
    QObject::connect(ui->serialPortInfoListBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                     this, &SettingsDialog::showPortInfo);
    QObject::connect(ui->baudRateBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                     this, &SettingsDialog::checkCustomBaudRatePolicy);
    QObject::connect(ui->serialPortInfoListBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                     this, &SettingsDialog::checkCustomDevicePathPolicy);

    fillPortsParameters();
    fillPortsInfo();

    updateSettings();
}

//!
//! \brief SettingsDialog::~SettingsDialog
//!
SettingsDialog::~SettingsDialog()
{
    delete ui;
}

//!
//! \brief SettingsDialog::settings
//! \return
//!
Settings_t SettingsDialog::settings() const
{
    return currentSettings;
}

Settings_t *SettingsDialog::serialPortSettings()
{
    return &currentSettings;
}

//!
//! \brief SettingsDialog::fillSettingsUi
//!
void SettingsDialog::fillSettingsUi()
{
    int index = ui->serialPortInfoListBox->findText( currentSettings.name );
    if ( index > -1 ) {
        ui->serialPortInfoListBox->setCurrentIndex( index );
    }
    index = ui->baudRateBox->findText( currentSettings.stringBaudRate );
    if ( index > -1 ) {
        ui->baudRateBox->setCurrentIndex( index );
    }
    else if ( currentSettings.baudRate > 0 )
    {
        int currentIndex = ui->baudRateBox->count() - 1;
        ui->baudRateBox->setCurrentIndex( currentIndex );
        ui->baudRateBox->setItemText( currentIndex, currentSettings.stringBaudRate );
    }
    index = ui->dataBitsBox->findText( currentSettings.stringDataBits );
    if ( index > -1 ) {
        ui->dataBitsBox->setCurrentIndex( index );
    }
    index = ui->parityBox->findText( currentSettings.stringParity );
    if ( index > -1 ) {
        ui->parityBox->setCurrentIndex( index );
    }
    index = ui->stopBitsBox->findText( currentSettings.stringStopBits );
    if ( index > -1 ) {
        ui->stopBitsBox->setCurrentIndex( index );
    }
    index = ui->flowControlBox->findText( currentSettings.stringFlowControl );
    if ( index > -1 ) {
        ui->flowControlBox->setCurrentIndex( index );
    }
}

//!
//! \brief SettingsDialog::show
//!
void SettingsDialog::show()
{
    fillPortsInfo();
    QWidget::show();
}

//!
//! \brief SettingsDialog::showPortInfo
//! \param idx
//!
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

//!
//! \brief SettingsDialog::apply
//!
void SettingsDialog::apply()
{
    updateSettings();
    currentSettings.isChanged = true;
    QWidget::hide();
}

//!
//! \brief SettingsDialog::cansel
//!
void SettingsDialog::cansel()
{
    fillSettingsUi();
    QWidget::hide();
}

//!
//! \brief SettingsDialog::reset
//!
void SettingsDialog::reset()
{
    ui->baudRateBox->setCurrentIndex(3);
    ui->dataBitsBox->setCurrentIndex(3);
    ui->parityBox->setCurrentIndex(0);
    ui->stopBitsBox->setCurrentIndex(0);
    ui->flowControlBox->setCurrentIndex(0);
}

//!
//! \brief SettingsDialog::checkCustomBaudRatePolicy
//! \param idx
//!
void SettingsDialog::checkCustomBaudRatePolicy(int idx)
{
    bool isCustomBaudRate = !ui->baudRateBox->itemData(idx).isValid();
    ui->baudRateBox->setEditable(isCustomBaudRate);
    if (isCustomBaudRate) {
        ui->baudRateBox->clearEditText();
        QLineEdit *edit = ui->baudRateBox->lineEdit();
        edit->setValidator(intValidator);
    }
}

//!
//! \brief SettingsDialog::checkCustomDevicePathPolicy
//! \param idx
//!
void SettingsDialog::checkCustomDevicePathPolicy(int idx)
{
    bool isCustomPath = !ui->serialPortInfoListBox->itemData(idx).isValid();
    ui->serialPortInfoListBox->setEditable(isCustomPath);
    if (isCustomPath)
        ui->serialPortInfoListBox->clearEditText();
}

//!
//! \brief SettingsDialog::fillPortsParameters
//!
void SettingsDialog::fillPortsParameters()
{
    ui->baudRateBox->addItem( QStringLiteral( "9600" ), QSerialPort::Baud9600 );
    ui->baudRateBox->addItem( QStringLiteral( "19200" ), QSerialPort::Baud19200 );
    ui->baudRateBox->addItem( QStringLiteral( "38400" ), QSerialPort::Baud38400 );
    ui->baudRateBox->addItem( QStringLiteral( "115200" ), QSerialPort::Baud115200 );
    ui->baudRateBox->addItem( QObject::tr( "Custom" ) );

    ui->dataBitsBox->addItem( QStringLiteral( "5" ), QSerialPort::Data5 );
    ui->dataBitsBox->addItem( QStringLiteral( "6" ), QSerialPort::Data6 );
    ui->dataBitsBox->addItem( QStringLiteral( "7" ), QSerialPort::Data7 );
    ui->dataBitsBox->addItem( QStringLiteral( "8" ), QSerialPort::Data8 );
    ui->dataBitsBox->setCurrentIndex(3);

    ui->parityBox->addItem( QStringLiteral( "NoParity" ), QSerialPort::NoParity );
    ui->parityBox->addItem( QStringLiteral( "EvenParity" ), QSerialPort::EvenParity );
    ui->parityBox->addItem( QStringLiteral( "OddParity" ), QSerialPort::OddParity );
    ui->parityBox->addItem( QStringLiteral( "MarkParity" ), QSerialPort::MarkParity );
    ui->parityBox->addItem( QStringLiteral( "SpaceParity" ), QSerialPort::SpaceParity );

    ui->stopBitsBox->addItem( QStringLiteral( "1" ), QSerialPort::OneStop );
#ifdef Q_OS_WIN
    ui->stopBitsBox->addItem( QObject::tr( "1.5" ), QSerialPort::OneAndHalfStop );
#endif
    ui->stopBitsBox->addItem( QStringLiteral( "2" ), QSerialPort::TwoStop );

    ui->flowControlBox->addItem( QStringLiteral( "None" ), QSerialPort::NoFlowControl );
    ui->flowControlBox->addItem( QStringLiteral( "RTS/CTS" ), QSerialPort::HardwareControl );
    ui->flowControlBox->addItem( QStringLiteral( "XON/XOFF" ), QSerialPort::SoftwareControl );
}

//!
//! \brief SettingsDialog::fillPortsInfo
//!
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

//!
//! \brief SettingsDialog::updateSettings
//!
void SettingsDialog::updateSettings()
{
    currentSettings.name = ui->serialPortInfoListBox->currentText();

    if (ui->baudRateBox->currentIndex() == 4) {
        currentSettings.baudRate = ui->baudRateBox->currentText().toInt();
    } else {
        currentSettings.baudRate = static_cast<QSerialPort::BaudRate>(
                    ui->baudRateBox->itemData(ui->baudRateBox->currentIndex()).toInt());
    }
    currentSettings.stringBaudRate = QString::number(currentSettings.baudRate);

    currentSettings.dataBits = static_cast<QSerialPort::DataBits>(
                ui->dataBitsBox->itemData(ui->dataBitsBox->currentIndex()).toInt());
    currentSettings.stringDataBits = ui->dataBitsBox->currentText();

    currentSettings.parity = static_cast<QSerialPort::Parity>(
                ui->parityBox->itemData(ui->parityBox->currentIndex()).toInt());
    currentSettings.stringParity = ui->parityBox->currentText();

    currentSettings.stopBits = static_cast<QSerialPort::StopBits>(
                ui->stopBitsBox->itemData(ui->stopBitsBox->currentIndex()).toInt());
    currentSettings.stringStopBits = ui->stopBitsBox->currentText();

    currentSettings.flowControl = static_cast<QSerialPort::FlowControl>(
                ui->flowControlBox->itemData(ui->flowControlBox->currentIndex()).toInt());
    currentSettings.stringFlowControl = ui->flowControlBox->currentText();

    currentSettings.localEchoEnabled = ui->localEchoCheckBox->isChecked();
}

