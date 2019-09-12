//
//  serial_port.cpp
//
//  Serial port access class, with reading and writing methods
//  Aimed at connecting to Arduino based devices
//
//  Rodrigo R. M. B. Maia
//  Created 08-Sept-2019
//

#include "serial_port.h"

// *************************************************************
// MacOS implementation
// Inspired by https://github.com/todbot/arduino-serial
// *************************************************************

#if defined(__APPLE__)
// Default constructor, need to call open_port() later with
// appropriate parameters to start connection
SerialPort::SerialPort()
{
    port_name = "";
    baudrate = 0;
    fd = -1;
    timeout = 0;
}

// Construct class and open connection with passed parameters
// _portname = name of serial port (eg: "COM2", "dev/tty.usbmodem431", etc)
// _baud = baud rate (9600, 14400, 57600, 115200, etc)
// _timeout = timeout in ms for each read attempt (default 0ms)
SerialPort::SerialPort(const std::string _portname,
                       int _baud, int _timeout)
{
    port_name = _portname;
    baudrate = _baud;
    timeout = _timeout;

    fd = this->open_port();
}

// Opens connection with internally stored parameters
int SerialPort::open_port()
{
    struct termios toptions;
    int pd;

    pd = open(port_name.c_str(), O_RDWR | O_NONBLOCK );
    
    if (pd == -1)  {
#if PORTCON_DEBUG
        std::cerr << "SerialPort open_port: Unable to open port " <<
            strerror(errno) << std::endl;
#endif
        return -1;
    }
    
    if (tcgetattr(pd, &toptions) < 0) {
#if PORTCON_DEBUG
        std::cerr << "SerialPort open_port: Couldn't get term attributes" <<
            strerror(errno) << std::endl;
#endif
        return -1;
    }

    speed_t brate = baudrate; 
    switch(baudrate) {
        case 4800:   brate=B4800;   break;
        case 9600:   brate=B9600;   break;
#ifdef B14400
        case 14400:  brate=B14400;  break;
#endif
        case 19200:  brate=B19200;  break;
#ifdef B28800
        case 28800:  brate=B28800;  break;
#endif
        case 38400:  brate=B38400;  break;
        case 57600:  brate=B57600;  break;
        case 115200: brate=B115200; break;
    }
    cfsetispeed(&toptions, brate);
    cfsetospeed(&toptions, brate);
    
    // 8N1
    toptions.c_cflag &= ~PARENB;
    toptions.c_cflag &= ~CSTOPB;
    toptions.c_cflag &= ~CSIZE;
    toptions.c_cflag |= CS8;
    // Disable flow control
    toptions.c_cflag &= ~CRTSCTS;
    
    // Disable hang-up-on-close to avoid reset
    // toptions.c_cflag &= ~HUPCL;   
    
    // Turn on READ & ignore ctrl lines
    toptions.c_cflag |= CREAD | CLOCAL;
    // Turn off s/w flow ctrl
    toptions.c_iflag &= ~(IXON | IXOFF | IXANY  | INLCR | ICRNL); 
    
    // Make raw
    toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    toptions.c_oflag &= ~OPOST; 
    
    // Reference: http://unixwiz.net/techtips/termios-vmin-vtime.html
    toptions.c_cc[VMIN]  = 0;
    toptions.c_cc[VTIME] = 20;
    //toptions.c_cc[VTIME] = 20;
    
    tcsetattr(pd, TCSANOW, &toptions);
    if( tcsetattr(pd, TCSAFLUSH, &toptions) < 0) {
#if PORTCON_DEBUG
        std::cerr << "SerialPort open_port: Couldn't set term attributes" <<
            strerror(errno) << std::endl;
#endif
        return -1;
    }
    
    fd = pd;

    return fd;
}

// Stores internally the passed parameters and open connection with them
int SerialPort::open_port(const std::string _portname, 
                          int _baud, int _timeout)
{
    port_name = _portname;
    baudrate = _baud;
    timeout = _timeout;

    fd = this->open_port();

    return fd;
}

// Write single byte
int SerialPort::swrite(uint8_t byte)
{
    int n = static_cast<int>(write(fd, &byte, 1));
    
    if(n != 1)
        return -1;
    
    return n;       // Return number of bytes written
}

// Write full string
int SerialPort::swrite(const std::string str)
{
    int len = static_cast<int>(str.size());
    int n = static_cast<int>(write(fd, str.c_str(), len));
    
    if(n != len) {
#if PORTCON_DEBUG
        std::cerr << "SerialPort swrite: couldn't write whole string" <<
            strerror(errno) << std::endl;
#endif
        return -1;
    }

    return n;       // Return number of bytes written
} 

// Read single byte (note argument passed byref)
int SerialPort::sread(uint8_t &byte)
{
    int n = static_cast<int>(read(fd, &byte, 1));
    
    if(n == -1) 
        return -1;              // Couldn't read
    if(n == 0) {
        for(int tout = timeout; tout > 0; tout--)
        {
            usleep(1 * 1000);   // Wait 1ms before trying again
            n = static_cast<int>(read(fd, &byte, 1));
            if(n > 0) 
                return n;
        }
        return -2;              // Timed out
    }

    return n;                   // Return number of bytes read
}

// Read number of bytes defined in size argument (note byref input)
int SerialPort::sread(std::vector<uint8_t> &vec_bytes, int size)
{
    uint8_t b;
    int i = 0;
    while(i < size)
    {
        int n = this->sread(b);
        if(n == 1) {
            vec_bytes.push_back(b);
        }
        else 
            return n;      // Timed out or read error
        i++;
    }

    return static_cast<int>(vec_bytes.size());
}

// Read full line (ending defined as the '\n' character, or
// max_size bytes, whatever happens first). Stores the read
// result in the string argument passed byref.
int SerialPort::sreadline(std::string &read_str, int max_size)
{
    std::vector<uint8_t> vec_str;
    int n = this->sread_until(vec_str, '\n', max_size);

    for(uint8_t byte : vec_str)
    {
        // Convert to char object so will print as ASCII
        read_str += static_cast<char>(byte);
    }

    return n;
}

// Reads until either the character defined in until is read or
// max_size bytes is reached. Whichever happens first. And stores
// the read result in the vector of byte objects passed byref.
int SerialPort::sread_until(std::vector<uint8_t> &vec_bytes, 
                            char until, int max_size)
{
    uint8_t b;
    int i = 0;

    do
    {
        int n = this->sread(b);
        if(n == 1) {
            vec_bytes.push_back(b);
        }
        else 
            return n;      // Timed out or read error
        i++;
    } while (b != until && i + 1 < max_size);
    
    return static_cast<int>(vec_bytes.size());
}

// Close serial connection
int SerialPort::sclose()
{
    return close(fd);
}

// Flush serial connection
int SerialPort::sflush()
{
    sleep(2); // Required to make flush work, for some reason
    return tcflush(fd, TCIOFLUSH);
}
#endif

// *************************************************************
// Win32 implementation
// Based on: https://www.xanthium.in/Serial-Port-Programming-using-Win32-API
// *************************************************************

#if defined(_WIN32)
// Default constructor, need to call open_port() later with
// appropriate parameters to start connection
SerialPortWin32::SerialPortWin32()
{	
	com = NULL;
	dcb = { 0 };
	timeouts = { 0 };
	port_name = _S("");
	baud_rate = 0;
	timeout = 0;
}

// Construct class and open connection with passed parameters
// _portname = name of serial port (eg: "COM2")
// _baud = baud rate (9600, 14400, 57600, 115200, etc)
// _timeout = timeout in ms for each read attempt (default 0ms)
SerialPortWin32::SerialPortWin32(const PSTRING _pname, int _baud, int _timeout)
{
	com = NULL;
	dcb = { 0 };
	timeouts = { 0 };
	port_name = _pname;
	baud_rate = _baud;
	timeout = _timeout;

	this->open_port();
}

// Destructor, closes the connection
SerialPortWin32::~SerialPortWin32()
{
    this->sclose();
}

// Opens connection with internally stored parameters
int SerialPortWin32::open_port()
{
    // Begin by closing any existing connection (if any)
	this->sclose();

    com = CreateFile(port_name.c_str(), 
                     GENERIC_READ | GENERIC_WRITE,
                     0,
                     NULL,
                     OPEN_EXISTING,
                     0,
                     NULL);

    if(com == INVALID_HANDLE_VALUE) {
        this->sclose();
#if PORTCON_DEBUG
        PCOUT << "Couldn't open port: " << port_name << std::endl;
#endif
        return -1;
    }

	dcb.DCBlength = sizeof(dcb);
	GetCommState(com, &dcb);

	// 8N1 - Default Arduino parameters
	dcb.BaudRate = baud_rate;
	dcb.ByteSize = 8;
	dcb.StopBits = ONESTOPBIT;
	dcb.Parity = NOPARITY;
 
    if(!SetCommState(com, &dcb)) {
        this->sclose();
#if PORTCON_DEBUG
        std::cout << "Couldn't set DCB attributes: " << std::endl;
#endif
        return -1;
    }

    timeouts.ReadIntervalTimeout = timeout;			// Read interval timeout
    timeouts.ReadTotalTimeoutConstant = timeout;	// Read time constant
    timeouts.ReadTotalTimeoutMultiplier = 10;		// Read time coefficient
    timeouts.WriteTotalTimeoutConstant = timeout;	// Write time constant
    timeouts.WriteTotalTimeoutMultiplier = 10;		// Write time coefficient

    if(!SetCommTimeouts(com, &timeouts)) {
        this->sclose();
#if PORTCON_DEBUG
        PCOUT << "Couldn't set Comm Timeouts: " << port_name << std::endl;
#endif
        return -1;
    }

    return 1;		// Successfully opened port
}

// Stores internally the passed parameters and open connection with them
int SerialPortWin32::open_port(const PSTRING _portname, int _baud, int _timeout)
{
	port_name = _portname;
	baud_rate = _baud;
	timeout = _timeout;

	int ret = this->open_port();

	return ret;
}

// Write single byte
int SerialPortWin32::swrite(uint8_t byte)
{
	DWORD nBytesWritten;

	if (com) {
		WriteFile(com, &byte, sizeof(byte), &nBytesWritten, NULL);

		return nBytesWritten;
	}

	return -1;		// Couldn't write any bytes
}

// Write full string
int SerialPortWin32::swrite(const std::string str)
{
	DWORD len = str.size();
	DWORD nBytesWritten;
	WriteFile(com, str.c_str(), len, &nBytesWritten, NULL); 

	if (nBytesWritten != len) {
#if PORTCON_DEBUG
		std::cout << "SerialPort swrite: couldn't write whole string" << std::endl;
#endif
		return -1;
	}

	return nBytesWritten;       // Return number of bytes written
}

// Read single byte (note argument passed byref)
int SerialPortWin32::sread(uint8_t &byte)
{
	if (com) {
		DWORD nBytesRead;

		ReadFile(com, &byte, sizeof(byte), &nBytesRead, NULL);
		
		return static_cast<int>(nBytesRead);
	}

	return -1;		// Couldn't read any bytes
}

// Read number of bytes defined in size argument (note byref input)
int SerialPortWin32::sread(std::vector<uint8_t> &vec_bytes, int size)
{
	uint8_t b;
	int i = 0;
	while (i < size)
	{
		int n = this->sread(b);
		if (n == 1) {
			vec_bytes.push_back(b);
		}
		else
			return n;      // Timed out or read error
		i++;
	}

	return static_cast<int>(vec_bytes.size());
}

// Read full line (ending defined as the '\n' character, or
// max_size bytes, whatever happens first). Stores the read
// result in the string argument passed byref.
int SerialPortWin32::sreadline(std::string &read_str, int max_size)
{

	std::vector<uint8_t> vec_str;
	int n = this->sread_until(vec_str, '\n', max_size);

	for (uint8_t byte : vec_str)
	{
		// Convert to char object so will print as ASCII
		read_str += static_cast<char>(byte);
	}

	return n;
}

// Reads until either the character defined in until is read or
// max_size bytes is reached. Whichever happens first. And stores
// the read result in the vector of byte objects passed byref.
int SerialPortWin32::sread_until(std::vector<uint8_t> &vec_bytes,
	char until, int max_size)
{
	uint8_t b;
	int i = 0;

	do
	{
		int n = this->sread(b);
		if (n == 1) {
			vec_bytes.push_back(b);
		}
		else
			return n;      // Timed out or read error
		i++;
	} while (b != until && i + 1 < max_size);

	return static_cast<int>(vec_bytes.size());
}

// Close serial connection
int SerialPortWin32::sclose()
{
	if (com) {
		CloseHandle(com);
		com = NULL;
		return 1;		// Successfully closed
	}
	return 0;			// Couldn't close port
}
#endif