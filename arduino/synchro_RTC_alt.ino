//------------------------------------------------------------------------------
//  Home Office
//  Nürnberg, Germany
//  E-Mail: sergej1@email.ua
//  Version 2.0.0 / is compatible with SynchroTimeApp version 2.x.x
//  Copyright (C) 2022 free Project SynchroTime. All rights reserved.
//------------------------------------------------------------------------------
/*
This sketch performs as a server on an arduino controller for connecting the PC with an RTC DS3231 module via a serial port.
  Built-in server functions allow you to:
  - adjust the RTC DS3231 time in accordance with the reference time of your computer;
  - correct the frequency drift of the RTC DS3231 oscillator;
  - evaluate the accuracy and reliability of the RTC oscillator for a specific sample,
    as well as the chances of a successful correction in the event of a significant frequency drift;
  - save parameters and calibration data to the energy-independent flash memory EEPROM arduino;
  - read value from the Aging register;
  - write value to the Aging register.
The settings are:
  - Set your local time zone in the settings of the application synchroTimeApp.
    time zone = Difference of the local time and UTC-time, a value from { -12, .., -2, -1, 0, +1, +2, +3, .., +12, +13, +14 }.
    For example, time zone for Central Europe = +1/+2, depending on which season is winter (+1) or summer time (+2).
  - MIN_TIME_SPAN the minimum time required for a stable calculation of the frequency drift.
Dependencies:
  - Arduino IDE version >= 1.8.13 (!Replace compilation flags from -Os to -O2);
  - Adafruit RTC library for Arduino RTClib version >= 1.13 (https://github.com/adafruit/RTClib).
Connecting DS3231 MINI module to arduino board:
  - VCC and GND of RTC DS3231 module should be connected to some power source +5V
  - SDA, SCL of RTC DS3231 module should be connected to SDA - data line, SCL - clock line of arduino (for arduino Nano this are A4 and A5)
*/

/*
 * Изменено poty для исключения подключений SQW (и соответствующего прерывания) и замена EEPROM из модуля на Ардуино
 */

#include <Wire.h>
#include <EEPROM.h>
#include "RTClib.h"

//#define INTERRUPT_PIN  2      // Interrupt pin (for Arduino Uno = 2 or 3)
#define STARTBYTE 0x40        // The starting byte of the data set from the communication protocol.
#define DS3231_ADDRESS 0x68   // I2C address for DS3231
#define DS3231_AGINGREG 0x10  // Aging offset register address
#define DS3231_TEMPERATUREREG 0x11 // Temperature register (high byte - low byte is at 0x12), 10-bit temperature value
//#define EEPROM_ADDRESS 0x57   // AT24C256 address (256 kbit = 32 kbyte serial EEPROM)
#define MIN_TIME_SPAN 100000  // The minimum time required for a stable calculation of the frequency drift [in secs]. Default value 200000.
typedef enum task : uint8_t { TASK_IDLE, TASK_ADJUST, TASK_INFO, TASK_CALIBR, TASK_RESET, TASK_SETREG, TASK_STATUS, TASK_WRONG } task_t;
typedef struct time_s {
  uint32_t utc;
  uint16_t milliSecs;
} time_t;

RTC_DS3231 rtc;
static uint8_t buff[4];  // temporary buffer
static uint8_t byteBuffer[18];
// ---- poty ----
int8_t offsetReg;
uint8_t previousSec;
// --------------

// Function Prototypes
int8_t readFromOffsetReg( void ); // read from offset register
bool writeToOffsetReg( const int8_t value ); // write to offset register
static inline void memcpy_byte( void *__restrict__ dstp, const void *__restrict__ srcp, uint16_t len );
inline void intToHex( uint8_t* const buff, const uint32_t value );
inline void floatToHex( uint8_t* const buff, const float value );
uint32_t hexToInt( const uint8_t* const buff );
bool adjustTimeDrift( float drift_in_ppm );
static float calculateDrift_ppm( const time_t* const ref, const time_t* const t );
static uint8_t sumOfBytes( const uint8_t* const bbuffer, const uint8_t blength );
static int16_t read_Temperature( void );

// ---- poty ----перенеcено сюда, так как теперь используется в setup()
uint32_t tickCounter;
uint32_t nowMillis;
// --------------

void setup () {
  // блок инициализации Serial-интерфейса
  Serial.begin( 115200 ); // initialization serial port with 115200 baud (_standard_)
  while ( !Serial );      // wait for serial port to connect. Needed for native USB

  // инициализация DS3231
  if ( !rtc.begin() ) {
    Serial.print( F( "Couldn't find DS3231 modul" ) );
    Serial.flush();
    abort();
  }
  
/* ---- poty ---- отключение SQW
  Ds3231SqwPinMode mode = rtc.readSqwPinMode();
  if ( mode != DS3231_SquareWave1Hz ) {
    rtc.writeSqwPinMode( DS3231_SquareWave1Hz );
  }*/
  // ---- poty ----
  rtc.writeSqwPinMode ( DS3231_OFF );
  // --------------

  if ( rtc.lostPower() ) {
    // If the RTC have lost power it will sets the RTC to the date & time this sketch was compiled in the following line
    const uint32_t newtime = DateTime( F(__DATE__), F(__TIME__) ).unixtime();
    
    // ---- poty ----запоминаем секунды установленного времени
    previousSec = DateTime( newtime ).second();
    tickCounter = millis();
    // --------------
    
    // offset value from -128 to +127, default is 0
    /* удалено poty - отсутствует EEPROM в mini-версиях DS3231
    uint8_t offset_val = i2c_eeprom_read_byte( EEPROM_ADDRESS, 4U );
    */
    rtc.adjust( newtime );
    /* удалено poty - отсутствует EEPROM в mini-версиях DS3231, EEPROM в Arduino бесполезен, так как не привязан к модулю
    if ( offset_val != 0xFF ) {
        writeToOffsetReg( int8_t( offset_val ) );
    }
    */
    // ---- poty ---- читаем регистр Aging
    offsetReg = readFromOffsetReg();    
  } else {
    // ищем границу секунды
    DateTime _now = rtc.now();      // читаем время из DS3231
    previousSec = _now.second();    // запоминаем секунду
    do {                            // читаем время из DS3231 пока не сменится секунда
      _now = rtc.now();
    } while ( previousSec == _now.second() );
    previousSec = _now.second();
    tickCounter = millis();
  }
  // --------------
  
  /* удалено poty - отключаем прерывание
  pinMode( INTERRUPT_PIN, INPUT_PULLUP );
  attachInterrupt( digitalPinToInterrupt( INTERRUPT_PIN ), oneHz, FALLING );
  */
  Serial.print( F( "Boot.. Ok, T[°С]=" ));
  const int16_t temp = read_Temperature();
  Serial.print( float( temp >> 8 ) + ( (temp & 0xFF) >> 6 ) * 0.25f );
}

/* удалено poty - фракция миллисекунд определяется другим способом
volatile uint32_t tickCounter;

void oneHz( void ) {
  tickCounter = millis();
}
*/

void loop () {
  task_t task = TASK_IDLE;
  bool ok = false;
  uint8_t byteCounter = 0U;
  uint8_t numberOfBytes = 0U;
  float drift_in_ppm = .0f;
  time_t t, ref;

  // ---- poty ---- расчёт фракции миллисекунд
  DateTime _now = rtc.now();      // читаем время из DS3231
  if ( previousSec != _now.second() ) {
    previousSec = _now.second();
    tickCounter = millis();
  }
  // --------------
  
  if ( Serial.available() > 1 && Serial.read() == STARTBYTE ) {       // if there is data available

    /* удалено poty - определяем фракцию миллисекунд
    while ( millis() - tickCounter > 998 );
    t.milliSecs = millis() - tickCounter;
    */
    // ---- poty ---- начало определения фракции millis()
    nowMillis = millis() - tickCounter;
    t.utc = _now.unixtime(); // reading clock time as UTC-time
    while ( nowMillis > 1000 ) nowMillis -= 1000;
    t.milliSecs = nowMillis;
    // --------------
    
    // Command Parser
    char thisChar = Serial.read();  // read the byte of request
    switch ( thisChar )
    {
      case 'a':                     // time adjustment request
        numberOfBytes = Serial.readBytes( byteBuffer, 7 );
        task = TASK_ADJUST;
        break;
      case 'i':                     // information request
        numberOfBytes = Serial.readBytes( byteBuffer, 7 );
        task = TASK_INFO;
        break;
      case 'c':                     // calibrating request
        numberOfBytes = Serial.readBytes( byteBuffer, 7 );
        task = TASK_CALIBR;
        break;
      case 'r':                     // reset request
        numberOfBytes = Serial.readBytes( byteBuffer, 1 );
        task = TASK_RESET;
        break;
      case 's':                     // set offset reg. request
        numberOfBytes = Serial.readBytes( byteBuffer, 5 );
        task = TASK_SETREG;
        break;
      case 't':                     // status request
        numberOfBytes = Serial.readBytes( byteBuffer, 1 );
        task = TASK_STATUS;
        break;
      default:                      // unknown request
        task = TASK_IDLE;
        Serial.print( F("Unknown Request ") );
        Serial.print( thisChar );
    }

    // Data Parser
    uint8_t crc = 0U;
    uint8_t sum = uint8_t( thisChar );
    if ( numberOfBytes > sizeof( ref ) ) {
      // reading reference time if data is available. in the form [sec|ms] = 4+2 bytes
      memcpy_byte( &ref, byteBuffer, sizeof( ref ));
      crc = byteBuffer[ sizeof( ref )];
      sum += sumOfBytes( byteBuffer, sizeof( ref ));
    }
    else if ( numberOfBytes > sizeof( drift_in_ppm ) ) {
      // reading new value for the offset reg. in the form [float] = 4 bytes
      
      memcpy_byte( &drift_in_ppm, byteBuffer, sizeof( drift_in_ppm ));
      crc = byteBuffer[ sizeof( drift_in_ppm )];
      sum += sumOfBytes( byteBuffer, sizeof( drift_in_ppm ));
    }
    else {
      crc = byteBuffer[0];
    }

    // checksum verification
    if ( crc != sum ) {
      task = TASK_IDLE;
      Serial.print( F("Invalid Data") );
    }
    else if ( task != TASK_IDLE ) {
      byteBuffer[byteCounter] = STARTBYTE;
      byteCounter++;
    }
  }

  switch ( task )
  {
    case TASK_ADJUST:               // adjust time
      rtc.adjust( DateTime( ref.utc ) );

      // ---- poty ----
      previousSec = DateTime( ref.utc ).second();
      tickCounter = millis();
      // --------------
      
      ok = true;
      byteBuffer[byteCounter] = ok;
      byteCounter++;
      task = TASK_IDLE;
      break;
    case TASK_INFO:                 // information
      memcpy_byte( byteBuffer + byteCounter, &t, sizeof( t ) );  // write time to buffer bytes
      byteCounter += sizeof( t );
      byteBuffer[byteCounter] = readFromOffsetReg();  // reading offset value
      byteCounter++;
      drift_in_ppm = calculateDrift_ppm( &ref, &t );  // calculate drift time
      floatToHex( byteBuffer + byteCounter, drift_in_ppm );
      byteCounter += sizeof( drift_in_ppm );
      // ---- poty ---- подмена функций EEPROM
//      if ( i2c_eeprom_read_buffer( EEPROM_ADDRESS, 0U, byteBuffer + byteCounter, sizeof( uint32_t )) ) {
      if ( eeprom_read_buffer( 101U, byteBuffer + byteCounter, sizeof( uint32_t )) ) {
        byteCounter += sizeof( uint32_t );
      }
      task = TASK_IDLE;
      break;
    case TASK_CALIBR:               // calibrating
      byteBuffer[byteCounter] = readFromOffsetReg();  // read last value from the offset register
      byteCounter++;
      drift_in_ppm = calculateDrift_ppm( &ref, &t );  // calculate drift time
      floatToHex( byteBuffer + byteCounter, drift_in_ppm ); // read drift as float value
      byteCounter += sizeof(drift_in_ppm);
      ok = adjustTimeDrift( drift_in_ppm );
      if ( ok ) {
        rtc.adjust( DateTime( ref.utc ) ); // adjust time

        // ---- poty ----
        previousSec = DateTime( ref.utc ).second();
        tickCounter = millis();
        // --------------
        
        byteBuffer[byteCounter] = readFromOffsetReg();  // read new value from the offset register
        byteCounter++;
      }
      byteBuffer[byteCounter] = ok;
      byteCounter++;
      task = TASK_IDLE;
      break;
    case TASK_RESET:                // reset
      ok = writeToOffsetReg( 0 );
      if ( ok ) {
        uint8_t buff5b[5];
        for (numberOfBytes = 0; numberOfBytes < sizeof(buff5b); numberOfBytes++ ) buff5b[numberOfBytes] = 0xFF;
        // ---- poty ---- подмена функций EEPROM
//        ok &= i2c_eeprom_write_page( EEPROM_ADDRESS, 0U, buff5b, sizeof( buff5b ) );
        ok &= eeprom_write_page( 101U, buff5b, sizeof( buff5b ) );
      }
      byteBuffer[byteCounter] = ok;
      byteCounter++;
      task = TASK_IDLE;
      break;
    case TASK_SETREG:               // set register
      ok = adjustTimeDrift( drift_in_ppm );
      byteBuffer[byteCounter] = ok;
      byteCounter++;
      task = TASK_IDLE;
      break;
    case TASK_STATUS:               // get status
      byteBuffer[byteCounter] = 0x00;
      byteCounter++;
      task = TASK_IDLE;
      break;
    case TASK_IDLE:                 // idle task
      break;
    default:
      Serial.print( F("Unknown Task ") );
      Serial.print( task, HEX );
      task = TASK_IDLE;
  }
// Response to the request
  if ( byteCounter > 0 ) {
    byteBuffer[byteCounter] = sumOfBytes( byteBuffer, byteCounter );
    Serial.write( byteBuffer, ++byteCounter );
    Serial.flush();
  }
}

int8_t readFromOffsetReg( void ) {
  Wire.beginTransmission( DS3231_ADDRESS ); // Sets the DS3231 RTC module address
  Wire.write( uint8_t( DS3231_AGINGREG ) ); // sets the offset register address
  Wire.endTransmission();
  int8_t offset_val = 0;
  Wire.requestFrom( uint8_t( DS3231_ADDRESS ), uint8_t(1) ); // Read a byte from register
  offset_val = int8_t( Wire.read() );
  return offset_val;
}

bool writeToOffsetReg( const int8_t value ) {
  Wire.beginTransmission( DS3231_ADDRESS ); // Sets the DS3231 RTC module address
  Wire.write( uint8_t( DS3231_AGINGREG ) ); // sets the offset register address
  Wire.write( value ); // Write value to register
  return ( Wire.endTransmission() == 0 );
}

inline void intToHex( uint8_t* const buff, const uint32_t value ) {
  memcpy_byte( buff, &value, sizeof(value) );
}

inline void floatToHex( uint8_t* const buff, const float value ) {
  memcpy_byte( buff, &value, sizeof(value) );
}

uint32_t hexToInt( const uint8_t* const buff ) {
  uint32_t *y = (uint32_t *)buff;
  return y[0];
}

// the result is rounded to the maximum possible values of type uint8_t
bool adjustTimeDrift( float drift_in_ppm ) {
  drift_in_ppm *= 10;
  int32_t offset = (drift_in_ppm > .0f) ? ( drift_in_ppm + 0.5f ) : ( drift_in_ppm - 0.5f );
  if ( offset == 0 ) return true;  // if offset is 0, nothing needs to be done
  const int8_t last_offset_reg = readFromOffsetReg();
  // ---- poty ---- подмена функций EEPROM
  //const int8_t last_offset_ee = i2c_eeprom_read_byte( EEPROM_ADDRESS, 4U );
  //if ( last_offset_reg == last_offset_ee ) {
    drift_in_ppm += last_offset_reg;
    offset = (drift_in_ppm > .0f) ? ( drift_in_ppm + 0.5f ) : ( drift_in_ppm - 0.5f );
  //}
  offset = (offset > 127) ? 127 : (offset < -128) ? -128 : offset;
  // ---- poty ---- подмена функций EEPROM
  //bool ok = i2c_eeprom_write_byte( EEPROM_ADDRESS, 4U, offset );  // write offset value to EEPROM of AT24C256
  bool ok = true;
  ok &= writeToOffsetReg( offset );  // write offset value to Offset Reg. of DS3231
  return ok;
}

/*
   "drift in ppm unit" - this is the ratio of the clock drift from the reference time,
   which is expressed in terms of one million control seconds.
   For example, reference_time = 1597590292 sec, clock_time = 1597590276 sec, last_set_time = 1596628800 sec,
   time_drift = clock_time - reference_time = -16 sec
   number_of_control_seconds = reference_time - last_set_time = 961492 sec, i.e 0.961492*10^6 sec
   drift_in_ppm = time_drift * 10^6 / number_of_control_seconds = -16*10^6 /(0.961492*10^6) = -16.64 ppm
*/
static float calculateDrift_ppm( const time_t* const referenceTime, const time_t* const clockTime ) {
  // ---- poty ---- подмена функций EEPROM
//  if ( !i2c_eeprom_read_buffer( EEPROM_ADDRESS, 0U, buff, sizeof(buff)) ) {
  if ( !eeprom_read_buffer( 101U, buff, sizeof(buff)) ) {
    return 0;
  }
  const uint32_t last_set_timeUTC = hexToInt( buff );
  const int32_t diff = referenceTime->utc - last_set_timeUTC;
  // verification is needed because the var. last_set_timeSecs can reach the overflow value
  if ( referenceTime->utc < last_set_timeUTC || diff < MIN_TIME_SPAN ) {
    return 0;
  }
  const int32_t time_driftSecs = clockTime->utc - referenceTime->utc;
  const int16_t time_driftMs = clockTime->milliSecs - referenceTime->milliSecs;
  const float time_drift = time_driftSecs * 1000 + time_driftMs;
  return time_drift * 1000 / diff;
}

static uint8_t sumOfBytes( const uint8_t* const bbuffer, const uint8_t blength ) {
  uint8_t sum = 0U;
  for ( uint8_t idx = 0U; idx < blength; idx++ ) {
    sum += bbuffer[idx];
  }
  return sum;
}


/* ---- poty ---- замена функций
uint8_t i2c_eeprom_read_byte( int deviceAddress, unsigned int eeAddress ) {
  uint8_t rdata = 0xFF;
  Wire.beginTransmission( deviceAddress );
  Wire.write( (int)( eeAddress >> 8 ) ); // MSB
  Wire.write( (int)( eeAddress & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom( deviceAddress, 1 );
  if ( Wire.available() ) rdata = Wire.read();
  return rdata;
}
*/
uint8_t eeprom_read_byte( unsigned int eeAddress ) {
  return EEPROM.read( eeAddress );
}


/*
bool i2c_eeprom_read_buffer( int deviceAddress, unsigned int eeAddress, uint8_t* const buffer, int length ) {
  Wire.beginTransmission( deviceAddress );
  Wire.write( (int)( eeAddress >> 8 ) ); // MSB
  Wire.write( (int)( eeAddress & 0xFF ) ); // LSB
  bool ret_val = ( Wire.endTransmission() == 0 );
  Wire.requestFrom( deviceAddress, length );
  int i;
  for ( i = 0; i < length; i++ ) {
    if ( Wire.available() ) {
      buffer[i] = Wire.read();
    }
  }
  return ret_val;
}
*/
bool eeprom_read_buffer( unsigned int eeAddress, uint8_t* const buffer, int length ) {
  int i;
  for ( i = 0; i < length; i++ ) {
    buffer[i] = EEPROM.read( eeAddress + i );
  }
  return true;
}

/*
bool i2c_eeprom_write_byte( int deviceAddress, unsigned int eeAddress, uint8_t data ) {
  int rdata = data;
  Wire.beginTransmission( deviceAddress );
  Wire.write( (int)( eeAddress >> 8 ) ); // MSB
  Wire.write( (int)( eeAddress & 0xFF)); // LSB
  Wire.write( rdata );
  return ( Wire.endTransmission() == 0 );
}
*/
bool eeprom_write_byte( unsigned int eeAddress, uint8_t data ) {
  EEPROM.write( eeAddress, data );
  return true;
}


/*
   WARNING: address is a page address, 6-bit end will wrap around
   also, data can be maximum of about 30 bytes, because the Wire library has a buffer of 32 bytes
*/
/*
bool i2c_eeprom_write_page( int deviceAddress, unsigned int eeAddressPage, const uint8_t* data, uint8_t length ) {
  Wire.beginTransmission( deviceAddress );
  Wire.write( (int)( eeAddressPage >> 8 ) ); // MSB
  Wire.write( (int)( eeAddressPage & 0xFF ) ); // LSB
  uint8_t i;
  for ( i = 0; i < length; i++ ) {
    Wire.write( data[i] );
  }
  return ( Wire.endTransmission() == 0 );
}
*/
bool eeprom_write_page( unsigned int eeAddressPage, const uint8_t* data, uint8_t length ) {
  uint8_t i;
  for ( i = 0; i < length; i++ ) {
    EEPROM.write( eeAddressPage + i, data[i] );
  }
  return true;
}

static inline void memcpy_byte( void *__restrict__ dstp, const void *__restrict__ srcp, uint16_t len ) {
    uint8_t *dst = ( uint8_t *) dstp;
    const uint8_t *src = ( uint8_t *) srcp;
    uint16_t idx;
    for( idx = 0U; idx < len; idx++ )
        *(dst++) = *(src++);
}

static int16_t read_Temperature() {
  int16_t temp = 0;
  Wire.beginTransmission( DS3231_ADDRESS );
  Wire.write( DS3231_TEMPERATUREREG );
  Wire.endTransmission();

  Wire.requestFrom( DS3231_ADDRESS, 2 );
  temp = Wire.read();
  temp = (temp << 8) | Wire.read();
  return temp;
}
