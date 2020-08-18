/* 
 * The time synchronization via Serial Port with a DS3231 Precision RTC module.
 * - read from offset register,
 * - write to offset register,
 * - etc.
 * Einstellungen sind: Die Zeitzone, die als lokale Ortszeit auf dem Arbeitscomputer bestimmt ist.
 * time_zone = Differenz UTC-time, from { -12, .., -2, -1, 0, +1, +2, +3, .., +12 } +1/+2 für Europa, je nachdem, 
 * welche Jahreszeit Winter- (+1) oder Sommerzeit (+2) ist.
 */
#include <Wire.h>
#include "RTClib.h"

#define TIME_ZONE 2          // Difference to UTC-time on the work computer, from { -12, .., -2, -1, 0, +1, +2, +3, .., +12 }
#define OFFSET_REGISTER 0x10
#define EEPROM_ADDRESS 0x57  // AT24C256 (256 kbit = 32 kbyte serial EEPROM)
RTC_DS3231 rtc;
byte buff[4];

// Function Prototypes
int8_t readFromOffsetReg( void ); // read from offset register
boolean writeToOffsetReg( int8_t value ); // write to offset register
inline void intToHex( byte* const buff, unsigned long value );
inline void floatToHex( byte* const buff, float value );
unsigned long hexToInt( byte* const buff );
unsigned long getUTCtime( unsigned long localTimeSecs );
void adjustTime( unsigned long utcTimeSecs );
boolean adjustTimeDrift( float drift_in_ppm );
float calculateDrift_ppm( unsigned long referenceTimeSecs, unsigned long clockTimeSecs );

void setup () {
  Serial.begin( 115200 ); // initialization serial port with 115200 baud (_standard_)
  Serial.setTimeout( 5 ); // timeout 5ms

  if ( !rtc.begin() ) {
    Serial.println( F( "Couldn't find DS3231 RTC modul" ) );
    while (1);
  }

/*
  int8_t offset_val = readFromOffsetReg();
  Serial.print( F("Offset register value = ") );
  Serial.println( offset_val );
  Serial.println();
*/
  if ( rtc.lostPower() ) {
    Serial.println( F("RTC lost power, lets set the time!") );
    // If the RTC have lost power it will sets the RTC to the date & time this sketch was compiled in the following line
    rtc.adjust( DateTime( F(__DATE__), F(__TIME__) ) + TimeSpan( 0, 0, 0, 15 ) );

//    offset_val = -32; // from -128 to +127, default 0
    int8_t offset_val = (int8_t) i2c_eeprom_read_byte( EEPROM_ADDRESS, 4U );
    writeToOffsetReg( offset_val );
    Serial.print( F( "Set Offset Reg: " ) );
    Serial.println( offset_val );
  }
// August 5, 2020 at 12:00 you would call:
/*  Serial.println( DateTime(2020, 8, 5, 12, 0, 0).unixtime(), DEC );
  intToHex( buff, DateTime(2020, 8, 5, 12, 0, 0).unixtime() ); // data to write
  i2c_eeprom_write_page( EEPROM_ADDRESS, 0U, buff, sizeof(buff)); // write to EEPROM AT24C256
  delay(100); //add a small delay
  Serial.println( F("Memory written") );

  i2c_eeprom_read_buffer( EEPROM_ADDRESS, 0U, buff, sizeof(buff) );
  unsigned long value = hexToInt( buff );
  Serial.println( value, DEC );
*/
}

void loop () {
  char task = 'n';
  uint8_t i = 0;
  int8_t offset_val = 0x0;
  float drift_in_ppm = 0;
  unsigned long utc_time = 0U;
  unsigned long ref_time = 0U;

  if ( Serial.available() ) {  // if there is data available

    DateTime now = rtc.now();

    while ( Serial.available() && i < 32 ) {
      char thisChar = Serial.read();      // read the first byte of command
      if ( thisChar == '@' && Serial.available() ) {
        thisChar = Serial.read();      // read the request for..
        switch ( thisChar )
        {
        case 'a': // time adjustment request
          task = thisChar;
          break;
        case 'i': // status information request
          task = thisChar;
          break;
        case 's': // time setting request
          task = thisChar;
          break;
        case 'v': // version request
          task = thisChar;
          break;
        default:  // unknown request
          task = 'n';
          Serial.println( "unknown request " + thisChar );
        }

        Serial.readBytes( buff, 4 ); // reading reference time
        ref_time = hexToInt( buff );

        switch ( task )
        {
        case 'a': // adjust time
          adjustTime( ref_time );
          Serial.print( "successful" );
          task = 'n';
          break;
        case 'i': // status information
          Serial.print( now.unixtime(), DEC ); // todo
          task = 'n';
          break;
        case 's': // set time
          adjustTime( ref_time );
          Serial.print( "successful" );
          task = 'n';
          break;
        case 'v': // get version
          utc_time = getUTCtime( now.unixtime() ); // reading clock time
          intToHex( buff, utc_time );
          Serial.write( buff, 4 );  // send UTC time
          offset_val = readFromOffsetReg();
          Serial.write( offset_val );  // send offset value
          drift_in_ppm = calculateDrift_ppm( ref_time, utc_time );
          floatToHex( buff, drift_in_ppm );
          Serial.write( buff, 4 );  // send drift time
          task = 'n';
          break;
        case 'n': // idle task
          break;
        default:
          Serial.println( "unknown task " + task );
          task = 'n';
        }
      }
      i++;
    }
  }
  delay( 5 );
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

boolean writeToOffsetReg( int8_t value ) {
  Wire.beginTransmission( DS3231_ADDRESS ); // Sets the DS3231 RTC module address
  Wire.write( uint8_t( OFFSET_REGISTER ) ); // sets the offset register address
  Wire.write( uint8_t( value ) ); // Write value to register
  return ( Wire.endTransmission() == 0 );
}

inline void intToHex( byte* const buff, unsigned long value ) {
  memcpy( buff, &value, sizeof(value) );
}

inline void floatToHex( byte* const buff, float value ) {
  memcpy( buff, &value, sizeof(value) );
}

unsigned long hexToInt( byte* const buff ) {
  unsigned long *y = (unsigned long *)buff;
  return y[0];
}

unsigned long getUTCtime( unsigned long localTimeSecs ) {
  return ( localTimeSecs - TIME_ZONE*3600 ); // UTC_time = local_Time - TIME_ZONE*3600 sec
}

void adjustTime( unsigned long utcTimeSecs ) {
  rtc.adjust( DateTime( utcTimeSecs - SECONDS_FROM_1970_TO_2000 + TIME_ZONE*3600 ) );
  intToHex( buff, utcTimeSecs ); // data to write
  i2c_eeprom_write_page( EEPROM_ADDRESS, 0U, buff, sizeof(buff)); // write last_set_time to EEPROM AT24C256
}

boolean adjustTimeDrift( float drift_in_ppm ) {
  int8_t offset = int8_t( drift_in_ppm * 10 );
  if ( offset == 0 ) return true;  // if offset is 0, nothing needs to be done
  int8_t last_offset_reg = readFromOffsetReg();
  int8_t last_offset_ee = i2c_eeprom_read_byte( EEPROM_ADDRESS, 4U );
  if ( last_offset_reg == last_offset_ee ) {
    offset = int8_t( drift_in_ppm * 10 + last_offset_reg );
  }
  i2c_eeprom_write_byte( EEPROM_ADDRESS, 4U, offset );  // write offset value to EEPROM AT24C256
  return writeToOffsetReg( offset );
}

// "drift in ppm unit" - this is the ratio of the clock drift from the reference time,
// which is expressed in terms of one million control seconds.
// For example, reference_time = 1597590292 sec, clock_time = 1597590276 sec, last_set_time = 1596628800 sec,
// time_drift = clock_time - reference_time = -16 sec
// number_of_control_seconds = reference_time - last_set_time = 961492 sec, i.e 0.961492*10^6 sec
// drift_in_ppm = time_drift * 10^6 / number_of_control_seconds = -16*10^6 /(0.961492*10^6) = -16.64 ppm
float calculateDrift_ppm( unsigned long referenceTimeSecs, unsigned long clockTimeSecs ) {
  if ( !i2c_eeprom_read_buffer( EEPROM_ADDRESS, 0U, buff, sizeof(buff)) ) {
    return 0;
  }
  unsigned long last_set_timeSecs = hexToInt( buff );
  if ( referenceTimeSecs <= last_set_timeSecs ) {
    return 0;
  }
  long time_drift = clockTimeSecs - referenceTimeSecs;
  return float(time_drift)*1000000/(referenceTimeSecs - last_set_timeSecs);
}

byte i2c_eeprom_read_byte( int deviceAddress, unsigned int eeAddress ) {
  byte rdata = 0xFF;
  Wire.beginTransmission( deviceAddress );
  Wire.write( (int)( eeAddress >> 8 ) ); // MSB
  Wire.write( (int)( eeAddress & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom( deviceAddress, 1 );
  if ( Wire.available() ) rdata = Wire.read();
  return rdata;
}

boolean i2c_eeprom_read_buffer( int deviceAddress, unsigned int eeAddress, byte* const buffer, int length ) {
  Wire.beginTransmission( deviceAddress );
  Wire.write( (int)( eeAddress >> 8 ) ); // MSB
  Wire.write( (int)( eeAddress & 0xFF ) ); // LSB
  boolean ret_val = ( Wire.endTransmission() == 0 );
  Wire.requestFrom( deviceAddress, length );
  int i;
  for ( i = 0; i < length; i++ ) {
    if ( Wire.available() ) {
      buffer[i] = Wire.read();
    }
  }
  return ret_val;
}

boolean i2c_eeprom_write_byte( int deviceAddress, unsigned int eeAddress, byte data ) {
  int rdata = data;
  Wire.beginTransmission( deviceAddress );
  Wire.write( (int)( eeAddress >> 8 ) ); // MSB
  Wire.write( (int)( eeAddress & 0xFF)); // LSB
  Wire.write( rdata );
  return ( Wire.endTransmission() == 0 );
}

// WARNING: address is a page address, 6-bit end will wrap around
// also, data can be maximum of about 30 bytes, because the Wire library has a buffer of 32 bytes
boolean i2c_eeprom_write_page( int deviceAddress, unsigned int eeAddressPage, byte* const data, uint8_t length ) {
  Wire.beginTransmission( deviceAddress );
  Wire.write( (int)( eeAddressPage >> 8 ) ); // MSB
  Wire.write( (int)( eeAddressPage & 0xFF ) ); // LSB
  uint8_t i;
  for ( i = 0; i < length; i++ ) {
    Wire.write( data[i] );
  }
  return ( Wire.endTransmission() == 0 );
}
