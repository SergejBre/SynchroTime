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
#ifndef SETTINGS_H
#define SETTINGS_H

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <QObject>

//------------------------------------------------------------------------------
// Preprocessor
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Enums
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

/**
 * @brief The Settings class
 */
class Settings : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QString param READ param WRITE setParam NOTIFY paramChanged )
    Q_PROPERTY( QString path READ path WRITE setPath NOTIFY pathChanged )

public:
    explicit Settings(QObject *parent = 0);
    ~Settings();

    QString param( void ) const;
    QString path( void ) const;
    void setParam( const QString &param );
    void setPath( const QString &path );
    QString pathToLog( void ) const;
    QString portName( void ) const;
    void setPortName( const QString &port );
    qint32 portBaudRate( void ) const;
    void setBaudRate( const qint32 baudRate );

signals:
    void paramChanged( void );
    void pathChanged( void );

public slots:

private:
    QString m_param;
    QString m_path;

    bool m_enableLog;
    QString m_pathToLog;
    quint32 m_maxSizeLog;

    QString m_portName;
    qint32 m_baudRate;

    void readSettings( void );
    void writeSettings( void ) const;
};

#endif // SETTINGS_H
