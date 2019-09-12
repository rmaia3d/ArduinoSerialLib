//
// test_serial_io.cpp
//
// File for testing the ArduinoSerialLib code, by writing a user input string to the
// Arduino and reading back the echo response. 
//
// Use the serial_write_test.ino sketch in conjunction with this file. That sketch 
// will take the string and send back it's echo, exactly as was read by the board. 
// Look into it's code for more details.
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

    SERIAL_PORT serial;
    INTERFACE_CLASS enum_ports;
    
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
    std::string p_str;
    std::getline(std::cin, p_str);
    int p = std::stoi(p_str);
    
    std::string baud_str;
    std::cout << "Enter the desired connection baud rate: ";
    std::getline(std::cin, baud_str);
    int baud = std::stoi(baud_str);
    
    // Attempt to open the selected port
    int sres = serial.open_port(vec_ports.at(p).calloutDevice, baud, 50);
    
    if(sres < 0)
        return 0;
    
    std::string out_str;
	while(1)		// Type "quit" to exit loop
	{
		std::cout << "Enter string to send to device: ";
		std::getline(std::cin, out_str);
        std::cout << "Entered string: " << out_str << " size: " << out_str.size() << std::endl;

		if(out_str == "quit")
			break;

		int nout = serial.swrite(out_str);
		std::cout << "Wrote: " << nout << " - ";
    
		if(nout == out_str.size()) {
			std::string read_str;
       		int rr = 0;
        	for(int i = 0; i < 255; i++)
        	{	
				// Try reading several times to circunvent timeouts
            	rr = serial.sreadline(read_str);
            	if(rr > 0) break;		// Succesful read, quit loop
            	read_str.clear();		// Clear buffer and try again
            	i++;
        	}
        	std::cout << "Read: " << rr << " : " << read_str << std::endl;
		}
	} 

    return 0;
}