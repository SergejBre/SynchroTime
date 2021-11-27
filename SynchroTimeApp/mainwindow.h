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
class Console;
class SettingsDialog;
class QLCDNumber;
class QLabel;
class QThread;
class QTimer;
class RTC;
struct Settings;
class QCPBars;
class QTranslator;
class QCloseEvent;
class QEvent;

namespace Ui {
class MainWindow;
}

//! \class MainWindow
//!
//! \brief The MainWindow class
//!
//! The MainWindow class provides the user with interface methods for the main application window.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    template <typename Container>
    static void push( Container &stack, const float value );
    template <typename Container>
    static void fill( Container &stack, const float value );
    template <typename Container>
    static float mean( const Container &stack );

signals:
    void setRegister( const float value );

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void changeEvent( QEvent *event ) Q_DECL_OVERRIDE;

private slots:
    void connectRTC();
    void disconnectRTC();
    void selectConsoleFont();
    void setRegisterSlot();
    void tickClock();
    void help();
    void aboutQt();
    void about();

    void putDelay( const float delay );
    void handleError( const QString &error );
    void handleSettingsError( const QString &error );
    void initBars();

    void on_actionEnglish_triggered();
    void on_actionGerman_triggered();
    void on_actionRussian_triggered();

private:
    Ui::MainWindow *ui;
    QLabel *status;
    QLCDNumber *clock;
    QTimer *m_pTimer;
    Console *m_pConsole;
    Settings *m_pSettings;
    SettingsDialog *m_pSettingsDialog;
    // RTC and a separate thread in which it will work.
    QThread *m_pThread;
    RTC *m_pRTC;
    QCPBars *m_pCPBars;
    bool m_detectDelayFlag;
    QTranslator *m_pTranslator;
    QString postfix;

    void readSettings( void );
    void writeSettings( void ) const;
    void actionsTrigger( bool value ) const;
    void showStatusMessage(const QString &message) const;
    void changeTranslator( const QString &postfix );
    void setLanguage( const QString &postfix );
    void uncheck( void );
};

#endif // MAINWINDOW_H
