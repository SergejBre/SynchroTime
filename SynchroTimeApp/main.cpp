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

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include "mainwindow.h"
#include <QApplication>

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
// Function Prototypes
//------------------------------------------------------------------------------

//!
//! \brief main
//! \param argc
//! \param argv
//! \return
//!
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName( "SynchroTime" );
    app.setApplicationVersion( "v.1.0.0, built on: " + QString(__DATE__).simplified() + " " + __TIME__ );
    app.setWindowIcon( QIcon( "../images/icon.png") );

    MainWindow w;
    w.show();

    return app.exec();
}
