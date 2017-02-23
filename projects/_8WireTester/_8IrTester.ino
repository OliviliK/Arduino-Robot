/**
 * Created by Olavi Kamppari on 4/10/2016.
 */

/**
 * A general purpose tester for cables with female pins
 * This is used with Arduino Mega Sensor shield that has male pins.
 * Every pin has Gnd and Vcc next to it for simple sensor/servo connections.
 * The test stimuli is given by the buttons in LED&KEY panel (TM1638)
 * The button drives an output pin connected to a wire.  The other end of the wire
 * is connected to an input pin.
 * The input pin drives a LED in the LED&KEY panel.
 * The corresponding display segments have the following meanings
 *    blank             = no stimuli, no response
 *    small letter o    = OK for a response of a stimuli
 *    capital letter F  = FAILURE to respond for a stimuli
 *    special letter E  = ERROR for a response without a stimuli
 *    
 *  All unconnected wires show the special E (triple dash)
 *  
 *  Note:
 *    The pull-up resistor is enabled in the inputs to detect the unconnected wires.
 */
  
 #include <TM1638.h>               // Include the LED & KEY library

TM1638  panel(37,36,35);          // Pin order: STB, CLK, DIO

#define firstIn     14            // First Input Pin
#define firstOut    26            // First Output Pin
#define pinCount    8             // Number of pins used in tests

#define failLetter  0x71          // F for failure to connect
#define okLetter    0x5C          // o for proper connection
#define errorLetter 0x49          // Triple dash, kind of E

void setup() {
                                  // Setup the pins for either input or output
  for (uint8_t i=0;i<pinCount;i++) {
    pinMode(firstIn + i,  INPUT_PULLUP);
    pinMode(firstOut + i, OUTPUT);
  }
  delay(100);                     // Flash all LEDs and digits
  panel.allOff();
}

void loop() {
  uint8_t buttonState, pinState, pattern;

  for (uint8_t i=0;i<pinCount;i++) {
    buttonState   = panel.getButton(1 + i);      // Read inputs
    pinState      = digitalRead(firstIn + i);
    
    digitalWrite(firstOut + i, buttonState);    // Write outputs
    panel.setLED(1+ i,pinState);

    if (buttonState == pinState) {
                                                // Pin matches button
      pattern = buttonState ? okLetter : 0;     // If button is down
                                                //  show OK
    } else {
                                                // Pin doesn't match button
      pattern = buttonState ? failLetter : errorLetter;                                                
    }

    panel.setSegments(i,pattern);           // Show status letter
  }
}
