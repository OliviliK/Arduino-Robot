/**
 *  Created by Olavi Kamppari on 2/18/2016.
 */

/**
 *  Titanmec (www.titanmec.com) TM1638 is an IC dedicated to LED control and and
 *  has a scanner for keypad.
 *  This interface is for LED&KEY board that has
 *    8 red LEDs
 *    8 7-segment LED display digits for numbers and rough text
 *    8 buttons
 *
 *  A typical LED&KEY board is 
 *  http://www.aliexpress.com/item/Key-Display-For-AVR-Arduino-8-Bit-Digital-LED-Tube-8-Bit-TM1638-Module-new/32440528623.html
 */

#include "Arduino.h"
#ifndef TM1638_H
#define TM1638_H

class TM1638 {
  public:
    TM1638  ( const uint8_t stbPin, const uint8_t clkPin, const uint8_t dioPin);
    void      setBrightness(uint8_t brightnessLevel);
    void      allOn();
    void      allOff();
    void      setDisplayOn();
    void      setDisplayOff();
    void      setLEDs(uint8_t pattern);
    void      setLED(uint8_t led, uint8_t value);
    void      setSegments(uint8_t firstDigit, uint8_t pattern);
    void      writeText(uint8_t firstDigit, char* txt, uint8_t digitCount);
    void      writeDec(uint8_t firstDigit, int32_t val, uint8_t digitCount);
    void      writeDec(uint8_t firstDigit, int32_t val, uint8_t digitCount, uint8_t decimals);
    void      writeHex(uint8_t firstDigit, uint32_t val, uint8_t digitCount);
    uint8_t   getButtons();
    uint8_t   getButton(uint8_t button);
    uint8_t   isKeyPressed();
    uint8_t   isKey(uint8_t button);
private:    
    uint8_t   _stbPin;    // Is low during command and data transfers
    uint8_t   _clkPin;    // Triggers the individual bit transfers
    uint8_t   _dioPin;    // Digital Input/Output fro the commands and data
    uint8_t   brightness; // Stored brightness level
    uint8_t   buttons;    // Bit pattern of the buttons
    uint8_t   buttons_1;  // Previous buttons value
    uint8_t   edges;      // Edge detection
    uint32_t  lastRead;   // Time in millis
  protected:
    void      sendByte(uint8_t data);
    void      sendCommand(uint8_t command);
    uint8_t   receiveByte();
    uint8_t   scanKeys();
    void      startFrame();
    void      endFrame();
};
#endif
