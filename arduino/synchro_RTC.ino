//------------------------------------------------------------------------------
//  Home Office
//  Nürnberg, Germany
//  E-Mail: sergej1@email.ua
//  Version 2.0.0 / is compatible with SynchroTimeApp version 2.x.x
//  Copyright (C) 2022 free Project SynchroTime. All rights reserved.
//------------------------------------------------------------------------------
/*
This sketch performs as a server on an arduino controller for connecting the PC with an RTC DS3231 ZS-042 module via a serial port.
  Built-in server functions allow you to:
  - adjust the RTC DS3231 time in accordance with the reference time of your computer;
  - correct the frequency drift of the RTC DS3231 oscillator;
  - evaluate the accuracy and reliability of the RTC oscillator for a specific sample,
    as well as the chances of a successful correction in the event of a significant frequency drift;
  - save parameters and calibration data to the energy-independent flash memory AT24C256;
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
  - SQW should be connected to INTERRUPT_PIN
  - INTERRUPT_PIN needs to work with interrupts
*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <Wire.h>
#include "RTClib.h"

#define INTERRUPT_PIN  2      // Interrupt pin (for Arduino Uno = 2 or 3)
#define STARTBYTE 0x40        // The starting byte of the data set from the communication protocol.
#define DS3231_ADDRESS 0x68   // I2C address for DS3231
#define DS3231_AGINGREG 0x10  // Aging register address
#define DS3231_TEMPERATUREREG 0x11 // Temperature register (high byte - low byte is at 0x12), 10-bit temperature value
#define EEPROM_ADDRESS 0x57   // AT24C256 address (256 kbit = 32 kbyte serial EEPROM)
#define MIN_TIME_SPAN 100000  // The minimum time required for a stable calculation of the frequency drift [in secs]. Default value 200000.
enum task_t : uint8_t { TASK_IDLE, TASK_ADJUST, TASK_INFO, TASK_CALIBR, TASK_RESET, TASK_SETREG, TASK_STATUS, TASK_WRONG };
struct time_t {
  uint32_t utc;       // This is a Coordinated Universal Time (UTC-time) + time-zone offset
  uint16_t milliSecs;
};

RTC_DS3231 rtc;
static uint8_t buff[4];  // temporary buffer
static uint8_t byteBuffer[18];

// Function Prototypes
int8_t readFromAgingReg( void ); // read from Aging register
bool writeToAgingReg( const int8_t value ); // write to Aging register
static inline void memcpy_byte( void *__restrict__ dstp, const void *__restrict__ srcp, uint16_t len );
inline void intToHex( uint8_t* const buff, const uint32_t value );
inline void floatToHex( uint8_t* const buff, const float value );
uint32_t hexToInt( const uint8_t* const buff );
static float calculateDrift_ppm( const time_t* const ref, const time_t* const t );
static int8_t roundUpDrift( float drift_in_ppm );
static uint8_t sumOfBytes( const uint8_t* const bbuffer, const uint8_t blength );
static int16_t read_Temperature( void );
static time_t getTime( void );

void setup () {
  Serial.begin( 115200 ); // initialization serial port with 115200 baud (_standard_)
  while ( !Serial );      // wait for serial port to connect. Needed for native USB

  if ( !rtc.begin() ) {
    Serial.print( F( "Couldn't find DS3231 modul" ) );
    Serial.flush();
    abort();
  }

  Ds3231SqwPinMode mode = rtc.readSqwPinMode();
  if ( mode != DS3231_SquareWave1Hz ) {
    rtc.writeSqwPinMode( DS3231_SquareWave1Hz );
  }
  rtc.disable32K();  //we don't need the 32K Pin, so disable it

  if ( rtc.lostPower() ) {
    // If the RTC have lost power it will sets the RTC to the date & time this sketch was compiled in the following line
    const uint32_t newtime = DateTime( F(__DATE__), F(__TIME__) ).unixtime();
    // Aging value from -128 to +127, default is 0
    uint8_t aging_val = i2c_eeprom_read_byte( EEPROM_ADDRESS, 4U );
    rtc.adjust( newtime );
    intToHex( buff, newtime ); // write time to buffer
    i2c_eeprom_write_page( EEPROM_ADDRESS, 0U, buff, sizeof(buff)); // write last_set_time to EEPROM AT24C256

    if ( aging_val != 0xFF ) {
        writeToAgingReg( int8_t( aging_val ) );
    }
  }
  PORTD |= 0x01 << INTERRUPT_PIN; // set INPUT_PULLUP to INTERRUPT_PIN port
  EICRA &= ~( bit(ISC00) | bit(ISC01) ); // Reset ISC00
  EICRA |= bit(ISC01); // Set ISC01 - tracks FALLING at INT0
  EIMSK |= bit(INT0); // Enable interrupt INT0

  Serial.print( F( "Boot.. Ok, T[°С]=" ));
  const int16_t temp = read_Temperature();
  Serial.print( float( temp >> 8 ) + ( (temp & 0xFF) >> 6 ) * 0.25f );
}

volatile uint32_t tickCounter;

ISR( INT0_vect ) {
  tickCounter = micros();
}

inline void reset( void ) {
  tickCounter = micros();
}

void loop () {
  task_t task = TASK_IDLE;
  bool ok = false;
  uint8_t byteCounter = 0U;
  uint8_t numberOfBytes = 0U;
  float drift_in_ppm = .0f;
  int8_t drift_val;
  time_t t, ref;

  if ( Serial.available() > 1 && Serial.read() == STARTBYTE ) {       // if there is data available
    t = getTime();
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
      case 's':                     // set Aging reg. request
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
      // reading new value for the Aging reg. in the form [float] = 4 bytes
      
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
      reset();                   // reset milliseconds
      rtc.adjust( DateTime( ref.utc ) );
      intToHex( buff, ref.utc ); // write time to buffer
      ok = i2c_eeprom_write_page( EEPROM_ADDRESS, 0U, buff, sizeof(buff)); // write last_set_time to EEPROM AT24C256
      byteBuffer[byteCounter] = ok;
      byteCounter++;
      task = TASK_IDLE;
      break;
    case TASK_INFO:                 // information
      memcpy_byte( byteBuffer + byteCounter, &t, sizeof( t ) );  // write time to buffer bytes
      byteCounter += sizeof( t );
      byteBuffer[byteCounter] = readFromAgingReg();  // reading Aging value
      byteCounter++;
      drift_in_ppm = calculateDrift_ppm( &ref, &t );  // calculate drift time
      floatToHex( byteBuffer + byteCounter, drift_in_ppm );
      byteCounter += sizeof( drift_in_ppm );
      if ( i2c_eeprom_read_buffer( EEPROM_ADDRESS, 0U, byteBuffer + byteCounter, sizeof( uint32_t )) ) {
        byteCounter += sizeof( uint32_t );
      }
      task = TASK_IDLE;
      break;
    case TASK_CALIBR:               // calibrating
      byteBuffer[byteCounter] = readFromAgingReg();  // read last value from the Aging register
      byteCounter++;
      drift_in_ppm = calculateDrift_ppm( &ref, &t );  // calculate drift time
      floatToHex( byteBuffer + byteCounter, drift_in_ppm ); // read drift as float value
      byteCounter += sizeof(drift_in_ppm);
      drift_val = roundUpDrift( drift_in_ppm );
      if ( drift_val != 0 ) {
        ok = i2c_eeprom_write_byte( EEPROM_ADDRESS, 4U, drift_val );  // write drift value to EEPROM of AT24C256
        ok &= writeToAgingReg( drift_val );  // write drift value to Aging Reg. of DS3231
        if ( ok ) {
          reset();                   // reset milliseconds
          rtc.adjust( DateTime( ref.utc ) );     // adjust time
          intToHex( buff, ref.utc ); // write UTC-time + TimeZone to buffer
          ok &= i2c_eeprom_write_page( EEPROM_ADDRESS, 0U, buff, sizeof(buff)); // write last_set_time to EEPROM AT24C256
        }
        byteBuffer[byteCounter] = drift_val;  // read new value from the Aging register
        byteCounter++;
      }
      else {
        ok = true;
        byteBuffer[byteCounter] = byteBuffer[1];  // read value from the Aging register
        byteCounter++;
      }
      byteBuffer[byteCounter] = ok;
      byteCounter++;
      task = TASK_IDLE;
      break;
    case TASK_RESET:                // reset
      ok = writeToAgingReg( 0 );
      if ( ok ) {
        uint8_t buff5b[5];
        for (numberOfBytes = 0; numberOfBytes < sizeof(buff5b); numberOfBytes++ ) buff5b[numberOfBytes] = 0xFF;
        ok &= i2c_eeprom_write_page( EEPROM_ADDRESS, 0U, buff5b, sizeof( buff5b ) );
      }
      byteBuffer[byteCounter] = ok;
      byteCounter++;
      task = TASK_IDLE;
      break;
    case TASK_SETREG:               // set register
      drift_in_ppm *= 10;
      drift_val = (drift_in_ppm > .0f) ? ( drift_in_ppm + 0.5f ) : ( drift_in_ppm - 0.5f );
      ok = i2c_eeprom_write_byte( EEPROM_ADDRESS, 4U, drift_val );  // write drift value to EEPROM of AT24C256
      ok &= writeToAgingReg( drift_val );  // write drift value to Aging Reg. of DS3231
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

int8_t readFromAgingReg( void ) {
  Wire.beginTransmission( DS3231_ADDRESS ); // Sets the DS3231 RTC module address
  Wire.write( uint8_t( DS3231_AGINGREG ) ); // sets the aging register address
  Wire.endTransmission();
  int8_t aging_val = 0;
  Wire.requestFrom( uint8_t( DS3231_ADDRESS ), 1U ); // Read a byte from register
  aging_val = int8_t( Wire.read() );
  return aging_val;
}

bool writeToAgingReg( const int8_t value ) {
  Wire.beginTransmission( DS3231_ADDRESS ); // Sets the DS3231 RTC module address
  Wire.write( uint8_t( DS3231_AGINGREG ) ); // sets the aging register address
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

// the result is rounded to the maximum possible values of type int8_t
static int8_t roundUpDrift( float drift_in_ppm ) {
  drift_in_ppm *= 10;
  int32_t offset = (drift_in_ppm > .0f) ? ( drift_in_ppm + 0.5f ) : ( drift_in_ppm - 0.5f );
  if ( offset == 0 ) {
    return offset;  // if offset is 0, nothing needs to be done
  }
  const int8_t last_aging_reg = readFromAgingReg();
  const int8_t last_aging_ee = (int8_t)i2c_eeprom_read_byte( EEPROM_ADDRESS, 4U );
  if ( last_aging_reg == last_aging_ee ) {
    drift_in_ppm += last_aging_reg;
    offset = (drift_in_ppm > .0f) ? ( drift_in_ppm + 0.5f ) : ( drift_in_ppm - 0.5f );
  }
  return (offset > 127) ? 127 : (offset < -128) ? -128 : offset;
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
  if ( !i2c_eeprom_read_buffer( EEPROM_ADDRESS, 0U, buff, sizeof(buff)) ) {
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

uint8_t i2c_eeprom_read_byte( int deviceAddress, unsigned int eeAddress ) {
  uint8_t rdata = 0xFF;
  Wire.beginTransmission( deviceAddress );
  Wire.write( int( eeAddress >> 8 ) ); // MSB
  Wire.write( int( eeAddress & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom( deviceAddress, 1 );
  if ( Wire.available() ) rdata = Wire.read();
  return rdata;
}

bool i2c_eeprom_read_buffer( int deviceAddress, unsigned int eeAddress, uint8_t* const buffer, int length ) {
  Wire.beginTransmission( deviceAddress );
  Wire.write( int( eeAddress >> 8 ) ); // MSB
  Wire.write( int( eeAddress & 0xFF ) ); // LSB
  const bool ret_val = ( Wire.endTransmission() == 0 );
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
  Wire.write( int( eeAddress >> 8 ) ); // MSB
  Wire.write( int( eeAddress & 0xFF)); // LSB
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

static time_t getTime() {
  time_t t;
  while ( micros() - tickCounter > 999980UL );
  const uint32_t difference = micros() - tickCounter;
  t.milliSecs = (difference + difference % 1000U)/ 1000U;
  DateTime dt = rtc.now();       // reading clock time
  t.utc = dt.unixtime(); // reading clock time as UTC-time + TimeZone
  return t;
}
