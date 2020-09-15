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

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include "base.h"

//! \brief Base::Base
//!
//! \details
//! The Constructor for class Basse
//!
Base::Base( QObject *parent )
    : QObject(parent)
    , standardOutput( stdout )
{

}

//! \brief Base::getClassName
//!
//! \details
//! Get class name as string
//!
//! \return className of the type QString.
//!
QString Base::getClassName(void)
{
    const QMetaObject* metaObject = this->metaObject();
    for(int i = metaObject->classInfoOffset(); i < metaObject->classInfoCount(); ++i)
    {
        if ( QString::compare(metaObject->classInfo(i).name(), "className" ) == 0 )
        {
            return metaObject->classInfo(i).value();
        }
    }
    return "";
}

//! \brief Base::stdOutput
//!
//! \details
//! Output stream for all messages.
//!
//! \return standardOutput of the type QTextStream&.
//!
QTextStream& Base::stdOutput( void )
{
    return this->standardOutput;
}
