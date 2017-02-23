/**
 * Created by Olavi Kamppari on 5/23/2016.
 */

/**
 * A test program for two Infrared Collision Detection Sensors connected to
 * Arduino Mega Sensor shield in Wissahickon Rover
 *  - connected to input pins 18..25
 *  - for sensors: LF, FL, FR, RF, RB, BR, BL, and LB
 *  
 *  Where
 *  - L = LE ft
 *  - R = ri ght
 *  - F = Fr nt
 *  - B = BA ck
 *
 * The results  LED&KEY panel.
 *  The LEDs are showing the states of the 8 IR sensors
 *  The display shows the pin number and the sensor identification
 *  
 */
  
#include <TM1638.h>                 // Include the LED & KEY library

TM1638  panel(37,36,35);            // Pin order: STB, CLK, DIO

#define firstPin  18

char* sensorName[] = {"LE Fr", "Fr LE", "Fr ri", "ri Fr",
                      "ri BA", "BA ri", "BA LE", "LE BA"};
uint8_t prevPattern = 0;
                           
void setup() {
  panel.setLEDs(prevPattern);       // Reset LEDs
  panel.writeText(0,"NO SENSR");      // Update display
  for (uint8_t i=0;i<8;i++) {       // Setup input pins
    pinMode(firstPin + i, INPUT_PULLUP);
  }
}

void loop() {
  uint8_t   pattern = 0;
  uint8_t   activeChannel;

  for (uint8_t i=0;i<8;i++) {       // Scan the IR Sensors
    pattern <<= 1;                  // Shift the pattern to left
    if (!digitalRead(firstPin + i)) {
      pattern++;                    // Mark the active sensor
      activeChannel = i;
    }
  }

  if (prevPattern != pattern) {     // Update panel only for changes
    prevPattern = pattern;
    panel.setLEDs(pattern);
    if (pattern)  {
                                    // Show the last active channel
      panel.writeDec    (0,firstPin + activeChannel, 2);
      panel.setSegments (2,0);
      panel.writeText   (3,sensorName[activeChannel]);
    } else {
                                    // Show that there are no active sensors
      panel.writeText   (0,"No SENSr");
    }
  }
}
