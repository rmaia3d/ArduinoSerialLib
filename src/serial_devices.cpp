//
//  serial_devices.cpp
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

#include "serial_devices.h"

// *************************************************************
// MacOS implementation
// Based on https://github.com/killpl/obd_cougar/tree/master/cougar_lib/serial
// *************************************************************

#if defined(__APPLE__)
// Converts a CFString to a STL string
PSTRING InterfacesOSX::CFStringToString(CFStringRef input)
{
    if (input) {
        int len = (int)CFStringGetLength(input) + 1; // ASCII, to allow further open() operation

        char* cstr = new char[len];
        Boolean result = CFStringGetCString(input, cstr, len, kCFStringEncodingASCII);
        CFRelease(input);
        
        if (result)
        {
            PSTRING resultString(cstr, len);
            delete[] cstr;
            return resultString;
        }
        delete[] cstr;
    }
    return "";
}

// Returns the device class as a STL string
PSTRING InterfacesOSX::GetDeviceClass(io_object_t& device)
{
    PSTRING result;
    io_name_t name;
    kern_return_t kern_result = IOObjectGetClass(device, name);
    if(kern_result == KERN_SUCCESS)
    {
        result = PSTRING(name);
    }
    return result;
}

// Returns the property string as a STL string
PSTRING InterfacesOSX::GetPropertyString(io_object_t& device, const char* key)
{
    PSTRING result;
    
    CFStringRef propertyName = CFStringCreateWithCString(kCFAllocatorDefault, key, kCFStringEncodingASCII);
    
    if (propertyName == NULL)
    {
        return result;
    }
    
    CFTypeRef propertyValue = IORegistryEntryCreateCFProperty(device, propertyName, kCFAllocatorDefault, 0);
    CFRelease(propertyName);
    
    if (propertyValue == NULL)
    {
#if DEVCON_DEBUG
        PCOUT << "Property " << key << " does not exist" << std::endl;
#endif
        return result;
    }
    
    if (CFGetTypeID(propertyValue) == CFStringGetTypeID())
    {
        return CFStringToString(static_cast<CFStringRef>(propertyValue));
    }
    else
    {
#if DEVCON_DEBUG
        PCOUT << "Property " << key << " is not string type" << std::endl;
#endif
    }
    
    CFRelease(propertyValue);
    return result;
}

// Returns the property integer for the device
uint InterfacesOSX::GetPropertyInt(io_object_t& device, const char* key)
{
    uint result = 0;
    
    CFStringRef propertyName = CFStringCreateWithCString(kCFAllocatorDefault, key, kCFStringEncodingASCII);
    
    if (propertyName == NULL)
    {
        return result;
    }
    
    CFTypeRef propertyValue = IORegistryEntryCreateCFProperty(device, propertyName, kCFAllocatorDefault, 0);
    CFRelease(propertyName);
    
    if (propertyValue == NULL)
    {
#if DEVCON_DEBUG
        PCOUT << "Property " << key << " does not exist" << std::endl;
#endif
        return result;
    }
    
    if (CFGetTypeID(propertyValue) == CFNumberGetTypeID())
    {
        CFNumberGetValue(static_cast<CFNumberRef>(propertyValue), kCFNumberSInt16Type, &result);
    }
    else
    {
#if DEVCON_DEBUG
        PCOUT << "Property " << key << " is not integer type" << std::endl;
#endif
    }
    
    CFRelease(propertyValue);
    return result;
}

// Returns the Device Key info as a STL string
PSTRING InterfacesOSX::GetStringDataForDeviceKey(io_object_t& device, CFStringRef key)
{
    CFTypeRef resultString = IORegistryEntryCreateCFProperty(device, key, kCFAllocatorDefault, 0);
    return CFStringToString((CFStringRef)resultString);
}

// Gets info from the Parent Device
ParentDevice InterfacesOSX::GetParentDevice(io_object_t& object)
{
    ParentDevice device;
    device.type = DeviceType::OTHER;
    
    io_registry_entry_t parent = 0;
    
    io_object_t deviceObject = object;
    IOObjectRetain(deviceObject);
    
    kern_return_t kern_result = KERN_SUCCESS;
    
    while (kern_result == KERN_SUCCESS)
    {
        kern_result = IORegistryEntryGetParentEntry(deviceObject,
                                                    kIOServicePlane,
                                                    &parent );
        
        if(kern_result != KERN_SUCCESS)
        {
#if DEVCON_DEBUG
            PCOUT << "Error accessing parent device" << std::endl;
#endif
            break;
        }
        
        IOObjectRelease(deviceObject);
        deviceObject = parent;
            
        if (GetDeviceClass(parent) == USB_DEVICE_ID)
        {
#if DEVCON_DEBUG
            PCOUT << BLUETOOTH_DEVICE_ID << std::endl;
#endif        
            device.type = DeviceType::USB_DEVICE;
            
            device.name = GetPropertyString(parent, "USB Product Name");
            device.vendorName = GetPropertyString(parent, "USB Vendor Name");
            device.serialNumber = GetPropertyInt(parent, "USB Serial Number");
            device.vendorId = GetPropertyInt(parent, "idVendor");
            device.productId = GetPropertyInt(parent, "idProduct");

#if DEVCON_DEBUG
            // Log the debug informations about the USB device
            PCOUT << "USB Product Name:\t" << device.name << std::endl;
            PCOUT << "USB Vendor Name:\t" << device.vendorName << std::endl;
            PCOUT << "USB Serial Number:\t" << device.serialNumber << std::endl;
            PCOUT << "Vendor:\t0x" << std::hex << device.vendorId << std::endl;
            PCOUT << "Product:\t0x" << std::hex << device.productId << std::endl;
#endif            
            break;
        }
        
        if (GetDeviceClass(parent) == BLUETOOTH_DEVICE_ID)
        {
#if DEVCON_DEBUG
            PCOUT << BLUETOOTH_DEVICE_ID << std::endl;
#endif
            
            device.type = DeviceType::BLUETOOTH_DEVICE;
            
            device.name = GetPropertyString(parent, "BTTTYName");
            device.channel = GetPropertyInt(parent, "BTRFCOMMChannel");
            device.connectionType = GetPropertyInt(parent, "BTSerialConnectionType");

#if DEVCON_DEBUG
            // Log the debug informations about the bluetooth device
            PCOUT << "BTRFCOMMChannel:\t" << device.channel << std::endl; // INT
            PCOUT << "BTName:\t" << GetPropertyString(parent, "BTName") << std::endl;
            PCOUT << "BTTTYName:\t" << device.name << std::endl;
            PCOUT << "PortDeviceName:\t" << GetPropertyString(parent, "PortDeviceName") << std::endl; // ?
            PCOUT << "BTSerialConnectionType:\t" << device.connectionType << std::endl; // INT
            PCOUT << "P49SerialPort:\t" << GetPropertyInt(parent, "P49SerialPort") << std::endl; // INT
#endif
            
            break;
        }
    }
    IOObjectRelease(deviceObject);
    return device; // RVO
}

// Returns the actual list of devices
std::vector<SerialDevice> InterfacesOSX::GetDevices()
{
    std::vector<SerialDevice> result;
    
    // Get devices iterator
    kern_return_t           kernResult = 0;
    CFMutableDictionaryRef  classesToMatch = IOServiceMatching(kIOSerialBSDServiceValue);
    
    if (classesToMatch == nullptr)
    {
#if DEVCON_DEBUG
        PCOUT << "None services matching kIOSerialBSDServiceValue found." << std::endl;
#endif
        return result;
    }
    
    io_iterator_t matchingServices;
    kernResult = IOServiceGetMatchingServices(kIOMasterPortDefault, classesToMatch, &matchingServices);
    
    if (KERN_SUCCESS != kernResult) {
#if DEVCON_DEBUG
        PCOUT << "IOServiceGetMatchingServices returned an error: " << kernResult << std::endl;
#endif
        return result;
    }
    
    // Iterate over found devices
    io_object_t serialPort;
    while ((serialPort = IOIteratorNext(matchingServices))) {
        SerialDevice device;
        
        device.name = GetStringDataForDeviceKey(serialPort, CFSTR(kIOTTYDeviceKey));
        device.calloutDevice = GetStringDataForDeviceKey(serialPort, CFSTR(kIOCalloutDeviceKey));
        device.dialinDevice = GetStringDataForDeviceKey(serialPort, CFSTR(kIODialinDeviceKey));
        device.deviceClass = GetDeviceClass(serialPort);

#if DEVCON_DEBUG
        PCOUT << kIOTTYDeviceKey << ":\t" << device.name << std::endl;
        PCOUT << kIOTTYBaseNameKey << ":\t" << GetStringDataForDeviceKey(serialPort, CFSTR(kIOTTYBaseNameKey)) << std::endl;
        PCOUT << kIOTTYSuffixKey << ":\t" << GetStringDataForDeviceKey(serialPort, CFSTR(kIOTTYSuffixKey)) << std::endl;
        PCOUT << kIOCalloutDeviceKey << ":\t" << device.calloutDevice << std::endl;
        PCOUT << kIODialinDeviceKey << ":\t" << device.dialinDevice << std::endl;
        
        PCOUT << "Class:\t" << device.deviceClass << std::endl;
#endif
        
        device.parent = GetParentDevice(serialPort);
        result.push_back(device);
        
        IOObjectRelease(serialPort);
    }
    IOObjectRelease(matchingServices);
    
    return result;
}
#endif

// *************************************************************
// Win32 implementation
// Based on: https://stackoverflow.com/questions/2674048/what-is-proper-way-to-detect-availabel-serial-ports-on-windows
// *************************************************************

#if defined(_WIN32)
// Returns the actual list of devices
std::vector<SerialDevice> InterfacesWin32::GetDevices()
{
	PCHAR lpTargetPath[5000];
	std::vector<SerialDevice> vec_ports;

	for (int i = 0; i < 255; i++)
	{
		PSTRING str = _S("COM") + TO_STRING(i);
		DWORD test = QueryDosDevice(str.c_str(), lpTargetPath, 5000);

		if (test != 0) {
#if DEVCON_DEBUG
			PCOUT << str << ": " << lpTargetPath << std::endl;
#endif
			SerialDevice device;

			device.name = lpTargetPath;			// Store path here as Win call is by name
			device.deviceClass = _S("");		// No info accessed
			device.calloutDevice = str;			// Use the actual name as callout path
			device.dialinDevice = _S("");		// No info
			device.parent = ParentDevice();		// No info

			vec_ports.push_back(device);
		}
	}

	return vec_ports;
}
#endif
