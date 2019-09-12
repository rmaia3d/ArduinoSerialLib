//
// serial_write_test
//
// Sketch for testing the ArduinoSerialLib I/O connection
// Upload this to your Arduino, no need for any perypherals,
// just the basic board will do. Connect to you computer via USB
// and run the test_serial_io.cpp file.
//
// Author: Rodrigo R. M. B. Maia
// Created: 12-Sept-2019
//

#include <Arduino.h>

bool led_on = false;

void setup() {
  // Initialize the serial connection
  Serial.begin(9600);

  // Initialize the LED pin. If not using the built-in LED, change pin accordingly
  pinMode(LED_BUILTIN, OUTPUT);
  
  delay(1000);
}

void loop() {
  if(Serial.available() > 0) {
    // Read the string coming from the serial connection    
    String in_str = Serial.readString();
    
    // Parse the string to turn on or off the LED
    if(in_str == "LED on" && led_on == false) {
      digitalWrite(LED_BUILTIN, HIGH);    // Turn on the LED
      led_on = true;
    }
    if(in_str == "LED off" && led_on == true) {
      digitalWrite(LED_BUILTIN, LOW);     // Turn off the LED
      led_on = false;
    }

    // Print back the incoming string
    Serial.print("Incoming string was: ");
    Serial.println(in_str);
  }
}
 