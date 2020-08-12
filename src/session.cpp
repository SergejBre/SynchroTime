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

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include "session.h"

//! \brief
//!
//! \details
//!
Session::Session( QObject *parent )
    : QObject( parent )
{

}

//! \brief
//!
//! \details
//!
Session::Session( QObject *parent, Protocol * const protocol )
    : QObject( parent )
    , protocol( protocol )
    , interface( NULL )
{
    Q_ASSERT( dynamic_cast<ProtocolMFM*> ( protocol) || dynamic_cast<ProtocolQSB*> ( protocol) || dynamic_cast<ProtocolROMbootLoader*> ( protocol) );
    if ( this->protocol != NULL )
    {
        this->protocol->setParent( this );
    }
}

//! \brief
//!
//! \details
//!
Session::Session( QObject *parent, Protocol *const protocol, Interface *const interface )
    : QObject( parent )
    , protocol( protocol )
    , interface( interface )
{
    Q_ASSERT( dynamic_cast<ProtocolMFM*> ( protocol) || dynamic_cast<ProtocolQSB*> ( protocol) || dynamic_cast<ProtocolROMbootLoader*> ( protocol) );
    if ( this->protocol != NULL )
    {
        this->protocol->setParent( this );
    }
    if ( this->interface != NULL )
    {
        this->interface->setParent( this );
    }
}

//! \brief
//!
//! \details
//!
Interface *Session::getInterface( void )
{
    return this->interface;
}

//! \brief
//!
//! \details
//!
Protocol *Session::getProtocol( void )
{
    return this->protocol;
}

//! \brief
//!
//! \details
//!
void Session::setInterface( Interface *const interface )
{
    this->interface = interface;
}

//! \brief
//!
//! \details
//!
void Session::setProtocol( Protocol *const protocol )
{
    this->protocol = protocol;
}
