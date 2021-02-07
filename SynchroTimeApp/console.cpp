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
//! \file console.cpp
//!
//! \brief The file contains the definition of the constructor and methods of the Console class.
//!
//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include "console.h"
#include <QScrollBar>

//!
//! \brief Console::Console
//! \param parent
//!
Console::Console( QWidget *parent )
    : QPlainTextEdit( parent )
    , localEchoEnabled( false )
{
    document()->setMaximumBlockCount( 100 );
    QPalette p = palette();
    p.setColor( QPalette::Base, Qt::black );
    p.setColor( QPalette::Text, Qt::green );
    setPalette( p );

    buffer = new QByteArray();
}

//!
//! \brief Console::~Console
//!
Console::~Console()
{
    if ( buffer != nullptr )
    {
        buffer->clear();
        delete buffer;
    }
}

//!
//! \brief Console::putData
//! \param data
//!
void Console::putData(const QString &data )
{
    insertPlainText( data );

    QScrollBar *bar = verticalScrollBar();
    bar->setValue( bar->maximum() );
}

//!
//! \brief Console::setLocalEchoEnabled
//! \param set
//!
void Console::setLocalEchoEnabled( bool set )
{
    localEchoEnabled = set;
}

//!
//! \brief Console::keyPressEvent
//! \param e
//!
void Console::keyPressEvent( QKeyEvent *e )
{
    switch ( e->key() ) {
    case Qt::Key_Backspace:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
    case Qt::Key_Alt:
    case Qt::Key_AltGr:
    case Qt::Key_Shift:
    case Qt::Key_Control:
    case Qt::Key_Home:
    case Qt::Key_End:
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if ( localEchoEnabled )
            QPlainTextEdit::keyPressEvent( e );
        emit getData( *buffer );
        buffer->clear();
        break;
    default:
        if ( localEchoEnabled )
            QPlainTextEdit::keyPressEvent( e );
        buffer->append( e->text().toLocal8Bit() );
    }
}

//!
//! \brief Console::mousePressEvent
//! \param e
//!
void Console::mousePressEvent( QMouseEvent *e )
{
    Q_UNUSED( e )
    setFocus();
}

//!
//! \brief Console::mouseDoubleClickEvent
//! \param e
//!
void Console::mouseDoubleClickEvent( QMouseEvent *e )
{
    Q_UNUSED( e )
}

//!
//! \brief Console::contextMenuEvent
//! \param e
//!
void Console::contextMenuEvent( QContextMenuEvent *e )
{
    Q_UNUSED( e )
}
