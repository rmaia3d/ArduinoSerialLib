//
//  serial_devices.h
//  
//  Classes and helper functions for enumerating serial
//  devices connected to a computer
//
//  Based on https://github.com/killpl/obd_cougar/tree/master/cougar_lib/serial
//	And https://stackoverflow.com/questions/2674048/what-is-proper-way-to-detect-availabel-serial-ports-on-windows
//
//  Rodrigo R. M. B. Maia
//  Created 08-Sept-2019
//

#pragma once

#if defined(__APPLE__) && defined(__MACH__)
    #include <CoreFoundation/CoreFoundation.h>
    #include <IOKit/IOKitLib.h>
    #include <IOKit/serial/IOSerialKeys.h>
    #include <IOKit/serial/ioss.h>
    #include <IOKit/IOBSD.h>
#elif defined(_WIN32)
	#include <windows.h>
#endif

#include <string>     
#include <iostream>
#include <vector>

#define USB_DEVICE_ID "IOUSBDevice"
#define BLUETOOTH_DEVICE_ID "IOBluetoothSerialClient"

#define DEVCON_DEBUG 0     // Set to 1 if error output to console is desired

#if defined(__APPLE__)
    #define INTERFACE_CLASS InterfacesOSX
#elif defined(_WIN32)
    #define INTERFACE_CLASS InterfacesWin32
#endif

// In Win32, if UNICODE is set, system functions for port opening
// require wchar based strings
#ifdef UNICODE
	#define PCOUT std::wcout
	#define PSTRING std::wstring
	#define PCHAR wchar_t
	#define TO_STRING std::to_wstring
	#define _S(X) L##X
#else
	#define PCOUT std::cout
	#define PSTRING std::string
	#define PCHAR char
	#define TO_STRING std::to_string
	#define _S(X) X
#endif	

enum class DeviceType
{
    USB_DEVICE,
    BLUETOOTH_DEVICE,
    OTHER
};
    
struct ParentDevice
{
    // COMMON
    std::string name;				//  Device name (BTName, USB Product Name)
    DeviceType type;            //  Device type, also indicates possibly filled fields
                                //  for parent device - USB or Bluetooth.
        
    // BLUETOOTH
    uint32_t channel;           //  Bluetooth channel number
    uint32_t connectionType;    //  Serial device type
        
    // USB
    uint32_t vendorId;          //  USB vendor ID
    uint32_t productId;         //  USB product ID
    uint32_t serialNumber;      //  USB device serial number
        
    std::string vendorName;			//  USB vendor name string     
};
    
struct SerialDevice
{
    PSTRING name;				// Serial device name
    PSTRING deviceClass;		// In most cases: IOSerialBSDClient
        
    PSTRING calloutDevice;		// UNIX serial callout device (serial port in /dev/tty...)
    PSTRING dialinDevice;		// UNIX serial dialin device
        
    ParentDevice parent;        // Parent device, either USB or Bluetooth supported
};


// Returns list of serial devices available on the platform
// as a vector of @{SerialDevice} objects.
class Interfaces
{
    virtual std::vector<SerialDevice> GetDevices() = 0;
};


#if defined(__APPLE__)
// Returns list of serial devices available in macOS, retrieved
// using IOKit.
class InterfacesOSX : public Interfaces
{
public:
    virtual std::vector<SerialDevice> GetDevices() override;
        
    virtual ~InterfacesOSX() {}
        
private:
    ParentDevice GetParentDevice(io_object_t& object);
        
    PSTRING CFStringToString(CFStringRef input);
    PSTRING GetDeviceClass(io_object_t& device);
    PSTRING GetPropertyString(io_object_t& device, const char* key);
    uint GetPropertyInt(io_object_t& device, const char* key);
    PSTRING GetStringDataForDeviceKey(io_object_t& device, CFStringRef key);
};
#endif

#if defined(_WIN32)
// Returns list of serial devices available in Windows machines
// Based on: https://stackoverflow.com/questions/2674048/what-is-proper-way-to-detect-availabel-serial-ports-on-windows
class InterfacesWin32 : public Interfaces
{
public:
	virtual std::vector<SerialDevice> GetDevices() override;

	virtual ~InterfacesWin32() {}
};
#endif