# ArduinoSerialLib

C++ library for connecting to Arduino based boards, giving a computer app access and control of the board via the USB port via serial connection.

Designed from the start to work with Windows and Mac machines, tested on Windows 10 and macOS 10.14. No tests on Linux machines, though.

Inspired and based on code on from the following links:

https://github.com/killpl/obd_cougar/tree/master/cougar_lib/serial

https://stackoverflow.com/questions/2674048/what-is-proper-way-to-detect-availabel-serial-ports-on-windows

https://github.com/todbot/arduino-serial

https://www.xanthium.in/Serial-Port-Programming-using-Win32-API

# Installation

No need for any complicated install procedure. Just add the following files (from the /src folder) to your project:

- serial_devices.cpp
- serial_devices.h
- serial_port.cpp
- serial_port.h

On you project main .cpp file, just add:

    #include "serial_devices.h"
    #include "serial_port.h"

That's it! All the default SDK libraries that are installed with VisualStudio (on Windows) and XCode (on Mac) should be enough to make the code work.

Note: On XCode, make sure that in the Project Properties page, under the "General" tab and "Linked Frameworks and Libraries" section, you have the following frameworks added:

- CoreFoundation.framework
- IOKit.framework

Under the "Build phase" tab, section "Link Binary with libraries", make sure that these two frameworks are also added.

# Testing

In the /src folder you will also find three files:

- test_serial_io.cpp
- test_serial_monitor.cpp
- serial_write_test.ino

test_serial_io.cpp is an example file for the use of the read and write functions of the library.

test_serial_monitor.cpp is an example for continously using the reading functions of the library.

serial_write_test.ino is an Arduino sketch to the used together with test_serial_io.cpp.

Refer to the comment section at the top of each file for more information.
