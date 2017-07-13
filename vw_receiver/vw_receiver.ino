/* N Channel Receiver

 This sketch is a multi channel receiver for RF modules. Each channel sends (via UDP 
 transmission) a letter ID to distinguish between channels and a 16-bit number to
 indicate joystick position for that channel. The message is printed to the serial 
 monitor. It is a simple example of how to use the VirtualWire library to receive
 messages. Implements a simplex (one-way) receiver with an Rx-B1 module.

 NOTE: since UDP is unreliable and data can be lost in transmission, this code is only
 intended as a learning tool.

 Based on Mike McCauley's receiver code for VirtualWire
 See VirtualWire.h for detailed API docs.

 The circuit:
 * 433MHz Rx module connected to 5V, GND and pin 12 to DATA pin

 Author: Andrew R. from HobbyTransform (http://hobbytransform.com/)

*/

#include <VirtualWire.h>

uint8_t buflen = VW_MAX_MESSAGE_LEN;

void setup(){
    Serial.begin(9600);
    Serial.println("setup");    // Prints "Setup" to the serial monitor
    vw_set_rx_pin(12);          // Sets pin D12 as the RX Pin
    vw_set_ptt_inverted(true);  // Required for DR3100
    vw_setup(4000);	            // Bits per sec
    vw_rx_start();              // Start the receiver PLL running
}

void loop(){
    uint8_t buf[buflen];
    if (vw_get_message(buf, &buflen)){  // Non-blocking
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
