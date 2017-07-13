/* N Channel Transmitter (Proportional Control With 2 Trimmer Buttons Per Channel)
 
 This sketch is intended as a learning tool for multi-channel RC hobbies. It is based on the
 Arduino Debounce example by David Mellis and the RadioHead library by Mike McCauley. Uses an
 ASK transmitter. Implements a simplex (one-way) transmitter with an TX-C1 module. For more 
 details on the RadioHead library see http://www.airspayce.com/mikem/arduino/RadioHead/
 
 NOTE: Unreliable UDP-like transmission which is simple to implement but data can be lost
 over transmission.

 The circuit for 1 channel:
 * pushbutton1 attached from pin 2 to +5V
 * 1K resistor attached from pin 2 to ground
 * pushbutton2 attached from pin 4 to +5V
 * 1K resistor attached from pin 4 to ground
 * 1K resistor attached from 5V to trimpot outer pin
 * trimpot outer pin attached to analog A1
 * 1K resistor attached from GND to trimpot inner pin
 * 433MHz Tx module connected to 5V, GND and pin 12 to ATAD

 Author: Andrew R. from HobbyTransform (http://hobbytransform.com/)
 
*/

#include <RH_ASK.h>
#include <SPI.h>                    // Not actually used but needed to compile

#define POT_TOLERANCE 10            // Noise tolerance level for potentiometers
#define POT_NUM 1                   // 1 in this case, but a handheld radio has up to 4

// Constants
int potPin[POT_NUM];                // Potentiometer pin numbers
int buttonPin[POT_NUM][2];          // Button pin numbers, each potentiometer has 2 buttons
uint8_t buflen = 3;                 // Buffer length: 1 letter per channel + a 2 byte reading

// Variables will change:
int buttonState[POT_NUM][2];        // The current reading from the button pins
int lastButtonState[POT_NUM][2];    // The previous reading from the push-button pins
int joystick[POT_NUM];              // Raw potentiometer readings from joystick commands
int trimmer[POT_NUM];               // Trimmer settings
int channelPos[POT_NUM];            // Final channel data to transmit wirelessly

// The following variables are longs because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastDebounceTime[POT_NUM][2];  // The last time each output pin was toggled
long debounceDelay = 50;            // The debounce time; increase if the output flickers

RH_ASK driver;


void setup() {
  int c, b;
  potPin[0] = A0;                         // Channel 1 potentiometer on analog A0
  buttonPin[0][0] = 2;                    // Channel 1 push-button 1 on pin 2
  buttonPin[0][1] = 4;                    // Channel 1 push-button 2 on pin 4

  //Iterate over channels and buttons
  for(c = 0; c < POT_NUM; c++){
    //Set initial states to 0
    trimmer[c] = 0;
    for(b = 0; b < 2; b++){
      lastButtonState[c][b] = LOW;
      lastDebounceTime[c][b] = 0;
      // Set button pins as inputs
      pinMode(buttonPin[c][b], INPUT);
    }
    joystick[c] = analogRead(potPin[c]);  // Get initial joystick reading
  }

  Serial.begin(9600);
  if (!driver.init())                     // Initiates and sets pin D12 as the TX pin
    Serial.println("init failed");
}

void loop() {
  int c, b, potReading, buttonReading;
  
  //Iterate over channels and buttons
  for(c = 0; c < POT_NUM; c++){
    // Read the joystick position
    potReading = analogRead(potPin[c]);
    joystick_update(c, &potReading);

    for(b = 0; b < 2; b++){
      // Read button to update trimmer
      buttonReading = digitalRead(buttonPin[c][b]);
      trim_update(c, b, &buttonReading);
      
      // Save the readings
      lastButtonState[c][b] = buttonReading;
    }
  }
}


// Helper Functions
void joystick_update(int c, int* potReading){
  // Only accept reading if the change is outside noise tolerance level
  if(abs(*potReading - joystick[c]) > POT_TOLERANCE){
    joystick[c] = *potReading;
    pos_update(c);
  }
}

void trim_update(int c, int b, int* buttonReading){
  // Check to see if you just pressed the button (i.e.
  // the input went from LOW to HIGH), and you've waited
  // long enough since the last press to ignore any noise:
  
  // If the switch changed, due to noise or pressing:
  if (*buttonReading != lastButtonState[c][b]) {
    // Reset the debouncing timer
    lastDebounceTime[c][b] = millis();
  }
  
  if ((millis() - lastDebounceTime[c][b]) > debounceDelay) {
    // Whatever the reading is at, it's been there for longer than
    // the debounce delay, so take it as the actual current state:

    // If the button state has changed:
    if (*buttonReading != buttonState[c][b]) {
      buttonState[c][b] = *buttonReading;

      // Only update trimmer if the new button state is HIGH
      if (buttonState[c][b] == HIGH) {
        if(b == 0 && channelPos[c] < 1023){
          trimmer[c]++;
        } else if(b == 1 && channelPos[c] > 0) {
          trimmer[c]--;
        }
        // Update channel position
        pos_update(c);
      }
    }
  }
}

void pos_update(int c){
  //Update position, send over channel and print data
  channelPos[c] = joystick[c] + trimmer[c];
  send_message('A'+c,&channelPos[c]);
  Serial.print("Channel: ");
  Serial.print(char('A'+c));
  Serial.print("  joystick: ");
  Serial.print(joystick[c]);
  Serial.print("  trimmer: ");
  Serial.print(trimmer[c]);
  Serial.print("  position: ");
  Serial.print(channelPos[c]);
  Serial.println("");
}

void send_message(char chanID, int *msg){
  //Since the reading is somewhere between 0 and 1023 (from the analog read) and we are only
  //sending 8-bit packets, each reading must be split as two 8-bit elements and channel ID is
  //sent as a single character, 8-bits large by default.
  uint8_t buf[buflen];
  buf[0] = chanID;    // Label this channel, to distinguish it from other channels
  buf[1] = *msg;      // Least significant byte (8 bits), rest is truncated from int to uint8_t
  buf[2] = (*msg)>>8; // Most significant byte
  
  digitalWrite(13, true);       // Flash a light to show transmitting
  driver.send(buf, buflen);     // Sending the message
  driver.waitPacketSent();      // Wait until the whole message is gone                
  digitalWrite(13, false);      // Turn the LED off.
  delay(50);                    // A short gap.
}
