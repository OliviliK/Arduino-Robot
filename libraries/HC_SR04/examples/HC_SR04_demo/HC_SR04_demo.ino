/**
 * Created by Olavi Kamppari on 7/7/2016.
 */

/**
 * Demonstrate the functionality of HC_SR04 Ultrasound Sensors
 *  - 6 sensors, numbered 0 .. 5
 *  - Fixed connections to Port K in Arduino Mega 2560
 *  - Echo signals to pins A8 .. A13
 *  - Trig signals from pins 43 .. 48
 *  - Sensor driver is connected with a mask, where 
 *    sensor n bit is 1<<n
 */

/** 
 *  User instructions for LED&KEY
 *  - S1 to enable all channels
 *  - S2 to disable all channels
 *  - S3 - S8 to toggle single channel enabling
 *  - LEDS show enabled channels
 *  - Display, show the distance in mm for the lowest selected channel
 *    Show the channel name and pin number before the sensor reading
 *    
 *  Use Serial Monitor and Plotter in Arduino tools to visualize the values  
 */

#include <HC_SR04.h>
#include <TM1638.h>

char* ussName[]  = {"FL43","FF44","Fr45","Br46","BB47","BL48"};

TM1638  panel(37,36,35);          // Pin order: STB, CLK, DIO
HC_SR04 uss;                      // Initialize an ultrasonic sensor array

uint8_t btnState, btnPrev;        // Panel button state
uint8_t sensorMask;               // Sensor channel selection mask

void setup() {
  Serial.begin(230400);           // Start Serial Monitor with 9600 bd

  Serial.println("HC_Sr04 demo");
  
  panel.allOff();                 // Turn all LEDs off

  btnPrev     = panel.getButtons();
  sensorMask  = uss.selectionMask();
}

uint32_t bitChange(uint32_t bitNumber) {
  return btnState & ~btnPrev & (1 << bitNumber);
}

void loop() {
  btnState  = panel.getButtons();
  if (btnState != btnPrev) {
    if (bitChange(7)) {           // S1
      sensorMask = 0x3F;
    } else if (bitChange(6)) {    // S2
      sensorMask = 0;
    } else {                      // S3 .. S8
      sensorMask ^= btnState & ~btnPrev;
    }
    btnPrev = btnState;
    panel.setLEDs(sensorMask);
    uss.selectSensors(sensorMask);
  }
  if (sensorMask == 0) {
      panel.writeText(0," HC_Sr04");
  } else {
    for (uint32_t chNr=0;chNr<6;chNr++) {
      if (sensorMask & (1 << chNr)) {
        panel.writeText(0,ussName[chNr]);
        panel.writeDec(4,uss.readSensor(chNr),4);
        break;     
      }
    }
  }
  if (sensorMask) {
    for (uint32_t chNr=0;chNr<6;chNr++) {
      Serial.print((chNr == 0)?"\n":"\t");
      Serial.print(uss.readSensor(chNr));
    }
  }
  delay(20);
}

