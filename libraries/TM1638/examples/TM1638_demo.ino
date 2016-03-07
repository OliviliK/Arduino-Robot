/**
 * Created by Olavi Kamppari on 2/23/2016.
 */

/**
 * Demonstrate the capabilities of TM1638 LED&KEY board
 *  - to drive the 8 LEDs as a pattern or individually
 *  - to update the 8 seven segment displays as a text or individually
 *    > the text can be a string, decimal number, or hex number
 *  - to read the 8 keys (buttons) as a pattern or individually
 *  - to detect key presses (rising edges)
 *  - to control the panel brightness
 *  - to turn the panel on or off
 *  
 *  Special notes
 *   LED1 on left is stored in bit 7
 *   LED8 on right is stored in bit 0
 *   S1 key on left is stored in bit 7
 *   S8 key on right is stored in bit 0
 */
 
#include "TM1638.h"

TM1638  panel(44,42,40);      // Pin order: STB, CLK, DIO

uint8_t asc     = 0;          // Scan through ASCII codes
char    txt[] = "Xo.";        // Show the ASCII code in a string
uint8_t tCase = 1;            // 8 API test cases

void setup() {
  panel.allOff();
  panel.setLEDs(0xC9);        // Predetermined LED pattern
  panel.setLED(6,1);
  
  panel.setSegments(0,0x01);  // Demonstrate segment codes
  panel.setSegments(1,0x02);
  panel.setSegments(2,0x04);
  panel.setSegments(3,0x08);
  panel.setSegments(4,0x10);
  panel.setSegments(5,0x20);
  panel.setSegments(6,0x40);
  panel.setSegments(7,0x80);
}

void loop() {
  panel.getButtons();                   // To detect key presses
  
  synch(10);
  if (panel.isKeyPressed()) {
    if (panel.isKey(1)) asc  += 10;
    if (panel.isKey(2)) asc  -= 10;
    if (panel.isKey(3)) asc  += 1;
    if (panel.isKey(4)) asc  -= 1;
    if (panel.isKey(5)) {
      tCase +=1;
      if (tCase>8) tCase = 1;
    }
    if (panel.isKey(6)) {
      tCase -=1;
      if (tCase>8) tCase = 8;  
    }
    if (panel.isKey(7)) panel.setDisplayOff();
    if (panel.isKey(8)) panel.setDisplayOn();

    if (panel.isKey(5) || panel.isKey(6)) {
      panel.setLEDs(0);
      panel.setLED(tCase,1);
      switch (tCase) {
        case 1: panel.setBrightness(0); break;
        case 2: panel.setBrightness(1); break;
        case 3: panel.setBrightness(3); break;
        case 4: panel.setBrightness(5); break;
        case 5: panel.setBrightness(7); break;
        case 6: panel.setBrightness(8); break;
        case 7: panel.allOn(); break;
        case 8: panel.allOff(); break;
      }
    } else {
      txt[0]  = asc;
      panel.writeDec(0,asc,4);      // Show ASCII code in decimal
      panel.writeHex(4,asc,2);      // Show ASCII code in HEX
      panel.writeText(6,txt,2);     // Show ASCII character
    }
  }
}

void synch(uint32_t ms) {                 // General purpose synchronization
  uint32_t  now   = millis();             // Used instead of a delay function
  uint32_t  delta = ms - (now % ms);      // Calculate time for the "next" milliseconds
  while ((millis() - now) < delta);
}


