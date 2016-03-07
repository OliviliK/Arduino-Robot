/**
 *  Created by Olavi Kamppari on 2/18/2016.
 */

/**
 *  Titanmec (www.titanmec.com) TM1638 is an IC dedicated to LED control and and
 *  has a scanner for keypad.
 *  This interface is for LED&KEY board that has
 *    8 red LEDs
 *    8 7-segment LED displays for numbers and rough text
 *    8 buttons
 *
 *  A typical LED&KEY board is 
 *  http://www.aliexpress.com/item/Key-Display-For-AVR-Arduino-8-Bit-Digital-LED-Tube-8-Bit-TM1638-Module-new/32440528623.html
 */

#include "TM1638.h"
#include "TM1638Font.h"

/*
 *  TM1638 Communication protocol
 *    1. Begin of Frame (STB pin to LOW)
 *    2. Command or Address (8 bits)
 *    3  From 0 to 16 data bytes
 *    4. End of Frame (STB pin to HIGH)
 *
 *  Bits in command byte
 *    B7 and B6
 *      00  N/A
 *      01  Write LEDs/Displays or Read Buttons
 *      10  Control LED/Display brightness
 *      11  Set data transfer source or destination address
 *    
 *    Data transfer commands
 *      0x40  Write a single byte
 *      0x42  Read a single byte (not used)
 *      0x44  Write 16 bytes, used only in allOn and allOff functions
 *      0x46  Read 4 bytes to scan the button keys
 *
 *    Display brightness
 *      0x80 .. 0x87  Display if off
 *      0x88 .. 0x8F  Display from dim to bright
 *
 *    Set address
 *      0xC0 .. 0xCF  Adresses from 0 to F
 *                    - even address for a display
 *                    - odd address for a LED
 *
 *  Communication cases
 *    1.  Brightness
 *        Frame 1: 0x80 .. 0x8F
 *
 *    2.  Set all On or Off
 *        Frame 1: 0x44
 *        Frame 2: 0xC0 followed by 16 0x00 or 0xFF
 *
 *    3.  Write LEDs and/or Displays
 *        Frame 1: 0x40
 *        Frame 2: 0xCx followed by a byte
 *        Frame 3: 0xCx followed by a byte
 *        :
 *        Frame N: 0xCx followed by a byte
 *
 *    4.  Scan key buttons
 *        Frame 1: 0x46
 *        Frame 2: 0xC0 switching to receive 4 bytes
 */

#define   WRITEALL  0x40
#define   WRITE1    0x44
#define   READ4     0x46
#define   BRIGHNESS 0x80
#define   BASEADDR  0xC0

#define   BADFONT   0x49
#define   DECPOINT  0x80
#define   NEGSIGN   0x40
#define   ZEROBASE  0x10
#define   HEXDELTA  0x07

/*
    Segment Values

        *** 01 ***
       *         *
       *         *
      20        02
      *         *
      *         *
      *** 40 ***
     *         *
     *         *
    10        04
    *         *
    *         *
    *** 08 ***    80

    For example "5." = 1+20+40+4+8+80 = 0xED
*/

/*
    Special Combound Cases

    1. Decimal point and normal point
        If a point is following a character, it is shown together in a single digit
        This is not done, if the preceding character already has the DECPOINT segment

    2. Negative sign
        If a negative number starts with 1, the sign and number can be combound together
        For example numbers from -100 to +100 can be shown in 3 digits.
        Number 1 and sign are not combound if there is space available.

*/

// Create an TM1638 class ------------------------------------------------------------------------

TM1638::TM1638 (const uint8_t stbPin, const uint8_t clkPin, const uint8_t dioPin) {
  _stbPin   = stbPin;
  _clkPin   = clkPin;
  _dioPin   = dioPin;

  pinMode(_stbPin, OUTPUT);
  pinMode(_clkPin, OUTPUT);
  pinMode(_dioPin, OUTPUT);

  digitalWrite(_stbPin, HIGH);
  digitalWrite(_clkPin, HIGH);
  digitalWrite(_dioPin, HIGH);
  
  allOn();
  setBrightness(7);
  getButtons();
}

// public methods ---------------------------------------------------------------------------------

void    TM1638::setBrightness(uint8_t brightnessLevel){
  brightness  = (brightnessLevel>7)?7:brightnessLevel;
  setDisplayOn();
}

void    TM1638::allOn() {
  startFrame();
  sendByte(WRITEALL);
  for (int8_t i=0; i<16; i++) sendByte(0xff);
  endFrame();
}

void    TM1638::allOff() {
  startFrame();
  sendByte(WRITEALL);
  for (uint8_t i=0; i<16; i++) sendByte(0x00);
  endFrame();
}

void    TM1638::setDisplayOn(){
  sendCommand(BRIGHNESS  + 8 + brightness);
}

void    TM1638::setDisplayOff(){
  sendCommand(BRIGHNESS);
}

void    TM1638::setLEDs(uint8_t pattern){
  sendCommand(WRITE1);
  for (uint8_t i=0; i<8; i++) {
    startFrame();
    sendByte(BASEADDR + 2*i + 1);
    sendByte((pattern & 0x80)? 0xFF: 0x00);
    pattern   <<= 1;
    endFrame();
 }
}

void    TM1638::setLED(uint8_t led, uint8_t value) {
  if ((led <1) || (led > 8)) return;
  sendCommand(WRITE1);
  startFrame();
  sendByte(BASEADDR + 2*(led-1) + 1);
  sendByte(value? 0xFF: 0x00);
  endFrame();
}

void    TM1638::setSegments(uint8_t firstDigit, uint8_t pattern){
  if (firstDigit >= 8) return;
  sendCommand(WRITE1);
  startFrame();
  sendByte(BASEADDR + 2*firstDigit);
  sendByte(pattern);
  endFrame();
}

void    TM1638::writeText(uint8_t firstDigit, char* txt, uint8_t digitCount){
  uint8_t fontNr, pattern;
  for (uint8_t i=firstDigit; i<8; i++) {
    if (digitCount == 0) return;
    if (*txt == 0) return;
    if ((*txt < 0x20) || (*txt > 0x7E)) {
      pattern = BADFONT;
    } else {
      fontNr  = *txt - 0x20;
      pattern = TM1638FONT[fontNr];
      if (txt[1] == '.') {
        if ((pattern & DECPOINT) == 0) {  // Combine only if effective
          pattern |= DECPOINT;            // Combound point with character
          txt++;                          // Skip the point
        }
      }
    }
    setSegments(i,pattern);
    txt++;
    digitCount--;
  }
}

void    TM1638::writeDec(uint8_t firstDigit, int32_t val, uint8_t digitCount){
  writeDec(firstDigit,val,digitCount,0);
}

void    TM1638::writeDec(uint8_t firstDigit, int32_t val, uint8_t digitCount, uint8_t decimals){
  uint8_t   negValue  = (val<0);
  if (digitCount == 0) return;          // Skip if nothing to be written
  if (negValue) val = -val;             // Do all computations with positive numbers
  if (firstDigit + digitCount > 8) {    // Check if there is space for the last digit
    for (uint8_t i=firstDigit; i<8; i++) {
      setSegments(i,BADFONT);
    }
  } else {
    uint8_t currentDigit  = firstDigit + digitCount -1;
    uint8_t decimalPlace  = currentDigit - decimals;
    uint8_t digitValue    = val % 10;
    uint8_t pattern       = TM1638FONT[ZEROBASE + digitValue];

    setSegments(currentDigit, pattern);   // Show the last digit

    while (currentDigit > firstDigit) {   // Get the preceding digits
      val   /= 10;
      if (val == 0) break;
      currentDigit--;
      digitValue    = val % 10;
      pattern       = TM1638FONT[ZEROBASE + digitValue];
      if (currentDigit == decimalPlace) pattern   |= DECPOINT;
      if (val < 10) {
        if (negValue && (digitValue == 1) && (currentDigit == firstDigit)) {
          pattern   |= NEGSIGN;           // Combine negative sign with 1
        }  
      }
      setSegments(currentDigit, pattern); // Show the other digits
    }
    if (currentDigit == firstDigit) return; // All spaces are used
    if (negValue) {
      currentDigit--;
      setSegments(currentDigit, NEGSIGN);
    }
    while (currentDigit > firstDigit) {   // Add preceding spaces
      currentDigit--;
      setSegments(currentDigit, 0);
    }
  }
}

void    TM1638::writeHex(uint8_t firstDigit, uint32_t val, uint8_t digitCount){
  if (digitCount == 0) return;          // Skip if nothing to be written
  if (firstDigit + digitCount > 8) {    // Check if there is space for the last digit
    for (uint8_t i=firstDigit; i<8; i++) {
      setSegments(i,BADFONT);
    }
  } else {
    uint8_t currentDigit  = firstDigit + digitCount;
    uint8_t digitValue,pattern;
    char    hexLetter;

    do {
      currentDigit--;
      digitValue  = val & 0x0f;
      val         >>= 4;
      if (digitValue <10) {
        pattern   = TM1638FONT[ZEROBASE + digitValue];
      } else {
        switch (digitValue) {
          case 10:  hexLetter = 'A'; break;
          case 11:  hexLetter = 'b'; break;
          case 12:  hexLetter = 'C'; break;
          case 13:  hexLetter = 'd'; break;
          case 14:  hexLetter = 'E'; break;
          case 15:  hexLetter = 'F'; break;
        }
        pattern   = TM1638FONT[(uint8_t)hexLetter - 0x20];
      }
      setSegments(currentDigit, pattern);
    } while (currentDigit > firstDigit);
  }
}

uint8_t TM1638::getButtons(){
  lastRead  = millis();
  buttons   = scanKeys();
  edges     = buttons & ~buttons_1;
  buttons_1 = buttons;
  return buttons;
}

uint8_t TM1638::getButton(uint8_t button){
  if ((button < 1) || (button > 8)) return 0;
  
  if ((millis() - lastRead) > 10) {
    getButtons();
  }
  return (buttons & (0x80 >> (button-1))) > 0;
}

uint8_t TM1638::isKeyPressed(){
  return edges > 0;
}

uint8_t TM1638::isKey(uint8_t key){
  if ((key < 1) || (key > 8)) return 0;
  return (edges & (0x80 >> (key-1))) > 0;
}

// protected methods ------------------------------------------------------------------------------

void    TM1638::sendByte(byte data) {
  for (uint8_t i = 0; i < 8; i++) {
    digitalWrite(_clkPin, LOW);
    digitalWrite(_dioPin, (data & 1) ? HIGH : LOW);
    data >>= 1;
    digitalWrite(_clkPin, HIGH);
  }
}

void    TM1638::sendCommand(uint8_t command){
  startFrame();
  sendByte(command);
  endFrame();
}

uint8_t TM1638::receiveByte(){
  uint8_t data = 0;
  for (int8_t i = 0; i < 8; i++) {
    data <<= 1;                       // Read bits in order from 7 to 0
    digitalWrite(_clkPin, LOW);
    if (digitalRead(_dioPin)) data |= 0x01;
    digitalWrite(_clkPin, HIGH);        
  }
  return data & 0B10001000;           // Keep only bits 7 and 3 

}

uint8_t TM1638::scanKeys() {
  uint8_t byte1;                      // For Key sets 1 & 2, B0 = S1, B4  = S2
  uint8_t byte2;                      // For Key sets 3 & 4, B0 = S3, B4  = S4
  uint8_t byte3;                      // For Key sets 5 & 6, B0 = S5, B4  = S6
  uint8_t byte4;                      // For Key sets 7 & 8, B0 = S7, B4  = S8
                                      // Bits B1, B2, B5, and B6 are for extra keys
                                      // Bits B3 and B7 are not used

  startFrame();                       // Keep strobe line down during reading
  sendByte(READ4);
  pinMode(_dioPin, INPUT);            // Change to data input

  byte1 = receiveByte();
  byte2 = receiveByte();
  byte3 = receiveByte();
  byte4 = receiveByte();

  pinMode(_dioPin, OUTPUT);           // Change back to data output
  endFrame();
                                      // Move the bits into right positions
  return  byte1 | (byte2 >> 1) | (byte3 >> 2) | (byte4 >> 3);
}

void  TM1638::startFrame() {
  digitalWrite(_stbPin,LOW);
}

void  TM1638::endFrame() {
  digitalWrite(_stbPin,HIGH);
}
