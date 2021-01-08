# SynchroTime - CLI- and GUI-client for adjust the exact time and calibrating the RTC DS3231 module
![PROJECT_IMAGE](./images/guiApp_About.png)

## Motivation

The real-time clock module on the [DS3231](https://create.arduino.cc/projecthub/MisterBotBreak/how-to-use-a-real-time-clock-module-ds3231-bc90fe) chip has proven itself well in work with microcontrollers Arduino, Raspberry Pi, etc. According to the declared specification, it has thermal correction, so that the drift of the clock time is within ±2 ppm (ca. 1 minute per year). But a large number of modules on the market do not meet the accuracy declared by the manufacturer, which is undoubtedly upsetting. Nevertheless, the manufacturer has provided for the possibility of correcting the drift of the clock time, which is associated with the aging of the oscillator crystal in the range from -12.8 to +12.7 ppm. This correction value can be written to one of the registers on the DS3231 (See the [datasheet](https://datasheets.maximintegrated.com/en/ds/DS3231.pdf) for exact ppm values). In addition, the manufacturer has provided a the energy-independent flash memory AT24C256 in the module, into which calibration parameters and correction factors can be placed. The tool below can automatically calibrate the DS3231 module.

## About the app

* CLI and GUI applications are used for fine adjust and calibrating the DS3231 RTC module.

* The application allows you to:
  * Synchronize the time of the RTC DS3231 with your computer time;
  * Correct the time drift of the DS3231-RTC clock. The algorithm performs correction in the range from -12.8 to +12.7 ppm;
  * The application allows you to evaluate the accuracy and reliability of the RTC oscillator for a particular sample, as well as the chances of successful correction in case of significant time drift;
  * Automatically save parameters and calibration data to the energy-independent flash memory of the type AT24C256. In case there is a power failure to the module.

* Developed in pure Qt, no third party libraries.

* Cross-platform application implementation (Linux and Windows).

* The client communicates with the Arduino server via the serial interface (UART). The application allows you to easily select a serial port for communication with the server and save the port number in the program settings.

* Command Help `$ ./synchroTime -h`

![synchroTime -h](./images/consoleApp_About.png)

## Using the CLI app

1. First, you need to load a sketch into Arduino from the [arduino/synchro_RTC.ino](arduino/synchro_RTC.ino) project directory and connect the RTC DS3231 module according to the circuit shown in the specification.
 Connect your Arduino to your computer via a free USB port. If there is a necessary driver in the system, a new virtual serial port will appear in the system (under Linux it will be /dev/ttyUSBx, under Windows - COMx).
 To find the name of this port, call the application with the -d (--discovery) switch:
```
 $ ./synchroTime -d
 Serial Port : ttyUSB1
 Description : USB2.0-Serial
 Manufacturer: 1a86
 Vendor ID   : 1a86
 Product ID  : 7523
 System Locat: /dev/ttyUSB1
 Busy        : No
 
 Serial Port : ttyUSB0
 Description : USB2.0-Serial
 Manufacturer: 1a86
 Vendor ID   : 1a86
 Product ID  : 7523
 System Locat: /dev/ttyUSB0
 Busy        : No
 
 A total of 2 serial ports were found. 
```
 And under the Windows OS
```
 C:\SynchroTime\build>synchroTime -d
 Serial Port : COM5
 Description : USB-SERIAL CH340
 Manufacturer: wch.cn
 Vendor ID   : 1a86
 Product ID  : 7523
 System Locat: \\.\COM5
 Busy        : No

 Serial Port : COM3
 Description : Agere Systems HDA Modem
 Manufacturer: Agere
 Vendor ID   : 11c1
 Product ID  : 1040
 System Locat: \\.\COM3
 Busy        : No

 A total of 2 serial ports were found.
``` 

2. To select a virtual Serial Port, enter its system name after the command -p \<portName\>. The app will automatically create a configuration file, and the next call will contact the selected port.
```
 $ ./synchroTime -p ttyUSB0
 Added new serial interface ttyUSB0. 
```
 And under the Windows OS
```
 C:\SynchroTime\build>synchroTime -p COM5
 Added new serial interface COM5.
``` 

3. Use the -i (--information) command to get the current information from the DS3231 module. If everything is connected correctly, then you will get the current time of both clocks, the difference between the clocks in milliseconds (with an accuracy of ±2 ms), the value written in the offset register and the calculated time drift value in ppm. If the offset register and time drift are zero, then the DS3231 has not yet been calibrated (see step 5.)
```
 $ ./synchroTime -i
 DS3231 clock time	1598896552596 ms: 31.08.2020 19:55:52.596
 System local time	1598896589772 ms: 31.08.2020 19:56:29.772
 Difference between	-37176 ms
 Offset reg. in ppm	0 ppm
 Time drift in ppm	-8.78162 ppm
 last adjust of time	1594663200000 ms: 13.07.2020 20:00:00.000 
```

4. To set the exact time, use the -a (--adjust) command. The module clock will be synchronized with the computer time with an accuracy of ±1 ms. After updating the time, the date of the time setting will be recorded in the module's memory, which will allow later to determine the exact drift of the clock time.
```
 $ ./synchroTime -a
 System local time	Mo. 31 Aug. 2020 20:02:52.000
 Request for adjustment completed successfully. 
```

5. To calibrate the clock of the DS3231 module, enter the -c (--calibration) command. For the successful execution of this procedure, the module must be activated (see point 4.) and it is necessary that enough time has passed so that the calculated value of the clock drift is well distinguishable from the rounding error (ca 55 hours or 2.3 days, see part **Discussion**). The algorithm of the program will calculate the amount of drift of the clock time and the correction factor, which will be written into the offset register. The clock time will also be updated. If the calibration is successful, the current time, drift and correction factor will be displayed, as in the screenshot.
```
 $ ./synchroTime -c
 System local time	Mo. 31 Aug. 2020 20:04:14.000
 Offset last value	0
 Time drift in ppm	-2.11938 ppm
 Offset new value	-21
 Request for calibration completed successfully. 
```

6. To reset the offset register to its default value and clear the module's memory of calibration data, enter the -r (--reset) command. The default value will be written to the register, and memory cells will be overwritten with bytes with 0xFF.
```
 $ ./synchroTime -r

 Request for reset completed successfully. 
```

7. Use the -s (--setreg) command to write the new value to the offset register of the DS3231. Warning: it makes sense to do this operation only in case of resetting all calibration data (see step 6).
```
 $ ./synchroTime -s

 Request for SetRegister completed successfully. 
```

## Using the GUI app

All functionality is similar to the CLI application (see figure below). As an extra, there is the option of selecting the numerous serial port settings.

![PROJECT_IMAGE](./images/guiApp_About2.png)

## Specification

* The application allows you to adjust the time with an accuracy of ±1 ms.

* The application allows you to control the time difference between the DS3231 module and the computer with an accuracy of ±2 ms.

* The application allows you to calibrate the module clock within the range from -12.8 to +12.7 ppm.

* The application communicates with the Arduino server through any virtual serial interface (UART). The Baud Rate is assumed unchanged and equals 115200 bps.

* The Arduino is in turn connected to the Precision RTC DS3231 module via the I²C-interface: A4 (SDA), A5 (SCL) pins (SDA - data line and SCL - clock line).

* The interrupt on the port D2 (or D3) serves to count milliseconds by the internal Arduino counter millis.

* The suggested connection to the DS3231 module is according to the Circuit below.
![circuit](images/Steckplatine_DS3231.png)

## Description of the request protocol

The client is the computer. The client is always the first to send a request. Upon receipt of each message, the microprocessor must send back the appropriate response.

Each request is as follows:
```
@ req <local time>, <value> [CRC]
``` 
where:

* `@` - mandatory start byte, sign for the beginning of the transfer (always equal to `0x40`),
* `req` - request from the set `{a, c, i, r, s, t}` (1 byte),
* `local time` - local computer time only for requests `a, c, i` (6 bytes),
* `value` - new value for the offset register only for request `s` (4 bytes),
* `CRC` - checksum (1 byte). The checksum is calculated as the sum of all bytes, starting from the first byte of the request and ending with the last byte of data,
* `a` - time adjustment request,
* `c` - calibrating request,
* `i` - information request,
* `s` - set offset register request,
* `r` - reset request,
* `t` - status request.

| Request Name        |  ID  | Data           | Expected Response |
|---------------------|------|----------------|-------------------|
| Time adjustment     | `@a` | `<local time>` | yes               |
| Calibrating         | `@c` | `<local time>` | yes               |
| Information         | `@i` | `<local time>` | yes               |
| Set offset Register | `@s` | `<value>`      | yes               |
| Reset               | `@r` |     ---        | yes               |
| Status              | `@s` |     ---        | yes               |

## Recommended System Requirements

* For correct work your system time required to be synchronized with NTP. Only in this case the program will work according to the declared specifications. Under Linux, the ntp service is installed by the following command
```
 $ sudo apt-get install ntp 
```

* Check the correct operation of the service ntp by running the command
```
$ ntpq -p
     remote           refid      st t when poll reach   delay   offset  jitter
==============================================================================
+gromit.nocabal. 131.188.3.222    2 u   64   64  377   27.218   -3.906   5.643
*www.kashra.com  .DCFa.           1 u    3   64  377   42.583   -5.584   4.940
+ext01.epiontis. 130.149.17.8     2 u    2   64  177   18.668   -7.450   5.801
+ntp1.hetzner.de 124.216.164.14   2 u   11   64  377   25.011   -5.987   6.489
+chilipepper.can 134.71.66.21     2 u   74   64  376   26.689   -5.881   4.974 
```

* Look for a table entry `*`: table values offset and jitter (ms), they should be as minimal as possible `max[offset ± jitter] <= 10ms`. If this is not the case, adjust the configuration file `/etc/ntp.conf` in which you enter the local time servers.

* Windows OS has its own specifics. Windows `W32tm` Time Service synchronizes time once a week, which is not enough for fine tuning and calibration. Therefore, it is necessary to adjust the computer time manually before setting up and calibrating, or using the subtleties of the settings in the system registry.

## Installing the CLI and GUI apps

* According to the working platform, download the appropriate archive with the command-line application from the [releases page](https://github.com/SergejBre/SynchroTime/releases).

* Unpack it to your home directory with write access, as the application retains its settings.
```
 $ tar -x -j -f SynchroTime_x64_linux_1.0.0-beta.tar.bz2
``` 

* Run the application according to the instructions in the section **Using the CLI or GUI app**.
```
 $ cd SynchroTime

 SynchroTime$ ./synchroTime -h
``` 

## Discussion

## Dependencies

| Name         | Version                          | Comment                                         |
|--------------|----------------------------------|-------------------------------------------------|
| Qt lib 32bit | >= 5.5.1                         | Didn't test with older versions, but it may work|
| Qt lib 64bit | >= 5.6                           | Didn't test with older versions, but it may work|
| C++ compiler | supporting C++11 (i.e. gcc 4.6+) |                                                 |
| Arduino IDE  | >= 1.8.13                        | !Replace compilation flags from -Os to -O2      |

```
$ ldd synchroTime
	libQt5SerialPort.so.5 => ./lib/libQt5SerialPort.so.5
	libQt5Core.so.5 => ./lib/libQt5Core.so.5
	...
	libicui18n.so.54 => ./lib/libicui18n.so.54
	libicuuc.so.54 => ./lib/libicuuc.so.54
	libicudata.so.54 => ./lib/libicudata.so.54
``` 

## Compilation on Linux

* `sudo apt-get install build-essential qt5-default qt5-qmake gdb git`

* `git clone https://github.com/SergejBre/SynchroTime.git`

* `cd ./SynchroTime`

* `QT_SELECT=5 qmake SynchroTime.pro`

* `make && make clean`

## License

SynchroTime is licensed under [MIT](LICENSE).
