//
// test_serial_monitor.cpp
//
// File for testing the ArduinoSerialLib code, by continously reading strings from
// the serial port. 
//
// You can use with this file any sketch that outputs strings to the serial port.
// The program will continously listen to the serial port and output any incoming strings.
// Exit program execution by pressing Ctrl+C.
//
// Author: Rodrigo R. M. B. Maia
// Created: 12-Sept-2019
//

#include "serial_port.h"
#include "serial_devices.h"

#include <csignal>		// So std::signal can work

// Ctrl+C handler function
void sig_handler(int s) {
    printf("Caught signal %d\n", s);
    exit(1);
}

int main()
{
    // Define the Ctrl+C handler function
    std::signal(SIGINT, sig_handler);

	// Declare our objects
	INTERFACE_CLASS enum_ports;
	SERIAL_PORT serial;

	std::vector<SerialDevice> vec_ports;

	// List available ports for user choice
	vec_ports = enum_ports.GetDevices();

	int i = 0;
	for (SerialDevice dev : vec_ports)
	{
		PCOUT << i << " : " << dev.name << " - " << dev.calloutDevice << std::endl;
		i++;
	}
	if (vec_ports.size() == 0) {
		PCOUT << "No serial ports found on device. Quitting." << std::endl;
		return 0;
	}

	// Ask user for serial port choice and baud rate
	std::cout << "Enter serial port index to connect to: ";
	int p = 0;
	std::cin >> p;

	int baud;
	std::cout << "Enter the desired connection baud rate: ";
	std::cin >> baud;

	// Attempt to open the selected port
	int sres = serial.open_port(vec_ports.at(p).calloutDevice, baud);

    if(sres != -1)		// Open succeed, read the incoming lines
    {
		std::string read_str;

		while (1)		// At runtime, break loop execution with Ctrl+C
		{
			serial.sreadline(read_str);
			std::cout << read_str;
			read_str.clear();
		}
    }
	else {
		std::cout << "Couldn't open port. Quitting." << std::endl;
	}
    
    return 0;
}