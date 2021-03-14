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
//! \details
//! Standard constructor
//!
//! \param parent of the type *QWidget - main application window (*MainWindow)
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
//! \details Default destructor
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
//! \brief Console::formatHtml
//! \details
//! This helper function solves the formatting problem.
//! The problem is that plain text can contain special characters that have meta-values in HTML,
//! literally: <> & "'. This is solved by a simple formatter that converts them to their corresponding HTML entities.
//! For example, the function converts the end of the line "\ n" to "\<br/>".
//!
//! \param qText of the type const QString. Block of text to convert to HTML.
//! \param qColor of the type const QColor. The color for the block of text.
//!
//! \return qHtmlText of the type const QString.
//!
const QString Console::formatHtml( const QString &qText, const QColor &qColor ) const
{
    QString qHtmlText = QString::fromLatin1( "<p style='white-space: pre-wrap; color: " )
            + qColor.name( QColor::HexRgb ) + QString::fromLatin1( "'>" );
    for ( const QChar qChar : qText ) {
        switch ( qChar.unicode() ) {
        case '<': qHtmlText += QString::fromLatin1("&lt;");
            break;
        case '>': qHtmlText += QString::fromLatin1("&gt;");
            break;
        case '&': qHtmlText += QString::fromLatin1("&amp;");
            break;
        case '"': qHtmlText += QString::fromLatin1("&quot;");
            break;
        case '\'': qHtmlText += QString::fromLatin1("&apos;");
            break;
        case '\t': qHtmlText += QString::fromLatin1("&nbsp;&nbsp;&nbsp;");
            break;
        case '\n': qHtmlText += QString::fromLatin1("<br/>");
            break;
        default: qHtmlText += qChar; // everything else unchanged
        }
    }
    qHtmlText += QString::fromLatin1("</p>");
    return qHtmlText;
}

//!
//! \brief Console::putData
//! \details
//! The function displays text in the console window,
//! while converting escape sequences '\\x1B' to colored HTML text.
//!
//! \param data of the type const QString. Incoming paint-text.
//!
void Console::putData( const QString &data )
{
    QColor qColor( Qt::green );
    int pos0 = data.indexOf( QChar('\x1B') );
    int pos1 = data.indexOf( QChar('\x1B'), pos0 + 1 );

    if ( pos0 > -1 ) {
        const QChar qChar = data.at( pos0 + 3 );
        switch ( qChar.unicode() ) {
        case '1':
            qColor = QColor( Qt::red );
            break;
        case '3':
            qColor = QColor( Qt::yellow );
            break;
        case '6':
            qColor = QColor( Qt::blue );
            break;
        case '7':
            qColor = QColor( Qt::white );
            break;
        default:
            break;
        }
    }

    if ( pos0 > -1 ) {
        if ( pos0 > 0 ) {
            appendHtml( formatHtml( data.left( pos0 )));
        }
        if ( pos1 > pos0 ) {
            appendHtml( formatHtml( data.mid( pos0 + 5, pos1 - pos0 - 5 ), qColor ));
        }
        if ( pos1 < data.lastIndexOf( QChar('\x1B') ) ) {
            putData( data.mid( pos1 + 4 ) );
        }
        else {
            appendHtml( formatHtml( data.mid( pos1 + 4 )));
        }
    }
    else {
        appendHtml( formatHtml( data ));
    }

    QScrollBar *bar = verticalScrollBar();
    bar->setValue( bar->maximum() );
}

//!
//! \brief Console::setLocalEchoEnabled
//! \param set of the type bool.
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
