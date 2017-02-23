/**
 * Created by Olavi Kamppari on 5/22/2016.
 */

/**
 * A test program for two Optical Distance Sensors connected to
 * Arduino Mega Sensor shield in Wissahickon Rover
 *  A6 for the left sensor, ODS_L
 *  A7 for the right sensor, ODS_R
 *
 * The results  LED&KEY panel.
 *  ODS_L is shown in digits 0..3
 *  ODS_R is shown in digits 4..7
 *  
 */
  
 #include <TM1638.h>               // Include the LED & KEY library

TM1638  panel(37,36,35);            // Pin order: STB, CLK, DIO

void setup() {
  Serial.begin(230400);             // Open Serial Plotter
  
  delay(100);                       // Flash all LEDs and digits
  panel.allOff();
}

void loop() {
  uint16_t  odsL  = analogRead(A6); // Read the inputs
  uint16_t  odsR  = analogRead(A7);

  panel.writeDec(0,odsL,4);         // Show results in LED&KEY
  panel.writeDec(4,odsR,4);
                                    // Show the graphs in Serial Plotter
  Serial.print(odsL); Serial.print('\t');
  Serial.print(odsR); Serial.print('\n');

  delay(10);                        // Pause between readings
}
