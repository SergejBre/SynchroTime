//------------------------------------------------------------------------------
//  Home Office
//  Nürnberg, Germany
//  E-Mail: sergej1@email.ua
//
//  Copyright (C) 2020 free Project SynchroTime. All rights reserved.
//------------------------------------------------------------------------------
/*
 * Sketch for old versions of SynchroTime, including version 1.1.2 !
 * 
This sketch performs as a server on an arduino controller for connecting a computer
  with an RTC DS3231 module via a serial port.Built-in server functions allow you to:
  - adjust the RTC DS3231 time in accordance with the reference time of your computer;
  - correct the time shift of the RTC DS3231 clock;
  - evaluate the accuracy and reliability of the RTC oscillator for a specific sample,
    as well as the chances of a successful correction in the event of a significant time drift;
  - save parameters and calibration data to the energy-independent flash memory;
  - read from the offset register;
  - write to the offset register;
  - etc.
The settings are:
  - The selection of the time zone, which is determined as the local local time on the worker computer.
    time_zone = Difference of the UTC-time. A value from { -12, .., -2, -1, 0, +1, +2, +3, .., +12 }
    +1/+2 for Europe, depending on which season is winter (+1) or summer time (+2).
  - MIN_TIME_SPAN the minimum time required for a stable calculation of the time drift.
*/
#include <Wire.h>
#include "RTClib.h"

#define TIME_ZONE 1           // Difference to UTC-time on the work computer, from { -12, .., -2, -1, 0, +1, +2, +3, .., +12 }
#define INTERRUPT_PIN  2      // Interrupt pin (for Arduino Uno = 2 or 3)
#define OFFSET_REGISTER 0x10  // Aging offset register address
#define CONRTOL_REGISTER 0x0E // Control Register address
#define EEPROM_ADDRESS 0x57   // AT24C256 address (256 kbit = 32 kbyte serial EEPROM)
#define MIN_TIME_SPAN 200000  // The minimum time required for a stable calculation of the time drift.
typedef enum task : uint8_t { TASK_IDLE, TASK_ADJUST, TASK_INFO, TASK_CALIBR, TASK_RESET, TASK_SETREG, TASK_STATUS, TASK_WRONG } task_t;
typedef struct time_s {
  uint32_t utc;
  uint16_t milliSecs;
} time_t;

RTC_DS3231 rtc;
uint8_t buff[4];  // temporary buffer

// Function Prototypes
int8_t readFromOffsetReg( void ); // read from offset register
bool writeToOffsetReg( const int8_t value ); // write to offset register
bool setControlRegTo1Hz( void ); // Control Register to 1 Hz Out (SQW-pin)
inline void intToHex( uint8_t* const buff, const uint32_t value );
inline void floatToHex( uint8_t* const buff, const float value );
uint32_t hexToInt( const uint8_t* const buff );
uint32_t getUTCtime( const uint32_t localTimeSecs );
bool adjustTime( const uint32_t utcTimeSecs );
bool adjustTimeDrift( float drift_in_ppm );
float calculateDrift_ppm( const time_t* const ref, const time_t* const t );

void setup () {
  Serial.begin( 115200 ); // initialization serial port with 115200 baud (_standard_)
  while ( !Serial );      // wait for serial port to connect. Needed for native USB
  Serial.setTimeout( 5 ); // timeout 5ms

  if ( !rtc.begin() ) {
    Serial.println( F( "Couldn't find DS3231 modul" ) );
    Serial.flush();
    abort();
  }

  Ds3231SqwPinMode mode = rtc.readSqwPinMode();
  if ( mode != DS3231_SquareWave1Hz ) {
    rtc.writeSqwPinMode( DS3231_SquareWave1Hz );
  }

  if ( rtc.lostPower() ) {
//    Serial.println( F("RTC lost power, lets set the time!") );
    // If the RTC have lost power it will sets the RTC to the date & time this sketch was compiled in the following line
    const uint32_t newtime = DateTime( F(__DATE__), F(__TIME__) ).unixtime();
    // offset value from -128 to +127, default is 0
    uint8_t offset_val = i2c_eeprom_read_byte( EEPROM_ADDRESS, 4U );
    adjustTime( newtime - TIME_ZONE * 3600 );

    if ( offset_val != 0xFF ) {
        writeToOffsetReg( int8_t( offset_val ) );
//        Serial.print( F( "Set Offset Reg: " ) );
//        Serial.println( offset_val, DEC );
    }
  }
  pinMode( INTERRUPT_PIN, INPUT_PULLUP );
  attachInterrupt( digitalPinToInterrupt( INTERRUPT_PIN ), oneHz, FALLING );
}

volatile uint32_t tickCounter;

void oneHz( void ) {
  tickCounter = millis();
}

void loop () {
  uint8_t byteBuffer[16];
  task_t task = TASK_IDLE;
  bool ok = false;
  uint8_t set = 0U;
  uint8_t numberOfBytes = 0U;
  float drift_in_ppm = 0;
  time_t t;
  time_t ref = {0, 0};

  if ( Serial.available() > 1 && Serial.read() == '@' ) {       // if there is data available

    while ( millis() - tickCounter > 998 );
    t.milliSecs = millis() - tickCounter;
    DateTime now = rtc.now();       // reading clock time
    t.utc = getUTCtime( now.unixtime() ); // reading clock time as UTC-time

    // Command Parser
    char thisChar = Serial.read();  // read the byte of request
    switch ( thisChar )
    {
      case 'a':                     // time adjustment request
        task = TASK_ADJUST;
        break;
      case 'i':                     // information request
        task = TASK_INFO;
        break;
      case 'c':                     // calibrating request
        task = TASK_CALIBR;
        break;
      case 'r':                     // reset request
        task = TASK_RESET;
        break;
      case 's':                     // set offset reg. request
        task = TASK_SETREG;
        break;
      case 't':                     // status request
        task = TASK_STATUS;
        break;
      default:                      // unknown request
        task = TASK_IDLE;
        Serial.print( F("Unknown Request ") );
        Serial.print( thisChar );
    }

    // Data Parser
    if ( Serial.available() > 0 ) {
      numberOfBytes = Serial.readBytes( byteBuffer, 6 );
      if ( numberOfBytes >= sizeof( ref ) ) {
        // reading reference time if data is available. in the form [sec|ms] = 4+2 bytes
        memcpy( &ref, byteBuffer, sizeof( ref ));
      }
      else if ( numberOfBytes >= sizeof( drift_in_ppm ) ) {
        // reading new value for the offset reg. in the form [float] = 4 bytes
        memcpy( &drift_in_ppm, byteBuffer, sizeof( drift_in_ppm ));
      }
    }
  }

  switch ( task )
  {
    case TASK_ADJUST:               // adjust time
//      oneHz();
      ok = adjustTime( ref.utc );
      byteBuffer[set] = ok;
      set++;
      task = TASK_IDLE;
      break;
    case TASK_INFO:                 // information
      memcpy( byteBuffer, &t, sizeof( t ) );  // write time to buffer bytes
      set += sizeof( t );
      byteBuffer[set] = readFromOffsetReg();  // reading offset value
      set++;
      drift_in_ppm = calculateDrift_ppm( &ref, &t );  // calculate drift time
      floatToHex( byteBuffer + set, drift_in_ppm );
      set += sizeof( drift_in_ppm );
      if ( i2c_eeprom_read_buffer( EEPROM_ADDRESS, 0U, byteBuffer + set, sizeof( uint32_t )) ) {
        set += sizeof( uint32_t );
      }
      task = TASK_IDLE;
      break;
    case TASK_CALIBR:               // calibrating
      byteBuffer[set] = readFromOffsetReg();  // read last value from the offset register
      set++;
      drift_in_ppm = calculateDrift_ppm( &ref, &t );  // calculate drift time
      floatToHex( byteBuffer + set, drift_in_ppm ); // read drift as float value
      set += sizeof(drift_in_ppm);
      ok = adjustTimeDrift( drift_in_ppm );
      if ( ok ) {
        ok &= adjustTime( ref.utc ); // adjust time
        byteBuffer[set] = readFromOffsetReg();  // read new value from the offset register
        set++;
      }
      byteBuffer[set] = ok;
      set++;
      task = TASK_IDLE;
      break;
    case TASK_RESET:                // reset
      ok = writeToOffsetReg( 0 );
      if ( ok ) {
        uint8_t buff5b[5];
        for (numberOfBytes = 0; numberOfBytes < sizeof(buff5b); numberOfBytes++ ) buff5b[numberOfBytes] = 0xFF;
        ok &= i2c_eeprom_write_page( EEPROM_ADDRESS, 0U, buff5b, sizeof( buff5b ) );
      }
      byteBuffer[set] = ok;
      set++;
      task = TASK_IDLE;
      break;
    case TASK_SETREG:               // set register
      ok = adjustTimeDrift( drift_in_ppm );
      byteBuffer[set] = ok;
      set++;
      task = TASK_IDLE;
      break;
    case TASK_STATUS:               // get status
      byteBuffer[set] = 0x00;
      set++;
      task = TASK_IDLE;
      break;
    case TASK_IDLE:                 // idle task
      break;
    default:
      Serial.print( F("Unknown Task ") );
      Serial.println( task, HEX );
      task = TASK_IDLE;
  }

  if ( set > 0 ) {
    Serial.write( byteBuffer, set );
  }
}

int8_t readFromOffsetReg( void ) {
  Wire.beginTransmission( DS3231_ADDRESS ); // Sets the DS3231 RTC module address
  Wire.write( uint8_t( OFFSET_REGISTER ) ); // sets the offset register address
  Wire.endTransmission();
  int8_t offset_val = 0x00;
  Wire.requestFrom( DS3231_ADDRESS, 1 ); // Read a byte from register
  offset_val = int8_t( Wire.read() );
  return offset_val;
}

bool writeToOffsetReg( const int8_t value ) {
  Wire.beginTransmission( DS3231_ADDRESS ); // Sets the DS3231 RTC module address
  Wire.write( uint8_t( OFFSET_REGISTER ) ); // sets the offset register address
  Wire.write( value ); // Write value to register
  return ( Wire.endTransmission() == 0 );
}

bool setControlRegTo1Hz( void ) {
  Wire.beginTransmission( DS3231_ADDRESS ); // Sets the DS3231 RTC module address
  Wire.write( uint8_t( CONRTOL_REGISTER ) ); // sets the Control Register address
  Wire.write( B01000000 ); // sets 1 Hz Out (SQW-pin)
  return ( Wire.endTransmission() == 0 );
}

inline void intToHex( uint8_t* const buff, const uint32_t value ) {
  memcpy( buff, &value, sizeof(value) );
}

inline void floatToHex( uint8_t* const buff, const float value ) {
  memcpy( buff, &value, sizeof(value) );
}

uint32_t hexToInt( const uint8_t* const buff ) {
  uint32_t *y = (uint32_t *)buff;
  return y[0];
}

uint32_t getUTCtime( const uint32_t localTimeSecs ) {
  return ( localTimeSecs - TIME_ZONE * 3600 ); // UTC_time = local_Time - TIME_ZONE*3600 sec
}

bool adjustTime( const uint32_t utcTimeSecs ) {
  rtc.adjust( DateTime( utcTimeSecs + TIME_ZONE * 3600 ) );
  intToHex( buff, utcTimeSecs ); // data to write
  return i2c_eeprom_write_page( EEPROM_ADDRESS, 0U, buff, sizeof(buff)); // write last_set_time to EEPROM AT24C256
}

// the result is rounded to the maximum possible values of type uint8_t
bool adjustTimeDrift( float drift_in_ppm ) {
  drift_in_ppm *= 10;
  int offset = (drift_in_ppm > 0) ? ( drift_in_ppm + 0.5 ) : ( drift_in_ppm - 0.5 );
  if ( offset == 0 ) return true;  // if offset is 0, nothing needs to be done
  const int8_t last_offset_reg = readFromOffsetReg();
  const int8_t last_offset_ee = i2c_eeprom_read_byte( EEPROM_ADDRESS, 4U );
  if ( last_offset_reg == last_offset_ee ) {
    drift_in_ppm += last_offset_reg;
    offset = (drift_in_ppm > 0) ? ( drift_in_ppm + 0.5 ) : ( drift_in_ppm - 0.5 );
  }
  offset = (offset > 127) ? 127 : (offset < -128) ? -128 : offset;
  bool ok = i2c_eeprom_write_byte( EEPROM_ADDRESS, 4U, offset );  // write offset value to EEPROM of AT24C256
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
float calculateDrift_ppm( const time_t* const referenceTime, const time_t* const clockTime ) {
  if ( !i2c_eeprom_read_buffer( EEPROM_ADDRESS, 0U, buff, sizeof(buff)) ) {
    return 0;
  }
  uint32_t last_set_timeUTC = hexToInt( buff );
  int32_t diff = referenceTime->utc - last_set_timeUTC;
  // verification is needed because the var. last_set_timeSecs can reach the overflow value
  if ( referenceTime->utc < last_set_timeUTC || diff < MIN_TIME_SPAN ) {
    return 0;
  }
  int32_t time_driftSecs = clockTime->utc - referenceTime->utc;
  int16_t time_driftMs = clockTime->milliSecs - referenceTime->milliSecs;
  float time_drift = time_driftSecs * 1000 + time_driftMs;
  return time_drift * 1000 / diff;
}

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

bool i2c_eeprom_write_byte( int deviceAddress, unsigned int eeAddress, uint8_t data ) {
  int rdata = data;
  Wire.beginTransmission( deviceAddress );
  Wire.write( (int)( eeAddress >> 8 ) ); // MSB
  Wire.write( (int)( eeAddress & 0xFF)); // LSB
  Wire.write( rdata );
  return ( Wire.endTransmission() == 0 );
}

/*
   WARNING: address is a page address, 6-bit end will wrap around
   also, data can be maximum of about 30 bytes, because the Wire library has a buffer of 32 bytes
*/
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
