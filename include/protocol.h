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
#ifndef PROTOCOL_H
#define PROTOCOL_H

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <QByteArray>
#include <QtXml/QDomDocument>
#include "base.h"

//------------------------------------------------------------------------------
// Preprocessor
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Enums
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
#define MASKBYTE 0xFF
// number Bytes of the four-byte word
#define WORD_LENGTH 4U
// One half of the four-byte word word
#define ONE_HALF_WORD WORD_LENGTH / 2U
// Header size of the access command (12-bytes)
#define HEADER_SIZE 12U
// Maximum address of the range
#define MAX_ADDRESS 0x1002FFFC
// Minimum address of the range
#define MIN_ADDRESS 0x10000000
// determine the byte order of this system
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
#define HOST_BIG_ENDIAN 1
#else
#define HOST_BIG_ENDIAN 0
#endif

//------------------------------------------------------------------------------
// Classes
//------------------------------------------------------------------------------
//! \brief
//!
//! \details
//!
class Protocol : public Base
{
    Q_OBJECT
    Q_CLASSINFO("className", "Protocol")

public:
    explicit Protocol( QObject *parent = 0 );

    // access Methods
    // Storage access Methods with command @s
    QByteArray accessRead(const quint8 storageNumber, const quint32 startBlock, const quint32 endBlock);
    QByteArray accessWrite(const quint8 storageNumber, const quint32 startBlock, const quint32 endBlock, const QByteArray & data);
    QByteArrayList accessWrite(const quint8 storageNumber, const quint32 startBlock, const quint32 endBlock, const QByteArray &data, const quint16 blockSize);
    QByteArray accessErase(const quint8 storageNumber, const quint32 startBlock, const quint32 endBlock);
    QByteArray accessFormat(const quint8 storageNumber);

    // Request Methods
    // Configuration request Method with command @c
    QByteArray requestConfig(void);
    // Version request Method with command @v
    QByteArray requestVersion(void);
    // Reset request Method with command @r
    QByteArray requestReset(void);
    // Information / Statistics request Method with command @i
    QByteArray requestInfo(void);

    // Check Method for data
    virtual bool checkData(const QByteArray & data) = 0;

    // Configuration Data
    int getVersion( void ) const;
    quint32 getBufferAddress( void ) const;
    quint32 getBufferSizeDirect( void ) const;
    quint32 getBufferSizeUART( void ) const;
    quint32 getBufferSizeETH( void ) const;
    int getKeyCount( void ) const;
    quint32 getKeySize( void ) const;
    int getStorageCount( void ) const;
    int getStatus( void ) const;
    void setVersion( const int version );
    void setBufferAddress( const quint32 bufferAddress );
    void setBufferSizeDirect( const quint32 bufferSizeDirect );
    void setBufferSizeUART( const quint32 bufferSizeUART );
    void setBufferSizeETH( const quint32 bufferSizeETH );
    void setKeyCount( const int keyCount );
    void setKeySize( const quint32 keySize );
    void setStorageCount( const int storageCount );
    void setStatus( const int status );

    // public Parser Methods for Configuration Protocol
    // Status for received request (1 byte)
    int parseStatus(const QByteArray & rawData);
    // Version of configuration format (4 bytes)
    int parseVersion(const QByteArray & rawData);
    // Start address for data in direct mode (4 bytes)
    quint32 parseBufferAddress(const QByteArray & rawData);
    // Buffer size for direct mode (4 bytes)
    quint32 parseBufferSizeDirect(const QByteArray &rawData);
    // Buffer size for UART interactive mode (4 bytes)
    quint32 parseBufferSizeUART(const QByteArray &rawData);
    // Buffer size for Ethernet interactive mode (4 bytes)
    quint32 parseBufferSizeETH(const QByteArray &rawData);
    // Number of key packets that can be stored on device 0 (4 bytes)
    int parseKeyCount(const QByteArray &rawData);
    // Size (in bytes) of a key packet (4 bytes)
    quint32 parseKeySize(const QByteArray &rawData);
    // Number of accessible storage devices (4 bytes, max. value: 16)
    int parseStorageCount(const QByteArray &rawData);
    // Overall size (in bytes) for storage device X (4 bytes)
    quint32 parseStorageXchipsSize(const QByteArray &rawData, const int startNumber);
    // Erasable size (in bytes) for storage device X  (4 bytes)
    quint32 parseStorageXeraseSize(const QByteArray &rawData, const int startNumber);
    // Writeable / Readable size (in bytes) for storage device X  (4 bytes)
    quint32 parseStorageXblockSize(const QByteArray &rawData, const int startNumber);

    virtual bool parseConfigProtocol(const QByteArray &rawData);

signals:
    void infoMessage( const QVariant & message ) const;
    void debugMessage( const QVariant & message ) const;
    void errorMessage( const QVariant & message ) const;

private:
    // private append Methods
    void appendBytes(QByteArray *byteArray, const quint8 numberBytes, quint64 value);
    void appendCRC32(QByteArray *data);

    // private Parser Methods
    quint64 parseToInt(const QByteArray &byteArray, const int startNumber, const int stopNumber);
    QString parseToStr(const QByteArray &byteArray, const int startNumber, const int stopNumber);

    // Configuration Data
    /*
    VERSION             Version of configuration format (4 bytes)
    BUFFER_ADDRESS		Start address for data in direct mode (4 bytes)
    BUFFER_SIZE_DIRECT	Buffer size for direct mode (4 bytes)
    BUFFER_ADDRESS		Start address for data in direct mode (4 bytes)
    BUFFER_SIZE_DIRECT	Buffer size for direct mode (4 bytes)
    BUFFER_SIZE_UART 	Buffer size for UART interactive mode (4 bytes)
    BUFFER_SIZE_ETH		Buffer size for Ethernet interactive mode (4 bytes)
    KEY_COUNT			Number of key packets that can be stored on device 0 (4 bytes)
    KEY_SIZE			Size (in bytes) of a key packet (4 bytes)
    STORAGE_COUNT		Number of accessible storage devices (4 bytes, max. value: 16)

    For the configuration of the MFM, the entries <KEY_COUNT> and <KEY_SIZE>
    are set to zero (0x00000000) since these values are only used for the extended QSB protocol.
    */
    //! Version of configuration format (4 bytes)
    int version;
    //! Start address for data in direct mode (4 bytes)
    quint32 bufferAddress;
    //! Buffer size for direct mode (4 bytes)
    quint32 bufferSizeDirect;
    //! Buffer size for UART interactive mode (4 bytes)
    quint32 bufferSizeUART;
    //! Buffer size for Ethernet interactive mode (4 bytes)
    quint32 bufferSizeETH;
    //! Number of key packets that can be stored on device 0 (4 bytes)
    int keyCount;
    //! Size (in bytes) of a key packet (4 bytes)
    quint32 keySize;
    //! Number of accessible storage devices (4 bytes, max. value: 16)
    int storageCount;

    //! Error status after parsing the binary file
    //! if 0, parsing of the data was successful,
    //! if 1, parsing of the data was with error.
    int status;
};

//! \brief
//!
//! \details
//!
class ProtocolMFM : public Protocol
{
    Q_OBJECT
    Q_CLASSINFO("className", "ProtocolMFM")

public:
    explicit ProtocolMFM( QObject *parent = 0 );

    // Check Method for data
    bool checkData( const QByteArray & data );

    // parse Method
    bool parseConfigProtocol( const QByteArray & rawData );

private:

};

//! \brief
//!
//! \details
//!
class ProtocolQSB : public ProtocolMFM
{
    Q_OBJECT
    Q_CLASSINFO("className", "ProtocolQSB")

public:
    explicit ProtocolQSB( QObject *parent = 0 );

    // Check Method for data
    bool checkData( const QByteArray & data );

    // parse Method
    bool parseConfigProtocol( const QByteArray & rawData );

private:

};

//! \brief
//!
//! \details
//!
class ProtocolROMbootLoader : public Protocol
{
    Q_OBJECT
    Q_CLASSINFO("className", "ProtocolROMbootLoader")

public:
    explicit ProtocolROMbootLoader(QObject *parent = 0);

    // access Methods
    using Protocol::accessRead;
    QByteArray accessRead( const quint32 startAddress, const quint16 lengthOfData );
    using Protocol::accessWrite;
    QByteArray accessWrite( const quint32 startAddress, const quint16 lengthOfData, const QByteArray & data );

    // Request Methods
    // Version request Method with command #v
    QByteArray requestVersion( void ) const;
    // Reset request Method with command #s
    QByteArray requestReset( const quint32 startAddress ) const;

    // parse Method
    bool parseConfigProtocol(const QByteArray &rawData);

    // Check Methods
    bool checkStatus( const QByteArray & data );
    bool checkData( const QByteArray & receivedData );
    quint32 calculateChecksum( const QByteArray & data );
    void setBigEndian( const bool endian );
    bool isBigEndian( void ) const;
//    void setCheckSum( );
    quint32 getCheckSum( void ) const;

private:

    quint16 get_word16( const quint8 *data );
    quint32 get_word32( const quint8 *data );
    quint32 calc_checksum( const quint8 *data, quint32 len_words );

    //! true if Big endian, false if Little endian
    bool byteEndian;

    //! Check sum for a send command
    quint32 checkSum;
};

#endif // PROTOCOL_H
