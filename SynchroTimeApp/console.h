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

//! \class Console
//!
//! \brief The Console class
//!
//! Provides the user with methods for the console interface.
class Console : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit Console( QWidget *parent = 0 );
    ~Console();

    void putData( const QString &data );
    void setLocalEchoEnabled( bool set );

signals:
    void getData( const QByteArray &data );

protected:
    virtual void keyPressEvent( QKeyEvent *e );
    virtual void mousePressEvent( QMouseEvent *e );
    virtual void mouseDoubleClickEvent( QMouseEvent *e );
    virtual void contextMenuEvent( QContextMenuEvent *e );

private:
    const QString formatHtml( const QString &qText, const QColor &qColor = QColor(Qt::green) ) const;
    bool localEchoEnabled;
    QByteArray *buffer;
};

#endif // CONSOLE_H
