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
#ifndef CONSOLE_H
#define CONSOLE_H

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <QPlainTextEdit>

//------------------------------------------------------------------------------
// Preprocessor
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Enums
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

//!
//! \brief The Console class
//!
class Console : public QPlainTextEdit
{
    Q_OBJECT

signals:
    void getData( const QByteArray &data );

public:
    explicit Console( QWidget *parent = 0 );
    ~Console();

    void putData( const QByteArray &data );

    void setLocalEchoEnabled( bool set );

protected:
    virtual void keyPressEvent( QKeyEvent *e );
    virtual void mousePressEvent( QMouseEvent *e );
    virtual void mouseDoubleClickEvent( QMouseEvent *e );
    virtual void contextMenuEvent( QContextMenuEvent *e );

private:
    bool localEchoEnabled;
    QByteArray *buffer;
};

#endif // CONSOLE_H
