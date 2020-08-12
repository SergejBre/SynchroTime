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
#include <QDebug>
#include <QLoggingCategory>
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
Q_LOGGING_CATEGORY(logProt, "Protocol")

//------------------------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------------------------

//! \brief Protocol::Protocol
//! Constructor of the class Protocol for creating objects with default values.
//!
//! \details
//!
//! \param[in] parent Pointer to the parent object used for QObject
//!
Protocol::Protocol( QObject *parent )
    : Base( parent )
    , version( 0 )
    , bufferAddress( 0U )
    , bufferSizeDirect( 0U )
    , bufferSizeUART( 0U )
    , bufferSizeETH( 0U )
    , keyCount( 0 )
    , keySize( 0U )
    , storageCount( 0 )
    , status( 0 )
{

}

//! \brief
//! Storage Access Methods for Read with command 'r'
//!
//! \details
//! \todo
//!
QByteArray Protocol::accessRead(const quint8 storageNumber, const quint32 startBlock, const quint32 endBlock)
{
    Q_ASSERT( endBlock >= startBlock );
    if ( endBlock <= startBlock )
    {
        // Error handling
        this->setStatus( 1 );
        qCritical( logProt ) << QObject::tr( "During reading, the incorrect data occurs: endBlock=%1, startBlock=%2" ).arg( endBlock ).arg( startBlock );
    }

    QByteArray instruction("");
    // Append a Storage Number
    appendBytes(&instruction, 1U, storageNumber);
    // Append a read Command
    instruction.append('r');
    appendBytes(&instruction, WORD_LENGTH, startBlock);
    appendBytes(&instruction, WORD_LENGTH, endBlock);
    // Append the CRC32 Cheksum
//    appendCRC32(&instruction);
    instruction.insert(0U, "@s");

    return instruction;
}

//! \brief
//! Storage Access Methods for Write with command 'w'
//!
//! \details
//! \todo
//!
QByteArray Protocol::accessWrite(const quint8 storageNumber, const quint32 startBlock, const quint32 endBlock, const QByteArray & data)
{
    Q_ASSERT( endBlock >= startBlock );
    if ( endBlock <= startBlock )
    {
        // Error handling
        this->setStatus( 1 );
        qCritical( logProt ) << QObject::tr( "During writing, the incorrect data occurs: endBlock=%1, startBlock=%2" ).arg( endBlock ).arg( startBlock );
    }

    QByteArray instruction("");
    // Append a Storage Number
    appendBytes(&instruction, 1U, storageNumber);
    // Append a write Command
    instruction.append('w');
    appendBytes(&instruction, WORD_LENGTH, startBlock);
    appendBytes(&instruction, WORD_LENGTH, endBlock);
    instruction.append(data);
    // Append the CRC32 Cheksum
//    appendCRC32(&instruction);
    instruction.insert(0U, "@s");

    return instruction;
}

//! \brief
//! Storage Access Methods for Write with command 'w'
//!
//! \details
//! \todo
//!
QByteArrayList Protocol::accessWrite(const quint8 storageNumber, const quint32 startBlock, const quint32 endBlock, const QByteArray &data, const quint16 blockSize)
{
    Q_ASSERT( endBlock >= startBlock );
    if ( endBlock <= startBlock )
    {
        // Error handling
        this->setStatus( 1 );
        qCritical( logProt ) << QObject::tr( "During writing, the incorrect data occurs: endBlock=%1, startBlock=%2" ).arg( endBlock ).arg( startBlock );
    }

    QByteArrayList instructionList;
    int blockDataSize = blockSize - HEADER_SIZE - WORD_LENGTH;

    // Check if the file size does not exceed the buffer size.
    if ( blockDataSize > 0 && blockDataSize < data.size() )
    {
        int pos = 0;
        quint32 start = startBlock;
        quint32 numberBlocks = blockDataSize;
        quint32 end = startBlock + numberBlocks - 1;
        while( pos < data.size() )
        {
            QByteArray tmp = data.mid( pos, blockDataSize );
            instructionList << this->accessWrite( storageNumber, start, end, tmp );
            start = end + 1;
            end += numberBlocks;
            pos += tmp.size();
        }
    }

    return instructionList;
}

//! \brief
//! Storage Access Methods for Erase with command 'e'
//!
//! \details
//! \todo
//!
QByteArray Protocol::accessErase(const quint8 storageNumber, const quint32 startBlock, const quint32 endBlock)
{
    Q_ASSERT( endBlock >= startBlock );
    if ( endBlock <= startBlock )
    {
        // Error handling
        this->setStatus( 1 );
        qCritical( logProt ) << QObject::tr( "During erasing, the incorrect data occurs: endBlock=%1, startBlock=%2" ).arg( endBlock ).arg( startBlock );
    }

    QByteArray instruction("");
    // Append a Storage Number
    appendBytes(&instruction, 1U, storageNumber);
    // Append a erase Command
    instruction.append('e');
    appendBytes(&instruction, 4U, startBlock);
    appendBytes(&instruction, 4U, endBlock);
    // Append the CRC32 Cheksum
//    appendCRC32(&instruction);
    instruction.insert(0U, "@s");

    return instruction;
}

//! \brief
//! Storage Access Methods for Format with command 'f'
//!
//! \details
//! \todo
//!
QByteArray Protocol::accessFormat(const quint8 storageNumber)
{
    Q_ASSERT( storageNumber < this->storageCount );
    QByteArray instruction("");
    // Append a Storage Number
    appendBytes(&instruction, 1U, storageNumber);
    // Append a format Command
    instruction.append('f');
    // Append the CRC32 Cheksum
//    appendCRC32(&instruction);
    instruction.insert(0U, "@s");

    return instruction;
}

//! \brief
//! Configuration request Method with command 'c'
//!
//! \details
//! \todo
//!
QByteArray Protocol::requestConfig(void)
{
    QByteArray instruction("@c");

    return instruction;
}

//! \brief
//! Version request Method with command 'v'
//!
//! \details
//! \todo
//!
QByteArray Protocol::requestVersion(void)
{
    QByteArray instruction("@v");

    return instruction;
}

//! \brief
//! Reset Method with command 'r'
//!
//! \details
//! \todo
//!
QByteArray Protocol::requestReset(void)
{
    QByteArray instruction("@r");

    return instruction;
}

//! \brief
//! Information / Statistics request Method with command 'i'
//!
//! \details
//! \todo
//!
QByteArray Protocol::requestInfo(void)
{
    QByteArray instruction("@i");

    return instruction;
}

//! \brief
//! The function is used to represent a unsigned integer as a sequence of bytes.
//!
//! \details
//! The number of bytes required to write the unsigned integer in the array is limited to 255 places.
//!
//! \param[in,out] *byteArray of type QByteArray
//! \param[in] numberBytes number of bytes to write the unsigned integer (4 or 8).
//! \param[in] value of unsigned integer (of type uint8, uint16, uint32 or uint64).
//!
void Protocol::appendBytes(QByteArray *byteArray, const quint8 numberBytes, quint64 value)
{
    Q_ASSERT( !byteArray->isNull() );
    if ( !byteArray->isNull() )
    {
        QByteArray temp;
        temp.resize(numberBytes);
        int n;
        for (n = numberBytes - 1; n >= 0; n--)
        {
            temp[n] = value & MASKBYTE;
            value = value >> 8U;
        }
        byteArray->append(temp);
    }
    else
    {
        // Error handling
        this->setStatus( 1 );
        qCritical( logProt ) << QObject::tr( "During byte appending, the incorrect data occurs: *byteArray=NULL" );
    }
}

//! \brief Protocol::parseToInt
//! The method restores bytewise numeric values.
//!
//! \details
//! it is assumed that the bit sequence as Big-Endian sent.
//!
//! \param[in] byteArray of type QByteArray
//! \param[in] startNumber first byte of the number representation (as Big-Endian).
//! \param[in] stopNumber Last byte of the number representation (as Big-Endian).
//!
//! \return returnValue the desired value of the type quint64.
//!
quint64 Protocol::parseToInt(const QByteArray &byteArray, const int startNumber, const int stopNumber)
{
//    Q_ASSERT( (stopNumber >= startNumber) && (byteArray.size() >= stopNumber) );
    Q_ASSERT( (stopNumber - startNumber) < 8 );
    if ( (stopNumber < startNumber) || (byteArray.size() < stopNumber) )
    {
        // Error handling
        this->status = 1;
        qCritical( logProt ) << QObject::tr( "During parsing the numbers data, the incorrect data occurs: %1, %2" ).arg( stopNumber ).arg( startNumber );
        return 0U;
    }
    if ( (stopNumber - startNumber) > 7 )
    {
        // Error handling
        this->status = 1;
        qCritical( logProt ) << QObject::tr( "During parsing the numbers data, the incorrect data occurs: %1, %2" ).arg( stopNumber ).arg( startNumber );
        return 0U;
    }
    quint64 returnValue = 0U;
    int n;

    for (n = startNumber - 1; n < stopNumber; n++)
    {
        returnValue = returnValue << 8U;
        returnValue += byteArray.at(n);
    }
    return returnValue;
}


//! \brief Protocol::parseToStr
//! The method restores bytewise char array (QString).
//!
//! \details
//! it is assumed that the bit sequence as char (Hex) sent.
//!
//! \param[in] byteArray of type QByteArray
//! \param[in] startNumber first char of the char array.
//! \param[in] stopNumber Last char of the char array.
//!
//! \return returnValue the desired value of the type QString.
//!
QString Protocol::parseToStr(const QByteArray &byteArray, const int startNumber, const int stopNumber)
{
    if ( (stopNumber < startNumber) || (byteArray.size() < stopNumber) )
    {
        // Error handling
        this->status = 1;
        qCritical( logProt ) << QObject::tr( "During parsing of string, the incorrect data occurs: stopNumber=%1, startNumber=%2" ).arg( stopNumber ).arg( startNumber );
        return NULL;
    }
    QString returnValue;
    int n;

    for (n = startNumber - 1; n < stopNumber; n++)
    {
        returnValue.append( byteArray.at(n) );
    }
    return returnValue;
}

// Configuration Protocol
//! \brief Protocol::parseStatus
//! The method returns the status byte back from a configuration protocol.
//!
//! \details
//! The Status is a first byte in the array. Status for received request (1 byte).
//! If status is different from 0x00 (“Success”), no further data follows.
//!
//! \param[in] rawData (configuration protocol) of type QByteArray
//!
//! \return rawData.at(0) the desired value of the type int.
//!
int Protocol::parseStatus(const QByteArray & rawData)
{
    Q_ASSERT( !rawData.isNull() );
    if ( rawData.isNull() )
    {
        // Error handling
        this->status = 1;
        qCritical( logProt ) << QObject::tr( "During parsing of Status, the incorrect data occurs." );
        return -1;
    }
    return (int) rawData.at(0);
}

//! \brief Protocol::parseVersion
//! The method restores bytewise numeric values of VERSION.
//!
//! \details
//! Version of configuration format (4 bytes).
//!
//! \param[in] rawData of type QByteArray
//!
//! \return returnValue the desired value of the type int.
//!
int Protocol::parseVersion(const QByteArray & rawData)
{
    Q_ASSERT( !rawData.isNull() );
    int returnValue;
    // start number in the bytes sequence
    quint8 startNumber = 1U;
    returnValue = parseToInt(rawData, startNumber, startNumber + WORD_LENGTH - 1);
    return returnValue;
}

//! \brief Protocol::parseBufferAddress
//! The method restores bytewise numeric values of BUFFER_ADDRESS.
//!
//! \details
//! Start address for data in direct mode (4 bytes).
//!
//! \param[in] rawData of type QByteArray
//!
//! \return returnValue the desired value of the type quint32.
//!
quint32 Protocol::parseBufferAddress(const QByteArray & rawData)
{
    Q_ASSERT( !rawData.isNull() );
    quint32 returnValue;
    // start number in the bytes sequence
    quint8 startNumber = 1U*WORD_LENGTH + 1U;
    returnValue = parseToInt(rawData, startNumber, startNumber + WORD_LENGTH - 1);
    return returnValue;
}

//! \brief Protocol::parseBufferSizeDirect
//! The method restores bytewise numeric values of BUFFER_SIZE_DIRECT.
//!
//! \details
//! Buffer size for direct mode (4 bytes).
//!
//! \param[in] rawData of type QByteArray
//!
//! \return returnValue the desired value of the type quint32.
//!
quint32 Protocol::parseBufferSizeDirect(const QByteArray &rawData)
{
    Q_ASSERT( !rawData.isNull() );
    quint32 returnValue;
    // start number in the bytes sequence
    quint8 startNumber = 2U*WORD_LENGTH + 1U;
    returnValue = parseToInt(rawData, startNumber, startNumber + WORD_LENGTH - 1);
    return returnValue;
}

//! \brief Protocol::parseBufferSizeUART
//! The method restores bytewise numeric values of BUFFER_SIZE_UART.
//!
//! \details
//! Buffer size for UART interactive mode (4 bytes).
//!
//! \param[in] rawData of type QByteArray
//!
//! \return returnValue the desired value of the type quint32.
//!
quint32 Protocol::parseBufferSizeUART(const QByteArray &rawData)
{
    Q_ASSERT( !rawData.isNull() );
    quint32 returnValue;
    // start number in the bytes sequence
    quint8 startNumber = 3U*WORD_LENGTH + 1U;
    returnValue = parseToInt(rawData, startNumber, startNumber + WORD_LENGTH - 1);
    return returnValue;
}

//! \brief Protocol::parseBufferSizeETH
//! The method restores bytewise numeric values of BUFFER_SIZE_ETH.
//!
//! \details
//! Buffer size for Ethernet interactive mode (4 bytes).
//!
//! \param[in] rawData of type QByteArray
//!
//! \return returnValue the desired value of the type quint32.
//!
quint32 Protocol::parseBufferSizeETH(const QByteArray &rawData)
{
    Q_ASSERT( !rawData.isNull() );
    quint32 returnValue;
    // start number in the bytes sequence
    quint8 startNumber = 4U*WORD_LENGTH + 1U;
    returnValue = parseToInt(rawData, startNumber, startNumber + WORD_LENGTH - 1);
    return returnValue;
}

//! \brief Protocol::parseKeyCount
//! The method restores bytewise numeric values of KEY_COUNT.
//!
//! \details
//! Number of key packets that can be stored on device 0 (4 bytes).
//!
//! \param[in] rawData of type QByteArray
//!
//! \return returnValue the desired value of the type int.
//!
int Protocol::parseKeyCount(const QByteArray &rawData)
{
    Q_ASSERT( !rawData.isNull() );
    int returnValue;
    // start number in the bytes sequence
    quint8 startNumber = 5U*WORD_LENGTH + 1U;
    returnValue = parseToInt(rawData, startNumber, startNumber + WORD_LENGTH - 1);
    return returnValue;
}

//! \brief Protocol::parseKeySize
//! The method restores bytewise numeric values of KEY_SIZE.
//!
//! \details
//! Size (in bytes) of a key packet (4 bytes).
//!
//! \param[in] rawData of type QByteArray
//!
//! \return returnValue the desired value of the type quint32.
//!
quint32 Protocol::parseKeySize(const QByteArray &rawData)
{
    Q_ASSERT( !rawData.isNull() );
    quint32 returnValue;
    // start number in the bytes sequence
    quint8 startNumber = 6U*WORD_LENGTH + 1U;
    returnValue = parseToInt(rawData, startNumber, startNumber + WORD_LENGTH - 1);
    return returnValue;
}

//! \brief Protocol::parseStorageCount
//! The method restores bytewise numeric values of STORAGE_COUNT.
//!
//! \details
//! Number of accessible storage devices (4 bytes, max. value: 16).
//!
//! \param[in] rawData of type QByteArray
//!
//! \return returnValue the desired value of the type int.
//!
int Protocol::parseStorageCount(const QByteArray &rawData)
{
    Q_ASSERT( !rawData.isNull() );
    int returnValue = 0;
    // start number in the bytes sequence
    quint8 startNumber = 7U*WORD_LENGTH + 1U;
    returnValue = parseToInt(rawData, startNumber, startNumber + WORD_LENGTH - 1);

    if ( returnValue > 16 )
    {
        // Error handling
        this->setStatus( 1 );
        qCritical( logProt ) << QObject::tr( "Number of accessible storage devices %1 exceeds the maximum number (16)." ).arg( returnValue );
#ifdef GUI_APP
        emit debugMessage( QObject::tr( "Number of accessible storage devices %1 exceeds the maximum number (16)." ).arg( returnValue ) );
#endif
        return 0;
    }
    return returnValue;
}

//! \brief Protocol::parseStorageXchipsSize
//! The method restores bytewise numeric values of STORAGE_X_CHIPSIZE.
//!
//! \details
//! Overall size (in bytes) for storage device X (4 bytes).
//!
//! \param[in] rawData of type QByteArray
//! \param[in] startNumber first byte of the number representation (as Big-Endian).
//!
//! \return returnValue the desired value of the type quint32.
//!
quint32 Protocol::parseStorageXchipsSize(const QByteArray &rawData, const int startNumber)
{
    Q_ASSERT( !rawData.isNull() );
    quint32 returnValue;
    returnValue = parseToInt(rawData, startNumber, startNumber + WORD_LENGTH - 1);
    return returnValue;
}

//! \brief Protocol::parseStorageXeraseSize
//! The method restores bytewise numeric values of STORAGE_X_ERASESIZE.
//!
//! \details
//! Erasable size (in bytes) for storage device X  (4 bytes).
//!
//! \param[in] rawData of type QByteArray
//! \param[in] startNumber first byte of the number representation (as Big-Endian).
//!
//! \return returnValue the desired value of the type quint32.
//!
quint32 Protocol::parseStorageXeraseSize(const QByteArray &rawData, const int startNumber)
{
    Q_ASSERT( !rawData.isNull() );
    quint32 returnValue;
    returnValue = parseToInt(rawData, startNumber, startNumber + WORD_LENGTH - 1);
    return returnValue;
}

//! \brief Protocol::parseStorageXblockSize
//! The method restores bytewise numeric values of STORAGE_X_BLOCKSIZE.
//!
//! \details
//! Writeable / Readable size (in bytes) for storage device X  (4 bytes).
//!
//! \param[in] rawData of type QByteArray
//! \param[in] startNumber first byte of the number representation (as Big-Endian).
//!
//! \return returnValue the desired value of the type quint32.
//!
quint32 Protocol::parseStorageXblockSize(const QByteArray &rawData, const int startNumber)
{
    Q_ASSERT( !rawData.isNull() );
    quint32 returnValue;
    returnValue = parseToInt(rawData, startNumber, startNumber + WORD_LENGTH - 1);
    return returnValue;
}

//! \brief Protocol::parseConfigProtocol
//! The Function to parse the protocol settings.
//!
//! \details
//! The function of the base class Protocol reads device parameters from the bytesstream.
//! Two parameters for handing over the keys (KEY_COUNT and KEY_SIZE) are installed
//! in the derivative classes will be, depending on type of selected connection protocol.
//!
//! \param[in] rawData of type QByteArray
//!
//! \retval true  if the parsing of the raw data was OK;
//! \retval false if the parsing of the raw data was not OK.
//!
bool Protocol::parseConfigProtocol(const QByteArray &rawData)
{
    Q_ASSERT( !rawData.isNull() );
    if ( rawData.isNull() )
    {
        this->setStatus( 1 );
        return false;
    }
    // Version of configuration format
    this->version = parseVersion(rawData);
    // Start address for data in direct mode
    this->bufferAddress = parseBufferAddress(rawData);
    // Buffer size for direct mode
    this->bufferSizeDirect = parseBufferSizeDirect(rawData);
    // Buffer size for UART interactive mode
    this->bufferSizeUART = parseBufferSizeUART(rawData);
    // Buffer size for Ethernet interactive mode
    this->bufferSizeETH = parseBufferSizeETH(rawData);
    // Number of key packets that can be stored on device 0
//    this->keyCount = parseKeyCount(rawData);
    // Size (in bytes) of a key packet
//    this->keySize = parseKeySize(rawData);
    // Number of accessible storage devices
    this->storageCount = parseStorageCount(rawData);

    if ( this->storageCount > 0 )
    {
        // Parse for the devices 0, 1, 2, ...
        // Number in the bytes sequence
        quint32 number = 8U*WORD_LENGTH + 1;
        int n;
        for ( n = 0; n < storageCount; n++)
        {
            number += 3U*WORD_LENGTH;
        }
    }
    return ( this->getStatus() == 0 ) ? true : false;
}

//! \brief Protocol::checkData
//! The method checks the data integrity.
//!
//! \details
//! The input data must contain a (CRC32) checksum at the end of the array.
//! Otherwise method can not properly check the integrity of the data.
//! [data][CRC32 checksum]
//!
//! \param[in] data of type QByteArray
//!
//! \retval true  if the verification of the data was OK;
//! \retval false if the verification of the data was not OK.
//!
bool Protocol::checkData(const QByteArray & data)
{
    Q_ASSERT( !data.isNull() );
    if ( data.isNull() )
    {
        // Error handling
        this->status = 1;
        qCritical( logProt ) << QObject::tr( "During checking the data on the integrity (CRCsum), the incorrect data occurs." );
        return false;
    }
//    if ( calculateCRC32( data ) == 0U )
    {
        return true;
    }
    return false;
}

//! Get Version of configuration format
int Protocol::getVersion( void ) const
{
    return this->version;
}

//! Get Start address for data in direct mode
quint32 Protocol::getBufferAddress( void ) const
{
    return this->bufferAddress;
}

//! Get Buffer size for direct mode
quint32 Protocol::getBufferSizeDirect( void ) const
{
    return this->bufferSizeDirect;
}

//! Get Buffer size for UART interactive mode
quint32 Protocol::getBufferSizeUART( void ) const
{
    return this->bufferSizeUART;
}

//! Get Buffer size for Ethernet interactive mode
quint32 Protocol::getBufferSizeETH( void ) const
{
    return this->bufferSizeETH;
}

//! Get Number of key packets that can be stored on device 0
int Protocol::getKeyCount( void ) const
{
    return this->keyCount;
}

//! Get Size (in bytes) of a key packet
quint32 Protocol::getKeySize( void ) const
{
    return this->keySize;
}

//! Get Number of accessible storage devices
int Protocol::getStorageCount( void ) const
{
    return this->storageCount;
}

//! Get error status of the parsing the binary file
int Protocol::getStatus( void ) const
{
    return this->status;
}

//! Set Version of configuration format
void Protocol::setVersion( const int version )
{
    this->version = version;
}

//! Set Start address for data in direct mode
void Protocol::setBufferAddress( const quint32 bufferAddress )
{
    this->bufferAddress = bufferAddress;
}

//! Set Buffer size for direct mode
void Protocol::setBufferSizeDirect( const quint32 bufferSizeDirect )
{
    this->bufferSizeDirect = bufferSizeDirect;
}

//! Set Buffer size for UART interactive mode
void Protocol::setBufferSizeUART( const quint32 bufferSizeUART )
{
    this->bufferSizeUART = bufferSizeUART;
}

//! Set Buffer size for Ethernet interactive mode
void Protocol::setBufferSizeETH( const quint32 bufferSizeETH )
{
    this->bufferSizeETH = bufferSizeETH;
}

//! Set Number of key packets that can be stored on device 0
void Protocol::setKeyCount( const int keyCount )
{
    this->keyCount = keyCount;
}

//! Set Size (in bytes) of a key packet
void Protocol::setKeySize( const quint32 keySize )
{
    this->keySize = keySize;
}

//! Set Number of accessible storage devices
void Protocol::setStorageCount( const int storageCount )
{
    this->storageCount = storageCount;
}

//! Set Error status of the parsing the binary file
void Protocol::setStatus( const int status )
{
    this->status = status;
}

//! \brief ProtocolMFM::ProtocolMFM
//! Constructor of the class ProtocolMFM for creating objects with default values.
//!
//! \details
//! Create a instance of the class
//!
//! \param[in] parent Pointer to the parent object used for QObject
//!
ProtocolMFM::ProtocolMFM( QObject *parent )
    : Protocol( parent )
{

}

//! \brief ProtocolMFM::parseConfigProtocol
//! The Function to parse the MFM protocol settings.
//!
//! \details
//! The function reads device parameters from the bytesstream.
//! For the configuration of the MFM, the entries KEY_COUNT and KEY_SIZE
//! are set to zero (0x00000000) since these values are only used for the extended QSB protocol.
//!
//! \param[in] rawData of type QByteArray
//!
//! \retval true  if the parsing of the raw data was OK;
//! \retval false if the parsing of the raw data was not OK.
//!
bool ProtocolMFM::parseConfigProtocol(const QByteArray &rawData)
{
    Q_ASSERT( !rawData.isNull() );
    if ( rawData.isNull() )
    {
        return false;
    }
    // Number of key packets that can be stored on device 0
    this->setKeyCount( 0 );
    // Size (in bytes) of a key packet
    this->setKeySize( 0U );

    return Protocol::parseConfigProtocol(rawData);
}

//! \brief ProtocolMFM::checkData
//! The method checks the data integrity.
//!
//! \details
//! The input data must contain a (CRC32) checksum at the end of the array.
//! Otherwise method can not properly check the integrity of the data.
//! [data][CRC32 checksum]
//!
//! \param[in] data of type QByteArray
//!
//! \retval true  if the verification of the data was OK;
//! \retval false if the verification of the data was not OK.
//!
bool ProtocolMFM::checkData( const QByteArray & data )
{
    return Protocol::checkData( data );
}

//! \brief ProtocolQSB::ProtocolQSB
//! Constructor of the class ProtocolQSB for creating objects with default values.
//!
//! \details
//! Create a instance of the class and call following init methods:
//!  -# Protocol::init()
//!  .
//!
//! \param[in] parent Pointer to the parent object used for QObject
//!
ProtocolQSB::ProtocolQSB(QObject *parent)
    : ProtocolMFM(parent)
{

}

//! \brief ProtocolQSB::parseConfigProtocol
//! The Function to parse the QSB protocol settings.
//!
//! \details
//! The function reads device parameters from the bytesstream.
//! For the configuration of the QSB set special entries KEY_COUNT and KEY_SIZE.
//!
//! \param[in] rawData of type QByteArray
//!
//! \retval true  if the parsing of the raw data was OK;
//! \retval false if the parsing of the raw data was not OK.
//!
bool ProtocolQSB::parseConfigProtocol( const QByteArray &rawData )
{
    Q_ASSERT( !rawData.isNull() );
    if ( rawData.isNull() )
    {
        return false;
    }
    // Number of key packets that can be stored on device 0
    this->setKeyCount( parseKeyCount( rawData ) );
    // Size (in bytes) of a key packet
    this->setKeySize( parseKeySize( rawData ) );

    return Protocol::parseConfigProtocol( rawData );
}

//! \brief ProtocolQSB::checkData
//! The method checks the data integrity.
//!
//! \details
//! The input data must contain a (CRC32) checksum at the end of the array.
//! Otherwise method can not properly check the integrity of the data.
//! [data][CRC32 checksum]
//!
//! \param[in] data of type QByteArray
//!
//! \retval true  if the verification of the data was OK;
//! \retval false if the verification of the data was not OK.
//!
bool ProtocolQSB::checkData( const QByteArray & data )
{
    return Protocol::checkData( data );
}

//! \brief ProtocolROMbootLoader::ProtocolROMbootLoader
//! Constructor of the class ProtocolROMbootLoader for creating objects with default values.
//!
//! \details
//! Create a instance of the class
//!
//! \param[in] parent Pointer to the parent object used for QObject
//!
ProtocolROMbootLoader::ProtocolROMbootLoader( QObject *parent )
    : Protocol( parent )
    , checkSum( 0U )
{
    //! \todo Big endian or Little endian?
    this->setBigEndian( false );
}

//! \brief ProtocolROMbootLoader::parseConfigProtocol
//! The Function to parse the ROMbootLoader protocol settings.
//!
//! \details
//! There is no specification for this protocol!
//!
//! \param[in] rawData of type QByteArray
//!
//! \retval true  if the parsing of the raw data was OK;
//! \retval false if the parsing of the raw data was not OK.
//!
bool ProtocolROMbootLoader::parseConfigProtocol(const QByteArray &rawData)
{
    Q_ASSERT( !rawData.isNull() );
    if ( rawData.isNull() )
    {
        return false;
    }
    //! \todo There is no specification for this protocol!
    return Protocol::parseConfigProtocol(rawData);
}

//! \brief
//! Storage Access Methods for Read with command 'r'
//!
//! \details
//! Read data : reads a defined number of bytes from a defined address
//!
//! \param[in] startAddress Start address of the type quint32
//!
//! \param[in] lengthOfData Length of data of the type quint16
//!
//! \return instruction a Instruction for Bootloader protocol of type QByteArray
//!
QByteArray ProtocolROMbootLoader::accessRead( const quint32 startAddress, const quint16 lengthOfData )
{
    Q_ASSERT( startAddress >= MIN_ADDRESS && startAddress <= MAX_ADDRESS );
    Q_ASSERT( lengthOfData > 0 );
    if ( startAddress < MIN_ADDRESS || startAddress > MAX_ADDRESS )
    {
        // Error handling
        this->setStatus( 1 );
        qCritical( logProt ) << QObject::tr( "During reading, the incorrect data occurs: startAddress=%1" ).arg( startAddress );
    }

    if ( lengthOfData == 0 )
    {
        // Error handling
        this->setStatus( 1 );
        qCritical( logProt ) << QObject::tr( "During reading, the incorrect data occurs: lengthOfData=%1" ).arg( lengthOfData );
    }

    this->checkSum = 0U;

    // Append a read Command
    QByteArray instruction("#r");
    uint8_t byteArray[ WORD_LENGTH ];

    if ( this->isBigEndian() )
    {
//        aspi_endian_htobabe32( byteArray, startAddress );
    }
    else
    {
//        aspi_endian_htobale32( byteArray, startAddress );
    }
    checkSum += startAddress;

    instruction.append( (char*) byteArray, WORD_LENGTH );

    if ( this->isBigEndian() )
    {
//        aspi_endian_htobabe16( byteArray, lengthOfData );
    }
    else
    {
//        aspi_endian_htobale16( byteArray, lengthOfData );
    }
    checkSum += lengthOfData;

    instruction.append( (char*) byteArray, ONE_HALF_WORD );

    return instruction;
}

//! \brief
//! Storage Access Methods for Write with command 'w'
//!
//! \details
//! writes a defined number of bytes to a defined address
//!
//! \param[in] startAddress Start address of the type quint32
//!
//! \param[in] lengthOfData Length of data of the type quint16
//!
//! \param[in] data Data bytes array of the type QByteArray
//!
//! \return instruction a Instruction for Bootloader protocol of type QByteArray
//!
QByteArray ProtocolROMbootLoader::accessWrite( const quint32 startAddress, const quint16 lengthOfData, const QByteArray & data )
{
    // MIN_ADDRESS = 268435456, MAX_ADDRESS = 268632060
    Q_ASSERT( startAddress >= MIN_ADDRESS && startAddress <= MAX_ADDRESS );
    Q_ASSERT( lengthOfData > 0 && ( data.size() == lengthOfData * 4 ) );
    if ( startAddress < MIN_ADDRESS || startAddress > MAX_ADDRESS )
    {
        // Error handling
        this->setStatus( 1 );
        qCritical( logProt ) << QObject::tr( "During write, the incorrect data occurs: startAddress=%1" ).arg( startAddress );
    }

    if ( lengthOfData == 0 || data.size() != lengthOfData * 4 )
    {
        // Error handling
        this->setStatus( 1 );
        qCritical( logProt ) << QObject::tr( "During write, the incorrect data occurs: lengthOfData=%1" ).arg( lengthOfData );
    }

    this->checkSum = 0U;

    // Append a write Command
    QByteArray instruction( "#w" );
    uint8_t byteArray[ WORD_LENGTH ];

    if ( this->isBigEndian() )
    {
//        aspi_endian_htobabe32( byteArray, startAddress );
    }
    else
    {
//        aspi_endian_htobale32( byteArray, startAddress );
    }
    checkSum += startAddress;

    instruction.append( (char*) byteArray, WORD_LENGTH );

    if ( this->isBigEndian() )
    {
//        aspi_endian_htobabe16( byteArray, lengthOfData );
    }
    else
    {
//        aspi_endian_htobale16( byteArray, lengthOfData );
    }
    checkSum += lengthOfData;

    instruction.append( (char*) byteArray, ONE_HALF_WORD );

    if ( this->isBigEndian() )
    {
        quint32 ind;
        for ( ind = 0; ind < data.size() - WORD_LENGTH + 1; ind += WORD_LENGTH )
        {
            instruction.append( data.at( ind + 3 ) );
            instruction.append( data.at( ind + 2 ) );
            instruction.append( data.at( ind + 1 ) );
            instruction.append( data.at( ind ) );
        }
//        checkSum += aspi_endian_htobe32( calc_checksum( (const quint8*)data.data(), data.size()/WORD_LENGTH ) );
    }
    else
    {
        instruction.append( data );
//        checkSum += aspi_endian_htole32( calc_checksum( (const quint8*)data.data(), data.size()/WORD_LENGTH ) );
    }

    return instruction;
}

//! \brief
//! Version request Method with command 'v'
//!
//! \details
//! Get version of hte DLC-X revision
//!
//! \return instruction a Instruction for Bootloader protocol of type QByteArray
//!
QByteArray ProtocolROMbootLoader::requestVersion( void ) const
{
    QByteArray instruction("#v");

    return instruction;
}

//! \brief
//! Reset / Start Method with command 's'
//!
//! \details
//! Reset to address / starts program at defined address.
//! Start address, address to begin writing/reading, 32 bit
//! For programs the address should be within the ASD address range (0x1000.0000-
//! 0x1002.FFFC), for debug reads/writes the whole memory map is possible.
//!
//! \param[in] startAddress Start address of the type quint32
//!
//! \return instruction a Instruction for Bootloader protocol of type QByteArray
//!
QByteArray ProtocolROMbootLoader::requestReset( const quint32 startAddress ) const
{
    Q_ASSERT( startAddress >= MIN_ADDRESS && startAddress <= MAX_ADDRESS );
    if ( startAddress < MIN_ADDRESS || startAddress > MAX_ADDRESS )
    {
        // Error handling
        qCritical( logProt ) << QObject::tr( "During request on the Reset, the incorrect data occurs: startAddress=%1" ).arg( startAddress );
    }

    QByteArray instruction("#s");

    //! \todo Big endian or Little endian?
    QString byteEndian = "bigEndian";
    uint8_t byteArray[ WORD_LENGTH ];

    if ( byteEndian == "bigEndian" )
    {
//        aspi_endian_htobabe32( byteArray, startAddress );
    }
    else
    {
//        aspi_endian_htobale32( byteArray, startAddress );
    }

    instruction.append( (char*) byteArray, WORD_LENGTH );

    return instruction;
}

//! \brief ProtocolROMbootLoader::checkData
//! The method checks the data integrity.
//!
//! \details
//! The input data must contain a (CCCC) checksum at the end of the array.
//! Otherwise method can not properly check the integrity of the data.
//! [status byte][.][CCCC][.] or [status byte][.][data][CCCC][.]
//!
//! \param[in] receivedData of type QByteArray
//!
//! \retval true  if the verification of the data was OK;
//! \retval false if the verification of the data was not OK.
//!
bool ProtocolROMbootLoader::checkData( const QByteArray & receivedData )
{
    Q_ASSERT( !receivedData.isNull() );
    if ( receivedData.isNull() )
    {
        // Error handling
        this->setStatus( 1 );
        qCritical( logProt ) << QObject::tr( "During check of the data (check sum), the incorrect data occurs." );
        return false;
    }

    const quint8 *pr = (const quint8*)receivedData.data();

    // Get the checksum from the received data
    quint32 check_Sum = get_word32( &pr[ receivedData.size() - WORD_LENGTH - 1 ] );

    if ( this->checkSum == check_Sum )
    {
        return true;
    }
    else if ( checkSum + calculateChecksum( receivedData ) == check_Sum )
    {
         return true;
    }
    return false;
}

//!
//! \brief
//! Calculate checksum for the bytes array
//!
//! \details
//!
//! \param[in] data of type QByteArray
//!
//! \return returnValue the desired value of the type quint32
//!
//! \note
//!
quint32 ProtocolROMbootLoader::calculateChecksum( const QByteArray & data )
{
    Q_ASSERT( !data.isNull() );
    if ( data.isNull() )
    {
        // Error handling
        this->setStatus( 1 );
        qCritical( logProt ) << QObject::tr( "During Calculate checksum (check sum), the incorrect data occurs." );
    }

    // Add data (DDD...)
    QByteArray tmp = data;
    tmp.remove( data.size() - WORD_LENGTH - 1, WORD_LENGTH + 1 );
    tmp.remove( 0, 2 );

    return calc_checksum( (const quint8*)tmp.data(), tmp.size()/WORD_LENGTH );
}

quint16 ProtocolROMbootLoader::get_word16( const quint8 *data )
{
    quint32 word16 = 0U;

    if ( this->isBigEndian() )
    {
        word16 |= ( (quint32) data[0] ) << 8;
        word16 |= ( (quint32) data[1] ) << 0;
    }
    else
    {
        word16 |= ( (quint32) data[0] ) << 0;
        word16 |= ( (quint32) data[1] ) << 8;
    }

    return word16;
}

quint32 ProtocolROMbootLoader::get_word32( const quint8 *data )
{
    quint32 word32 = 0U;

    if ( this->isBigEndian() )
    {
        word32 |= ( (quint32) data[0] ) << 24;
        word32 |= ( (quint32) data[1] ) << 16;
        word32 |= ( (quint32) data[2] ) << 8;
        word32 |= ( (quint32) data[3] ) << 0;
    }
    else
    {
        word32 |= ( (quint32) data[0] ) << 0;
        word32 |= ( (quint32) data[1] ) << 8;
        word32 |= ( (quint32) data[2] ) << 16;
        word32 |= ( (quint32) data[3] ) << 24;
    }

    return word32;
}

quint32 ProtocolROMbootLoader::calc_checksum( const quint8 *data, quint32 len_words )
{
    quint32 check_sum = 0U;
    quint32 ind;

    for ( ind = 0; ind < len_words * WORD_LENGTH; ind += WORD_LENGTH )
    {
        check_sum += get_word32( &data[ ind ] );
    }

    return check_sum;
}

//! \brief ProtocolROMbootLoader::checkStatus
//! The method checks the status byte of return signal
//!
//! \details
//! Status:
//! 1 is OK, program counter is set to start program address
//! 0 is error, waits for other instructions
//! Return data: [status byte][.][CCCC][.] or [status byte][.][data][CCCC][.]
//!
//! \param[in] data of type QByteArray
//!
//! \retval true  if the verification of the data was OK;
//! \retval false if the verification of the data was not OK.
//!
bool ProtocolROMbootLoader::checkStatus( const QByteArray & data )
{
    Q_ASSERT( !data.isNull() );
    if ( data.isNull() || data.isEmpty() )
    {
        // Error handling
        this->setStatus( 1 );
        qCritical( logProt ) << QObject::tr( "During Calculate the Status, the incorrect data occurs." );
        return false;
    }
    if ( data.at( 0 ) == 1U )
    {
        return true;
    }
    return false;
}

//! \brief ProtocolROMbootLoader::setBigEndian
//! The method sets Big endian setting
//!
//! \details
//! true set Big endian, false set Little endian
//!
//! \param[in] endian of type const bool
//!
void ProtocolROMbootLoader::setBigEndian( const bool endian )
{
    this->byteEndian = endian;
}


//! \brief ProtocolROMbootLoader::isBigEndian
//! The method gets endian setting
//!
//! \details
//! true if Big endian, otherwise false
//!
//! \retval true  if Big endian;
//! \retval false if Little endian.
//!
bool ProtocolROMbootLoader::isBigEndian( void ) const
{
    return this->byteEndian;
}

//! \brief ProtocolROMbootLoader::getCheckSum
//!
//! \details
//! Get a check sum for the send command
//!
//! \return checkSum of the type quint32
//!
quint32 ProtocolROMbootLoader::getCheckSum( void ) const
{
    return this->checkSum;
}
