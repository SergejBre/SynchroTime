# SynchroTime - Console client for setting the time and calibrating the RTC DS3231 module

## About
* Console application is used for fine tuning and calibration of the RTC DS3231 module.

* And also for saving the parameters and calibration data in the energy-independent flash memory of the AT24C256 chip.

* The client communicates with the Precision RTC DS3231 module via the serial interface (Serial Port).

* The application allows you to:
 * Synchronize RTC DS3231 with computer time;
 * Correct the DS3231 RTC clock drift. The algorithm performs correction in the range from -12.8 to +12.7 ppm;
 * Automatically save parameters and calibration data to energy independent memory. In case there is a power failure to the module.

