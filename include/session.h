//------------------------------------------------------------------------------
//  Home Office
//  Nürnberg, Germany
//  E-Mail: sergej1@email.ua
//
//  Copyright (C) 2020 free Project SynchroTime. All rights reserved.
//------------------------------------------------------------------------------
//  Project SynchroTime: Command-line client for adjust the exact time and
//  calibrating the RTC DS3231 module via the serial interface (UART).
//------------------------------------------------------------------------------
#ifndef SESSION_H
#define SESSION_H

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <QObject>
#include "interface.h"

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
// Classes
//------------------------------------------------------------------------------
//! \brief
//!
//! \details
//!
class Session : virtual public QObject
{
    Q_OBJECT
    Q_CLASSINFO("className", "Session")

public:
    Session( QObject *parent = 0 );
    Session( QObject *parent, Interface *const interface );

    Interface *getInterface( void );

    void setInterface( Interface *const interface );

private:
    Interface *interface;
};

#endif // SESSION_H
