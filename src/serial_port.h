//
//  serial_port.h
//  
//  Serial port access class, with reading and writing methods
//  Aimed at connecting to Arduino based devices
//
//  Inspired by https://github.com/todbot/arduino-serial
//  And: https://www.xanthium.in/Serial-Port-Programming-using-Win32-API
//
//  Rodrigo R. M. B. Maia
//  Created 08-Sept-2019
//

#pragma once

#if defined(__APPLE__)
    #include <unistd.h>   // UNIX standard function definitions
    #include <fcntl.h>    // File control definitions
    #include <errno.h>    // Error number definitions
    #include <termios.h>  // POSIX terminal control definitions
#elif defined(_WIN32)
    #include <windows.h>
#endif

#include <string>     
#include <iostream>
#include <vector>

#if defined(__APPLE__)
    #define SERIAL_PORT SerialPort
#elif defined(_WIN32)
    #define SERIAL_PORT SerialPortWin32
#endif

// Needed for Win32, if UNICODE is set, system functions for port opening
// require wchar based strings
#ifdef UNICODE
	#define PCOUT std::wcout
	#define PSTRING std::wstring
	#define _S(X) L##X
#else
	#define PCOUT std::cout
	#define PSTRING std::string
	#define _S(X) X
#endif	

#define PORTCON_DEBUG 1    // Set to 1 to print error messages to console output

#if defined(__APPLE__)
class SerialPort
{
public:
    SerialPort();
    SerialPort(const std::string _portname, int _baud, int _timeout = 0);

    int open_port();
    int open_port(const std::string _portname, int _baud, 
                  int _timeout = 0);                                // Open port (if used default constructor)
    int swrite(uint8_t byte);                                       // Write single byte
    int swrite(const std::string str);                              // Write string
    int sread(uint8_t &byte);                                       // Read single byte
    int sread(std::vector<uint8_t> &vec_bytes, int size);           // Read size bytes
    int sreadline(std::string &read_str, int max_size = 256);       // Read line into string
    int sread_until(std::vector<uint8_t> &vec_bytes, char until, 
                   int max_size = 256);                             // Read until passed character
    int sclose();        // Close the port
    int sflush();        // Flush the port

private:
    std::string port_name;
    int baudrate;
    int fd; 
    int timeout;       
};
#endif

#if defined(_WIN32)
// Based on: https://www.xanthium.in/Serial-Port-Programming-using-Win32-API
class SerialPortWin32
{
public:
    SerialPortWin32();
	SerialPortWin32(const PSTRING _pname, int _baud, int _timeout = 0);
    ~SerialPortWin32();
    
	int open_port();
	int open_port(const PSTRING _pname, int _baud, int _timeout = 0);	// Open port (if used default constructor)
	int swrite(uint8_t byte);											// Write single byte
	int swrite(const std::string str);										// Write string
	int sread(uint8_t &byte);											// Read single byte
	int sread(std::vector<uint8_t> &vec_bytes, int size);				// Read size bytes
	int sreadline(std::string &read_str, int max_size = 256);				// Read line into string
	int sread_until(std::vector<uint8_t> &vec_bytes, char until,
		int max_size = 256);											// Read until passed character
	int sclose();														// Close the port

private:
    HANDLE com;
    DCB dcb;
    COMMTIMEOUTS timeouts;

	PSTRING port_name;
	DWORD baud_rate;
	int timeout;
};
#endif