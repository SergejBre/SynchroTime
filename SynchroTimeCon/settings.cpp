//------------------------------------------------------------------------------
//  Home Office
//  Nürnberg, Germany
//  E-Mail: sergej1@email.ua
//
//  Copyright (C) 2020 free Project SynchroTime RTC DS3231. All rights reserved.
//------------------------------------------------------------------------------
//  Project SynchroTime: Time synchronization of the Precision RTC module DS3231
//  with the UTC System Time via the Serial Interface (UART).
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include "settings.h"
#include <QSettings>

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

/**
 * @brief Settings::Settings
 * @param parent
 */
Settings::Settings( QObject *parent )
    : QObject(parent)
{
    this->readSettings();
}

/**
 * @brief Settings::~Settings
 */
Settings::~Settings()
{
    /// \todo
    this->writeSettings();
}

/**
 * @brief Settings::param
 * @return
 */
QString Settings::param( void ) const
{
    return this->m_param;
}

/**
 * @brief Settings::path
 * @return
 */
QString Settings::path() const
{
    return this->m_path;
}

/**
 * @brief Settings::setParam
 * @param param
 */
void Settings::setParam( const QString &param )
{
    this->m_param = param;
}

/**
 * @brief Settings::setPath
 * @param path
 */
void Settings::setPath(const QString &path)
{
    this->m_path = path;
}

/**
 * @brief Settings::pathToLog
 * @return
 */
QString Settings::pathToLog() const
{
    return this->m_pathToLog;
}

/**
 * @brief Settings::portName
 * @return
 */
QString Settings::portName() const
{
    return this->m_portName;
}

/**
 * @brief Settings::setPortName
 * @param port
 */
void Settings::setPortName(const QString &port)
{
    this->m_portName = port;
}

/**
 * @brief Settings::portBaudRate
 * @return
 */
qint32 Settings::portBaudRate() const
{
    return this->m_baudRate;
}

/**
 * @brief Settings::setBaudRate
 * @param baudRate
 */
void Settings::setBaudRate(const qint32 baudRate)
{
    this->m_baudRate = baudRate;
}

/**
 * @brief The function reads the parameters necessary for the user interface that were saved in the previous session.
 *
 * Such important parameters will be read as
 * - the position of the window on the screen and window size,
 * - interface font and its size,
 * - the user interface settings (overwrite of the data, recurse of dir's, etc.)
 * - error and event logging options.
 */
void Settings::readSettings( void )
{
    QSettings settings("synchroTime.ini", QSettings::IniFormat);

    settings.beginGroup("Logging");
    bool enableLog = settings.value("enableLog", true).toBool();
    this->m_enableLog = enableLog;
    QString pathToLog = settings.value("pathToLog", "synchroTime.log").toString();
    this->m_pathToLog = pathToLog;
    quint32 maxSizeLog = settings.value("maxSizeLog", 10U).toUInt();
    this->m_maxSizeLog = maxSizeLog;
    settings.endGroup();

    settings.beginGroup("SerialPort");
    QString portName = settings.value("portName", "ttyUSB0").toString();
    this->m_portName = portName;
    qint32 baudRate = settings.value("baudRate", 115200).toUInt();
    this->m_baudRate = baudRate;
    settings.endGroup();
}

/**
 * @brief The function saves the user interface parameters that have been changed by the user in the current session.
 *
 * Such parameters will be updated as
 * - the position of the window on the screen and window size,
 * - interface font and its size,
 * - the user interface settings (overwrite of the data, recurse of dir's, etc.)
 * - error and event logging options.
 */
void Settings::writeSettings( void ) const
{
    QSettings settings("synchroTime.ini", QSettings::IniFormat);

    settings.beginGroup("Logging");
    settings.setValue("enableLog", this->m_enableLog);
    settings.setValue("pathToLog", this->m_pathToLog);
    settings.setValue("maxSizeLog", this->m_maxSizeLog);
    settings.endGroup();

    settings.beginGroup("SerialPort");
    settings.setValue("portName", this->m_portName);
    settings.setValue("baudRate", this->m_baudRate);
    settings.endGroup();
}
