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

#define TIME_ZONE 2           // Difference to UTC-time on the work computer, from { -12, .., -2, -1, 0, +1, +2, +3, .., +12 }
#define OFFSET_REGISTER 0x10  // Aging offset register address
#define CONRTOL_REGISTER 0x0E // Control Register address
#define EEPROM_ADDRESS 0x57   // AT24C256 address (256 kbit = 32 kbyte serial EEPROM)
typedef enum task : uint8_t {TASK_IDLE = 0x0, TASK_ADJUST, TASK_INFO, TASK_CALIBR, TASK_RESET} task_t;

RTC_DS3231 rtc;
uint8_t buff[4];
uint8_t byteBuffer[11];

// Function Prototypes
int8_t readFromOffsetReg( void ); // read from offset register
bool writeToOffsetReg( const int8_t value ); // write to offset register
bool setControlReg1Hz( void ); // Control Register to 1 Hz Out (SQW-pin)
inline void intToHex( uint8_t* const buff, const uint32_t value );
inline void floatToHex( uint8_t* const buff, const float value );
uint32_t hexToInt( uint8_t* const buff );
uint32_t getUTCtime( const uint32_t localTimeSecs );
bool adjustTime( const uint32_t utcTimeSecs );
bool adjustTimeDrift( float drift_in_ppm );
float calculateDrift_ppm( uint32_t referenceTimeSecs, uint16_t referenceTimeMs, uint32_t clockTimeSecs, uint16_t clockTimeMs );
void printOk( const bool ok );

void setup () {
  Serial.begin( 115200 ); // initialization serial port with 115200 baud (_standard_)
  while ( !Serial );      // wait for serial port to connect. Needed for native USB
  Serial.setTimeout( 5 ); // timeout 5ms

  if ( !rtc.begin() ) {
    Serial.println( F( "Couldn't find DS3231 RTC modul" ) );
    Serial.flush();
    abort();
  }

  Ds3231SqwPinMode mode = rtc.readSqwPinMode();
  if ( mode != DS3231_SquareWave1Hz ) {
    rtc.writeSqwPinMode( DS3231_SquareWave1Hz );
  }

  if ( rtc.lostPower() ) {
    Serial.println( F("RTC lost power, lets set the time!") );
    // If the RTC have lost power it will sets the RTC to the date & time this sketch was compiled in the following line
    rtc.adjust( DateTime( F(__DATE__), F(__TIME__) ) + TimeSpan( 0, 0, 0, 15 ) );

    // offset value from -128 to +127, default is 0
    int8_t offset_val = (int8_t) i2c_eeprom_read_byte( EEPROM_ADDRESS, 4U );
    writeToOffsetReg( offset_val );
    Serial.print( F( "Set Offset Reg: " ) );
    Serial.println( offset_val );
  }
/* August 5, 2020 at 12:00 you would call:
  Serial.println( DateTime(2020, 7, 13, 18, 0, 0).unixtime(), DEC );
  intToHex( buff, DateTime(2020, 7, 13, 18, 0, 0).unixtime() ); // data to write
  i2c_eeprom_write_page( EEPROM_ADDRESS, 0U, buff, sizeof(buff)); // write to EEPROM AT24C256
  delay(100); //add a small delay
  Serial.println( F("Memory written") );

  i2c_eeprom_read_buffer( EEPROM_ADDRESS, 0U, buff, sizeof(buff) );
  uint32_t value = hexToInt( buff );
  Serial.println( value, DEC );
*/
  attachInterrupt( 0, oneHertz, FALLING );
}

//extern volatile unsigned long timer0_millis;
volatile uint32_t tickCounter;

void oneHertz( void ) {
//  timer0_millis = 0;
  tickCounter = millis();
}

void loop () {
  task_t task = TASK_IDLE;
  bool ok;
  uint8_t numberOfBytes = 0;
  uint16_t utc_milliSecs = 0U;
  uint16_t ref_milliSecs = 0U;
  uint32_t utc_time = 0UL;
  uint32_t ref_time = 0UL;
  float drift_in_ppm = 0;

  if ( Serial.available() ) {  // if there is data available

//    while( timer0_millis > 998 );
//    utc_milliSecs = timer0_millis;
    while( millis() - tickCounter > 998 );
    utc_milliSecs = millis() - tickCounter;
    DateTime now = rtc.now(); // read clock time + ms

    char thisChar = Serial.read();  // read the first byte of command
    if ( thisChar == '@' && Serial.available() ) {
      thisChar = Serial.read();     // read the request for..
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
      default:                      // unknown request
        task = TASK_IDLE;
        Serial.print( F("unknown request ") );
        Serial.println( thisChar );
      }

      numberOfBytes = Serial.readBytes( byteBuffer, 6 ); // reading reference time + ms
      if ( numberOfBytes >= 4 ) ref_time = hexToInt( byteBuffer );
      if ( numberOfBytes >= 6 ) {
        uint16_t *y = (uint16_t *)( byteBuffer + 4 );
        ref_milliSecs = y[0];
      }

      switch ( task )
      {
        case TASK_ADJUST:               // adjust time
          ok = adjustTime( ref_time );
          printOk( ok );
          task = TASK_IDLE;
          break;
        case TASK_INFO:                 // information
          utc_time = getUTCtime( now.unixtime() ); // reading clock time as UTC-time
          intToHex( byteBuffer, utc_time );
          memcpy( byteBuffer + 4, &utc_milliSecs, sizeof(utc_milliSecs) );  // write ms
          byteBuffer[6] = readFromOffsetReg();  // reading offset value
          drift_in_ppm = calculateDrift_ppm( ref_time, ref_milliSecs, utc_time, utc_milliSecs );  // calculate drift time
          floatToHex( byteBuffer + 7, drift_in_ppm );
          Serial.write( byteBuffer, 11 );  // send data
          task = TASK_IDLE;
          break;
        case TASK_CALIBR:               // calibrating
/*            utc_time = getUTCtime( now.unixtime() ); // reading clock time as UTC-time
          intToHex( byteBuffer, utc_time );
          memcpy( byteBuffer, &utc_milliSecs, sizeof(utc_milliSecs) );  // write bytes
          drift_in_ppm = calculateDrift_ppm( ref_time, ref_milliSecs, utc_time, utc_milliSecs );  // reading drift time
          ok = adjustTimeDrift( drift_in_ppm );
          if ( ok ) {
            ok &= adjustTime( ref_time ); // adjust time
          }
*/          ok = true;
          printOk( ok );
          task = TASK_IDLE;
          break;
        case TASK_RESET:                // reset
          ok = writeToOffsetReg( 0 );
          if ( ok ) {
            uint8_t buff5b[5];
            for (numberOfBytes=0; numberOfBytes<5; numberOfBytes++ ) buff5b[numberOfBytes] = 0xFF;
            ok &= i2c_eeprom_write_page( EEPROM_ADDRESS, 0U, buff5b, sizeof( buff5b ) );
          }
          printOk( ok );
          task = TASK_IDLE;
          break;
        case TASK_IDLE:                 // idle task
          break;
        default:
          Serial.print( F("unknown task ") );
          Serial.println( task, HEX );
          task = TASK_IDLE;
      }
    }
  }
}

int8_t readFromOffsetReg( void ) {
  Wire.beginTransmission( DS3231_ADDRESS ); // Sets the DS3231 RTC module address
  Wire.write( uint8_t( OFFSET_REGISTER ) ); // sets the offset register address
  Wire.endTransmission();
  int8_t offset_val = 0x0;
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

bool setControlReg1Hz( void ) {
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

uint32_t hexToInt( uint8_t* const buff ) {
  uint32_t *y = (uint32_t *)buff;
  return y[0];
}

uint32_t getUTCtime( const uint32_t localTimeSecs ) {
  return ( localTimeSecs - TIME_ZONE*3600 ); // UTC_time = local_Time - TIME_ZONE*3600 sec
}

bool adjustTime( const uint32_t utcTimeSecs ) {
  rtc.adjust( DateTime( utcTimeSecs + TIME_ZONE*3600 ) );
  intToHex( buff, utcTimeSecs ); // data to write
  return i2c_eeprom_write_page( EEPROM_ADDRESS, 0U, buff, sizeof(buff)); // write last_set_time to EEPROM AT24C256
}

// the result is rounded to the maximum possible values of type uint8_t
bool adjustTimeDrift( float drift_in_ppm ) {
  drift_in_ppm *= 10;
  int8_t offset = (drift_in_ppm > 0) ? int8_t( drift_in_ppm + 0.5 ) : int8_t( drift_in_ppm - 0.5 );
  if ( offset == 0 ) return true;  // if offset is 0, nothing needs to be done
  int8_t last_offset_reg = readFromOffsetReg();
  int8_t last_offset_ee = i2c_eeprom_read_byte( EEPROM_ADDRESS, 4U );
  if ( last_offset_reg == last_offset_ee ) {
    drift_in_ppm += last_offset_reg;
    offset = (drift_in_ppm > 0) ? int8_t( drift_in_ppm + 0.5 ) : int8_t( drift_in_ppm - 0.5 );
  }
  i2c_eeprom_write_byte( EEPROM_ADDRESS, 4U, offset );  // write offset value to EEPROM AT24C256
  return writeToOffsetReg( offset );
}

/*
 * "drift in ppm unit" - this is the ratio of the clock drift from the reference time,
 * which is expressed in terms of one million control seconds.
 * For example, reference_time = 1597590292 sec, clock_time = 1597590276 sec, last_set_time = 1596628800 sec,
 * time_drift = clock_time - reference_time = -16 sec
 * number_of_control_seconds = reference_time - last_set_time = 961492 sec, i.e 0.961492*10^6 sec
 * drift_in_ppm = time_drift * 10^6 / number_of_control_seconds = -16*10^6 /(0.961492*10^6) = -16.64 ppm
 */
float calculateDrift_ppm( uint32_t referenceTimeSecs, uint16_t referenceTimeMs, uint32_t clockTimeSecs, uint16_t clockTimeMs ) {
  if ( !i2c_eeprom_read_buffer( EEPROM_ADDRESS, 0U, buff, sizeof(buff)) ) {
    return 0;
  }
  uint32_t last_set_timeSecs = hexToInt( buff );
  int32_t diff = referenceTimeSecs - last_set_timeSecs;
  if ( diff < 10000 ) {
    return 0;
  }
  int32_t time_driftSecs = clockTimeSecs - referenceTimeSecs;
  int16_t time_driftMs = clockTimeMs - referenceTimeMs;
  float time_drift = time_driftSecs*1000 + time_driftMs;
  return time_drift * 1000 / diff;
}

void printOk( const bool ok ) {
  if ( ok ) {
    Serial.print( F("successful") );
  }
  else {
    Serial.print( F("fail") );
  }  
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
 * WARNING: address is a page address, 6-bit end will wrap around
 * also, data can be maximum of about 30 bytes, because the Wire library has a buffer of 32 bytes
 */
bool i2c_eeprom_write_page( int deviceAddress, unsigned int eeAddressPage, uint8_t* const data, uint8_t length ) {
  Wire.beginTransmission( deviceAddress );
  Wire.write( (int)( eeAddressPage >> 8 ) ); // MSB
  Wire.write( (int)( eeAddressPage & 0xFF ) ); // LSB
  uint8_t i;
  for ( i = 0; i < length; i++ ) {
    Wire.write( data[i] );
  }
  return ( Wire.endTransmission() == 0 );
}
