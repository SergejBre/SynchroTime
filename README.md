# SynchroTime - Console client for setting the time and calibrating the RTC DS3231 module

## Motivation
The real-time clock module on the [DS3231](https://create.arduino.cc/projecthub/MisterBotBreak/how-to-use-a-real-time-clock-module-ds3231-bc90fe) chip has proven itself well in work with microcontrollers Arduino, Raspberry Pi, etc. According to the declared specification, it has thermal correction, so that the clock drift is within ±2 ppm (about 1 minute per year). But a large number of modules on the market do not meet the accuracy declared by the manufacturer, which is undoubtedly upsetting. Nevertheless, the manufacturer has provided for the possibility of correcting the clock drift, which is associated with the aging of the oscillator crystal in the range from -12.8 to +12.7 ppm. This correction value can be written to one of the registers on the DS3231 ([datasheet](https://datasheets.maximintegrated.com/en/ds/DS3231.pdf)). In addition, the manufacturer has provided a non-volatile memory in the module, into which calibration parameters and correction factors can be placed. The tool below can automatically calibrate the DS3231 module.

## About the app

* Console application is used for fine tuning and calibration of the [RTC DS3231](https://create.arduino.cc/projecthub/MisterBotBreak/how-to-use-a-real-time-clock-module-ds3231-bc90fe) module.

* The application allows you to:
  * Synchronize RTC DS3231 with computer time;
  * Correct the DS3231 RTC clock drift. The algorithm performs correction in the range from -12.8 to +12.7 ppm;
  * Automatically save parameters and calibration data to the energy-independent flash memory of the type AT24C256. In case there is a power failure to the module.

* The client communicates with the Arduino server via the serial interface (UART). The Arduino is in turn connected to the Precision RTC DS3231 module via the I²C-interface. The application allows you to easily select a port for communication with the server and save the port number in the program settings. (The Baud Rate is assumed unchanged and equals 115200).

* Command Help 
`
~$ ./synchroTime -h
`

![synchroTime -h](images/consoleApp_About.png)

## Using the app

1. Connect your Arduino to your computer via a free USB port. If there is a necessary driver in the system, a virtual device - Serial port will appear in the system (under Linux it will be /dev/ttyUSBx, under Windows - COMx).
 To find a new port, you can view the entire list of ports in the system with the discovery command. To do this, call the application with the -d (--discovery) switch:
```
~$ ./synchroTime -d 
```
![synchroTime -d](images/consoleApp_Discovery.png)
 
2. To select a virtual Serial Port, enter its system name after the command -p 'portName'. The app will automatically create a configuration file, and the next call will contact the selected port.
```
~$ ./synchroTime -p ttyUSB0 
```
![synchroTime -p](images/consoleApp_SetPort.png)

3.

## Specification

see [datasheet](https://datasheets.maximintegrated.com/en/ds/DS3231.pdf) page 13.

![circuit](images/Steckplatine_DS3231.png)

## System requirements

## Installing the app

## TODO
