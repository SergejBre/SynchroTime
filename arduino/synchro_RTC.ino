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
//#include <EEPROM.h>
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
unsigned long hexToInt( byte* const buff );
unsigned long getUTCtime( unsigned long localTimeSecs );
void adjustTime( unsigned long utcTimeSecs );
void adjustDrift( int8_t ppm ); // todo
char string_parser();

void setup () {
  Serial.begin( 115200 ); // initialization serial port with 115200 baud (_standard_)

  if ( !rtc.begin() ) {
    Serial.println( F( "Couldn't find DS3231 RTC modul" ) );
    while (1);
  }

  int8_t offset_reg = readFromOffsetReg();
/*
  Serial.print( F("Offset register value = ") );
  Serial.println( offset_reg );
  Serial.println();
*/
  if ( rtc.lostPower() ) {
    Serial.println( F("RTC lost power, lets set the time!") );
    // If the RTC have lost power it will sets the RTC to the date & time this sketch was compiled in the following line
    rtc.adjust( DateTime( F(__DATE__), F(__TIME__) ) + TimeSpan( 0, 0, 0, 15 ) );

//    offset_reg = -32; // from -127 to +127, default 0
    offset_reg = (int8_t) i2c_eeprom_read_byte( EEPROM_ADDRESS, 4U );
    writeToOffsetReg( offset_reg );
    Serial.print( F( "Set Offset Reg: " ) );
    Serial.println( offset_reg );
  }
/*
  intToHex( buff, 123456789 ); // data to write
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
  unsigned long intValue = 0U;
  unsigned long utc_time = 0U;

  if ( Serial.available() ) {  // if there is data available

    DateTime now = rtc.now();

    while ( Serial.available() && i < 31 ) {
      char thisChar = Serial.read();      // read the first byte
      if ( thisChar == '@' && Serial.available() ) {
        thisChar = Serial.read();      // read the first request for..
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
        switch ( task )
        {
        case 'a': // adjust time
          Serial.readBytes( buff, 4 ); // reading new time
          utc_time = hexToInt( buff );
          adjustTime( utc_time );
          Serial.print( "successful" );
          task = 'n';
          break;
        case 'i': // status information
          Serial.print( now.unixtime(), DEC ); // todo
          task = 'n';
          break;
        case 's': // set time
          Serial.readBytes( buff, 4 ); // reading new time
          utc_time = hexToInt( buff );
          adjustTime( utc_time );
          Serial.print( "successful" );
          task = 'n';
          break;
        case 'v': // get version
          utc_time = getUTCtime( now.unixtime() );
          intToHex( buff, utc_time );
          Serial.write( buff, 4 );  // send UTC time
          task = 'n';
          break;
        case 'n': // idle task
          break;
        default:
          Serial.println( "unknown task " + task );
          task = 'n';
        }
      }
      if ( isDigit(thisChar) ) {
        intValue = intValue * 10 + thisChar - '0';
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
  int8_t offset_reg;
  Wire.requestFrom( DS3231_ADDRESS, 1 ); // Read a byte from register
  offset_reg = int8_t( Wire.read() );
  return offset_reg;
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
  i2c_eeprom_write_page( EEPROM_ADDRESS, 0U, buff, sizeof(buff)); // write to EEPROM AT24C256
}

byte i2c_eeprom_read_byte( int deviceaddress, unsigned int eeaddress ) {
  byte rdata = 0xFF;
  Wire.beginTransmission( deviceaddress );
  Wire.write( (int)( eeaddress >> 8 ) ); // MSB
  Wire.write( (int)( eeaddress & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom( deviceaddress, 1 );
  if ( Wire.available() ) rdata = Wire.read();
  return rdata;
}

void i2c_eeprom_read_buffer( int deviceaddress, unsigned int eeaddress, byte* const buffer, int length ) {
  Wire.beginTransmission( deviceaddress );
  Wire.write( (int)( eeaddress >> 8 ) ); // MSB
  Wire.write( (int)( eeaddress & 0xFF ) ); // LSB
  Wire.endTransmission();
  Wire.requestFrom( deviceaddress, length );
  int i;
  for ( i = 0; i < length; i++ ) {
    if ( Wire.available() ) {
      buffer[i] = Wire.read();
    }
  }
}

boolean i2c_eeprom_write_byte( int deviceaddress, unsigned int eeaddress, byte data ) {
  int rdata = data;
  Wire.beginTransmission( deviceaddress );
  Wire.write( (int)( eeaddress >> 8 ) ); // MSB
  Wire.write( (int)( eeaddress & 0xFF)); // LSB
  Wire.write( rdata );
  return ( Wire.endTransmission() == 0 );
}

// WARNING: address is a page address, 6-bit end will wrap around
// also, data can be maximum of about 30 bytes, because the Wire library has a buffer of 32 bytes
boolean i2c_eeprom_write_page( int deviceaddress, unsigned int eeaddresspage, byte* const data, uint8_t length ) {
  Wire.beginTransmission( deviceaddress );
  Wire.write( (int)( eeaddresspage >> 8 ) ); // MSB
  Wire.write( (int)( eeaddresspage & 0xFF ) ); // LSB
  uint8_t i;
  for ( i = 0; i < length; i++ ) {
    Wire.write( data[i] );
  }
  return ( Wire.endTransmission() == 0 );
}
