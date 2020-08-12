//------------------------------------------------------------------------------
//  Home Office
//  Nürnberg, Germany
//  E-Mail: sergej1@email.ua
//
//  Copyright (C) 2020 free Project SynchroTime. All rights reserved.
//------------------------------------------------------------------------------
//  Project SynchroTime: Time synchronization via Serial Port (UART)
//
//
//------------------------------------------------------------------------------
#ifndef SESSION_H
#define SESSION_H

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <QObject>
#include "interface.h"
#include "protocol.h"

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
    Session( QObject *parent, Protocol *const protocol );
    Session( QObject *parent, Protocol *const protocol, Interface *const interface );

    Interface *getInterface( void );
    Protocol *getProtocol( void );

    void setInterface( Interface *const interface );
    void setProtocol( Protocol *const protocol );

private:
    Protocol *protocol;
    Interface *interface;
};

#endif // SESSION_H
