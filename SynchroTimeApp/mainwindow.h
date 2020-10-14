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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <QMainWindow>

//------------------------------------------------------------------------------
// Preprocessor
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Enums
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
class QLabel;
class Console;
class QThread;
class QTimer;
class RTC;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private slots:
    void about();
    void connectRTC();
    void disconnectRTC();
    void tickClock();

private:
    Ui::MainWindow *ui;
    QLabel *status;
    QLabel *clock;
    Console *m_pConsole;
    // RTC and a separate thread in which it will work.
    QThread *m_pThread;
    RTC *m_pRTC;
    QTimer *m_pTimer;

    void readSettings( void );
    void writeSettings( void ) const;
    void actionsTrigger( bool value ) const;
    void showStatusMessage(const QString &message) const;
};

#endif // MAINWINDOW_H
