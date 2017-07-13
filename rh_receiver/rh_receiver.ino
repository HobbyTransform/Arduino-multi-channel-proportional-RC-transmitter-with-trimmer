/* N Channel Receiver

 This sketch is a multi channel receiver for RF modules. Each channel sends (via UDP- 
 like transmission) a letter ID to distinguish between channels and a 16-bit number
 to indicate joystick position for that channel. The message is printed to the serial 
 monitor. It is a simple example of how to use the RadioHead library to receive
 messages. Implements a simplex (one-way) receiver with an Rx-B1 module.

 NOTE: since UDP is unreliable and data can be lost in transmission, this code is only
 intended as a learning tool.

 Based on Mike McCauley's ASK receiver code for RadioHead. For more details on the
 RadioHead Library see http://www.airspayce.com/mikem/arduino/RadioHead/

 The circuit:
 * 433MHz Rx module connected to 5V, GND and pin 11 to DATA pin

 Author: Andrew R. from HobbyTransform (http://hobbytransform.com/)

*/

#include <RH_ASK.h>
#include <SPI.h>                // Not actualy used but needed to compile

uint8_t buflen = RH_ASK_MAX_MESSAGE_LEN;

RH_ASK driver;

void setup(){
    Serial.begin(9600);
    Serial.println("setup");    // Prints "Setup" to the serial monitor
    if (!driver.init())         // Initiates and sets pin D11 as the RX Pin
         Serial.println("init failed");
}

void loop(){
    uint8_t buf[buflen];
    if (driver.recv(buf, &buflen)){  // Non-blocking
      int i;
      digitalWrite(13, true);  // Flash a light to show received good message
      // Message with a good checksum received, dump it.
      Serial.print("Got: ");
      
      for (i = 0; i < buflen-2; i += 3){
        char chanID = buf[i];               // Extract channel ID
        int c = (buf[i+2] << 8) | buf[i+1]; // Reconstruct 16-bit joystick reading
        Serial.print(chanID);
        Serial.print(" ");
        Serial.print(c);
      }
      Serial.println("");
      digitalWrite(13, false);
    }
}
