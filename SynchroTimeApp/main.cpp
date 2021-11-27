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
//
// \mainpage SynchroTimeApp - GUI application (GUI-app)
//
// SynchroTimeApp: GUI client for adjust the exact time and
// calibrating the RTC DS3231 module via the serial interface (UART).
//
// \author SergejBre sergej1@email.ua
//

//!
//! \file main.cpp
//!
//! \brief The file contains main functions main()
//!
//!  The main function executes the Qt application instance in a event loop,
//!  sets it up with the specified parameters, and installs
//!  a Qt Translator and GUI MainWindow.
//!
//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QtPlugin>

#ifdef Q_OS_WIN
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#else
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
#endif

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
//! \brief main function of SynchroTimeApp
//!
//! In this function, an instance of a Qt-GUI application app is executed in a event loop
//! and set up with the parameters entered.
//!
//! \param[in] argc the number of parameter.
//! \param[in] argv the command line options.
//!
//! \return  value of the function QApplication::exec()
//! Enters the main event loop and waits until exit() is called.
//! Returns the value that was set to exit() (which is 0 if exit() is called
//! via quit()).
//!
int main(int argc, char *argv[])
{
    qputenv( "QT_STYLE_OVERRIDE", "Fusion" );
    QApplication app(argc, argv);

    app.setApplicationName( QStringLiteral( "synchroTimeApp" ));
    app.setApplicationVersion( QStringLiteral( "1.1.5.32, built on: " ) + QString(__DATE__).simplified() + " " + __TIME__ );
    app.setWindowIcon( QIcon( QStringLiteral( ":/images/icon.png") ));
#ifdef Q_OS_WIN
    app.addLibraryPath( QLibraryInfo::location( QLibraryInfo::LibrariesPath ));
#endif

#if 0 //ndef QT_NO_TRANSLATION
    QString translatorFileName = app.applicationName() + QLatin1String( "_" );
    translatorFileName += QLocale::system().name();
    QTranslator *translator = new QTranslator( &app );
    if ( translator->load( translatorFileName, QLibraryInfo::location( QLibraryInfo::TranslationsPath ) ))
    {
        app.installTranslator( translator );
    }
#endif

    MainWindow w;
    w.show();

    return app.exec();
}
