/**
 *  Created by Olavi Kamppari on 7/7/2016.
 */

/**
 * Support for an array of sonar sensors using HC_SR04 type
 * interface.  Max array size is is restricted to 7 due to the
 * layout of the Mega Sensor Shield (pin 50 is in "wrong" place)
 *
 * Platform: Arduino Mega 2560
 * - Port K Pin Change interrupt,for Echo inputs (pins A8 - A14)
 *    Pin A15 is not used because the corresponding trigger pin
 *    is used for SD_MISO signal and is in "wrong" place.
 * - Timer  2 Comparator A for trigger delay
 * - Pins 43, 44, 45, 46, 47, 48, and  49 for Trigger outputs
 * 
 * This library is used in the Ultrasonic Sensors on Wissahickon Rover.
 * The implementation code is described a blog post at
 * 
 *  http://olliesworkshops.blogspot.com/2016/02/ultrasonic-sensor-array.html?view=classic
 *  
 * * The literal porting to a different platform requires good insight in details.
 * The basic algorithm can be ported to any platform
 * - Measure only one channel at the time
 * - After completing a measurement in one channel, allow extra time
 *   > to avoid detecting echoes from previous channel
 * - Generate a 10 us long triggering pulse
 * - Wait for detection of rising edge in the echo pulse
 * - Wait for detection of falling edge in the echo pulse
 * - The duration of the echo pulse is calculated in us
 * - The duration is transformed into millimeters based on the speed of sound
 *   > 340.3 m/s is same as
 *   > 340.3 mm/ms is same as
 *   > 340.3 um/us is same as
 *   > 340,300 nm/us
 * - The sound travels from the transmitter (T) to a reflective surface
 *   and back to receiver (R).  In other words, every us represents 170,150 nm.
 * - For faster execution, integer calculations are used instead of floating point
 *
 *      distance in mm = 170,150UL * duration in us / 1000,000UL
 *    
 * - The UL represent a 32 bit unsigned long constant (range from 0 to 4,294,967,295)   
 * - The practical maximum distance with the HC-SR04 sensor is 3.7 m.
 *   > With 22 ms duration the product is 170150 * 22000 = 3,743,300,000
 * - The practical minimum distance with the HC-SR04 sensor is 17 mm   
 *   > With 100 us duration, the distance is 17.015 mm
 * - If the rising or falling edge is not detected in 30 ms, the channel is skipped  
 */

#include "HC_SR04.h"

//----------------------------------------------- Constants ----------------------------

/*
 *  Arduino Mega 2560 Timer 2
 *  - 8 bit counter
 *  - Use 256 prescaler, 16,000,000 / 256 = 62,500
 *    > Clock frequency is 62.5 kHz
 *  - Prescaler code is 6 (binary 110) stored in TCCR2B bits CS22, CS21, and CS20
 *  - Prescaler code 0 stops the timer
 *  - Use Waveform Generation Mode 2 for Clear To Completion (CTC) operation
 *    > Mode 2 (binary 010) is stored in TCCR2B bit WGM2 and TCCR2A bits WGM1 and WGM0
 *  - Use Timer/Counter2 Output Compare Match A Interrupt Enable
 *    > The mask is stored in TIMSK2 register bit OCIE2A
 */

                        // See 2560 datasheet section 20.10.2 for TCCR2B
#define   TIMERSTART    (1 << CS22) | (1 << CS21) | (0 << CS20)
#define   TIMERSTOP     0
                        // See 2560 datasheet section 20.10.1 for TCCR2A
                        //  Note: WGM22 (0) is in TCCR2B
                        //  Wave Generation Mode 010 = Clear Timer on Compare
                        //  Note: Only OCR2A is used for CTC
#define   TIMERCTCMODE  (1 << WGM21) | (0 << WGM20)
                        // OCR2B is for channel isolation, OCR2A is for sensor detection
#define   TIMERINTENA   (1 << OCIE2A) | (1 << OCIE2B)

#define   MAX_CHANNEL   7
#define   FILTER_WINDOW 5
#define   NMPERMS       170150UL
#define   NMINMM        1000000UL
#define   MINTIME       100L
#define   MAXTIME       22000L
#define   TIMEOUT       30000L

//---------------------------------------- Enumerations -------------------------------
enum sState {
  start,
  waitTrigger,
  waitRisingEdge,
  waitFallingEdge,
  done
};

//---------------------------------------- Global Variables ---------------------------

volatile sState     state;                // Each channel goes  through all states
volatile uint32_t   tTrigger;             // Channel trigering time
volatile uint32_t   tRise, tFall, dt;     // Measurement of the echo signal width [ms]
volatile uint16_t   chNr;                 // Current measurement channel 0..MAX_CHANNEL-1

volatile uint8_t    busyChannel;
volatile uint8_t    nextSlot[MAX_CHANNEL];
volatile uint16_t   readings[MAX_CHANNEL][FILTER_WINDOW];

static uint8_t      _selectionMask;

//---------------------------------------- Forward References ------------------------
void      startScanning();
void      startNextChannel();
    
//---------------------------------------- Class Initialization ----------------------

HC_SR04::HC_SR04 (uint8_t selectionMask) {
  uint32_t i,j;
  _selectionMask  = selectionMask;
  for (i=0;i<MAX_CHANNEL;i++) {
    pinMode(43 + i, OUTPUT);        // Enable triggers
    nextSlot[i] = 0;
    for (j=0;j<FILTER_WINDOW;j++) {
      readings[i][j] = 0;
    }
  }
  busyChannel = 0xFF;
  startScanning();
}

HC_SR04::HC_SR04 () {
  HC_SR04((1 << (MAX_CHANNEL + 1)) -1);
}

//---------------------------------------------------- Class Methods ---------

void HC_SR04::selectSensors(uint8_t selectionMask) {
  uint32_t i,j;
  _selectionMask  = selectionMask;
  for (i=0;i<MAX_CHANNEL;i++) {
                                // Clear the past readings
    if ((_selectionMask & (1<<i)) == 0) {
      for (j=0;j<FILTER_WINDOW;j++) {
        readings[i][j] = 0;
      }
    }
  }
}

uint8_t HC_SR04::selectionMask() {
  return _selectionMask;
}

uint32_t HC_SR04::readSensor(uint8_t sensorNumber) {
  uint32_t  i,j;
  uint32_t  x,sum,min,max;

  i = sensorNumber;
  if ((_selectionMask & (1 << i)) == 0) return 0;
  busyChannel = i;
  x   = readings[i][0];
  sum = x;
  min = x;
  max = x;
  for (j=1;j<FILTER_WINDOW;j++) {
    x = readings[i][j];
    sum += x;
    if (x < min) min = x;
    if (x > max) max = x;
  }  
  busyChannel = 0xFF;
  if ((micros() - tRise) > TIMEOUT) {           // Missing reply from a sensor
    tRise = micros();                           // Prevent multiple recoveries
    startNextChannel();
  }
                                                // Calculate average duration in us
  uint32_t  aveTime     = (sum - min - max) / (FILTER_WINDOW - 2);
  uint32_t  nanoMeters  = NMPERMS * aveTime;    // Convert from us duration into nm
  uint32_t  milliMeters = nanoMeters / NMINMM;  // Convert from nm into mm
  return milliMeters; 
}

//---------------------------------------- Initialization -----------------------------

void initPinChangeInterrupts() {
  PCICR     |= (1 << 2);      // Enable Pin-Change Interrupt for port K bits
                              // Enable input pins PK0, PK1, .. PK (MAX_CHANNEL-1)
                              // No extra interrupt pins are allowed
  PCMSK2    = 0xFFFF ^ (0xFFFF << MAX_CHANNEL);
}

void initTriggerDelayTimer() {
  cli();                      // Clear interrupts when setting timer2 (not really required)
  TCCR2A    = TIMERCTCMODE;   // Only the TCT bit is set
  OCR2B     = 100;            // Extra delay between channels is 100/62500 = 1.6 ms
  OCR2A     = OCR2B + 40;     // Timeout for missing sensor (40/62500 = 0.64 ms
  TIMSK2    |= TIMERINTENA;   // Timer interrupt enable
  TCCR2B    = TIMERSTART;     // Start Timer 2 for the first reading
  sei();
}

void startScanning() {
  chNr  = 0;
  initPinChangeInterrupts();
  initTriggerDelayTimer();    // This will trigger the first probe
}

//----------------------------------------- Scheduling ----------------------------

void scheduleTrigger() {
  state   = waitTrigger;
  TCCR2B  = TIMERSTART;      // Schedule next trigger
}

void startNextChannel() {
  if (_selectionMask == 0){ // If none selected, keep reading 0 channel
    chNr = 0;
  } else {                  // Find next selected channel
    for (uint8_t i=0;i<MAX_CHANNEL;i++) {
      if (++chNr >= MAX_CHANNEL) chNr = 0;
      if (_selectionMask & (1 << chNr)) break;
    }
  }                                      
  scheduleTrigger();                      // Start the next reading
}

//-------------------------------------------- Interrupt Routines ---------------------

ISR(TIMER2_COMPB_vect) {        // TIMER 2 COMPARE B INTERRUPT TO START MEASUREMENT
  uint8_t   trigPin = 43 + chNr;
  digitalWrite(trigPin, HIGH);  // Create trigger pulse
  delayMicroseconds(4);
  digitalWrite(trigPin, LOW);
  state     = waitRisingEdge;
  tTrigger  = micros();
}

ISR(TIMER2_COMPA_vect) {        // TIMER 2 COMPARE A INTERRUPT TO DETECT TIMEOUT
  TCCR2B = TIMERSTOP;           // Stop the timer
  if (state == waitRisingEdge) {// Failing or missing sensor
    startNextChannel();
  }
}

ISR(PCINT2_vect) {              // PORT K PIN CHANGE INTERRUPT (#2)
  if (state == waitRisingEdge) {
    tRise   = micros();         // Record the rising time
    state   = waitFallingEdge;
  } else if (state == waitFallingEdge) {
    tFall   = micros();         // Record the falling time
    dt  = tFall - tRise;        // Calculate echo pulse length in us
    if ((dt > MINTIME) && (dt < MAXTIME) && (chNr != busyChannel)) {
                                // Open the next slot
      if (++nextSlot[chNr] >= FILTER_WINDOW) nextSlot[chNr] = 0;
                                // Store the latest reading there
      readings[chNr][nextSlot[chNr]]   = dt;
    }
    startNextChannel();         // Start the next channel
  }
}

