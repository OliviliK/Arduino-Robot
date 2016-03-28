/**
 * Created by Olavi Kamppari on 2/23/2016.
 */

/**
 * Demonstrate the basic capabilities of TM1638 LED&KEY board
 */
 
#include <TM1638.h>

TM1638  panel(37,36,35);          // Pin order: STB, CLK, DIO

void setup() {
  Serial.begin(9600);             // Start Serial Monitor with 9600 bd
  
  panel.allOff();                 // Turn all LEDs off
  panel.writeText(2,"1638");      // 2 = display on left, below LED3
                                  // "1638" = text to be written
}

void loop() {
  uint8_t keys = panel.getButtons();  // Store keys in a variable
  
  if (keys == 0B10000001) {       // Key pattern in binary form
                                  // Test if S1 and S8 are pressed
                                  // at the same time
    panel.allOff();               // If yes, turn all LEDs off
  }
  
  if (panel.isKeyPressed()) {     // Check if any Key is pressed
    
    Serial.print("Key pattern = "); // Show pattern on Serial Monitor
    Serial.println(keys,HEX);
    
    if (panel.isKey(1)) {
      panel.setLED(1,HIGH);       // Turn LED1 on (HIGH)
      panel.writeDec(0,0,1);      // In position 0, write 0, with 1 digit
    }
    if (panel.isKey(2)) {
      panel.setLED(2,HIGH);
      panel.writeDec(1,1,1);      // In position 1, write 1, with 1 digit
    }
    if (panel.isKey(3)) {
      panel.setLED(3,HIGH);
      panel.writeDec(2,32,1);     // In position 2, write 2, with 1 digit
                                  // Ignore the 30 before 2
    }
    if (panel.isKey(4)) {
      panel.setLED(4,HIGH);
      panel.writeDec(3,34,2);     // In position 3, write 34, with 2 digits
    }
    if (panel.isKey(5)) {
      panel.setLED(5,HIGH);
      panel.writeText(3,"...",2); // In position 3 and 4, write 2 dots
                                  // Ignore the 3rd dot
    }
    if (panel.isKey(6)) {
      panel.setLED(6,HIGH);
      panel.writeDec(5,123,3,1);  // In position 5, write 12.3
                                  // This requires 3 digits and
                                  // one decimal
    }
    if (panel.isKey(7)) {
      panel.setLED(7,HIGH);      
      panel.writeDec(5,-123,3,1); // In position 5, write -12.3
    }
    if (panel.isKey(8)) {
      panel.setLED(8,HIGH);      
      panel.writeText(5,"---",3); // In position 5, write "---"
    }
  }

  delay(10);                  // Wait 10 ms before next update
}

void synch(uint32_t ms) {                 // General purpose synchronization
  uint32_t  now   = millis();             // Used instead of a delay function
  uint32_t  delta = ms - (now % ms);      // Calculate time for the "next" milliseconds
  while ((millis() - now) < delta);
}
