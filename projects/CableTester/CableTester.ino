/**
 * Created by Olavi Kamppari on 4/10/2016.
 */

/**
 *  Simple sensor cable tester where 
 *    - panel button controls an output
 *    - the output is connected with a cable wire to an input
 *    - the input controls a panel LED
 */
 
#include <TM1638.h>               // Include the LED & KEY library

TM1638  panel(37,36,35);          // Pin order: STB, CLK, DIO

#define yellowIn  14              // In lower camelcase
#define yellowOut 26              //  the first word is not capitalized 
#define blackIn   15              //  all other words are capitalized
#define blackOut  27
#define redIn     16
#define redOut    28

void setup() {
                                  // Setup the pins for either input or output
  pinMode(yellowIn, INPUT);
  pinMode(yellowOut,OUTPUT);
  pinMode(blackIn,  INPUT);
  pinMode(blackOut, OUTPUT);
  pinMode(redIn,    INPUT);
  pinMode(redOut,   OUTPUT);

  delay(100);                               // Flash all LEDs and digits
  panel.allOff();
}

void loop() {
                                            // Send stimuli to all outputs
                                            //  Read the current button states
                                            //  Update the output pins
  digitalWrite(yellowOut, panel.getButton(1));
  digitalWrite(blackOut,  panel.getButton(2));
  digitalWrite(redOut,    panel.getButton(3));
                                            // Render the results in LEDs
  panel.setLED(1,digitalRead(yellowIn));
  panel.setLED(2,digitalRead(blackIn));
  panel.setLED(3,digitalRead(redIn));
}
