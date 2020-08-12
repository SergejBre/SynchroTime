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
#ifndef BASE_H
#define BASE_H

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <QObject>
#include <QMetaClassInfo>
#include <QList>
#include <QTextStream>

//------------------------------------------------------------------------------
// Preprocessor
//------------------------------------------------------------------------------

#ifdef DOXYGEN_RUNNING
//! Only to create doxygen collaboration diagram
//! (dummy definition of QList as doxygen
//! naturally does not know these types)
template<class T> class QList   {private: T QList;};
#endif

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
class Base : public QObject
{
    Q_OBJECT
public:
    explicit Base( QObject *parent = 0 );

    virtual QString getClassName(void);
    QTextStream& stdOutput( void );

private:

    //! Output stream for all messages
    QTextStream standardOutput;

};

//! \brief
//! Class List overrides QList class methods.
//!
//! \details
//! This class is designed to expand the functionality of class QList.
//! - added method clearList() to free the memory occupied by the objects
//! and elements of the list (via at the QVector).
template <typename T>
class List : public QList<T>
{
public:
    void clearList(void);
};

#endif // BASE_H
