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
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <QDialog>
#include "serialportsettings.h"

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
namespace Ui {
class SettingsDialog;
}
class QIntValidator;

//! \class SettingsDialog
//!
//! \brief The SettingsDialog class
//!
//! The SettingsDialog class is an implementation of a dialog with a user to control
//! and select parameters for connecting via a serial port.
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog( QWidget *parent = 0 );
    ~SettingsDialog();

    Settings settings() const;
    Settings *serialPortSettings();
    void fillSettingsUi();

public slots:
    void show();

private slots:
    void showPortInfo(int idx);
    void apply();
    void cansel();
    void reset();
    void checkCustomBaudRatePolicy(int idx);
    void checkCustomDevicePathPolicy(int idx);
    void on_statusControlCheckBox_clicked( bool checked );

private:
    void fillPortsParameters();
    void fillPortsInfo();
    void updateSettings();

    Ui::SettingsDialog *ui;
    Settings currentSettings;
    QIntValidator *intValidator;
};

#endif // SETTINGSDIALOG_H
