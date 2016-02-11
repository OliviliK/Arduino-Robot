/**
 * Created by Olavi Kamppari on 2/9/2016.
 */

/**
 * Demonstrate the simultaneous usage of 6 parallel ultrasonic sensors.
 * Show the results on Excel Data Sheet.
 *
 * Platform: Arduino Mega 2560
 * - Port B Pin Change interrupt,for Echo inputs
 * - Timer 2 Comparator A for trigger delay
 * - Pins 19, 18, 17, 16, 15, and 14 for Trigger outputs
 *
 * The literal porting to a different platform requires good insight in details.
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

#define   TIMERSTART    (1 << CS22) | (1 << CS21) | (0 << CS20)
#define   TIMERSTOP     0
#define   TIMERCTCMODE  (1 << WGM21) | (0 << WGM20)
#define   TIMERINTENA   (1 << OCIE2A)

#define   CHCOUNT       6
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
volatile uint32_t   scanStart, scanDelta; // Measurement of scanning all channels
volatile uint16_t   chNr;                 // Current measurement channel 0..CHCOUNT-1
volatile uint32_t   duration[CHCOUNT];    // Stored measurement results [ms]
volatile uint16_t   valOK   [CHCOUNT];    // Number of times measurment is within limits
volatile uint16_t   minFail [CHCOUNT];    // Number of times where duration is too small
volatile uint16_t   maxFail [CHCOUNT];    // Number of times where duration is too high

//---------------------------------------- Initialization -----------------------------
void setup() {
  doSetup(__FILE__);
                              // Init interrupts
  initPinChangeInterrupts();
  initTriggerDelayTimer();
                              // Init channels and print labels
  Serial.print("ms");
  for (int i = 0; i < CHCOUNT; i++) {
    Serial.print("\tch");
    Serial.print(i);
    duration[i] = 0;
    valOK[i]    = 0;
    minFail[i]  = 0;
    maxFail[i]  = 0;
    pinMode(19 - i, OUTPUT);
  }
  Serial.println();

  state       = start;
  scanStart   = millis();
}

void doSetup(const char *fileName) {
  while (!Serial);            // Wait for the serial port to open
  Serial.begin(230400);       // Initialize Serial communication
  Serial.print(fileName);     // Print sketch identification info
  Serial.println( " " __DATE__ ", " __TIME__ ", " __VERSION__);
}

void initPinChangeInterrupts() {
  PCICR     = 0x01;           // Enable Pin-Change Interrupt for port B bits
  PCMSK0    = 0x7F;           // Enable PB0 - PB6 interrupts
}

void initTriggerDelayTimer() {
  cli();                      // Clear interrupts when setting timer2 (not really required)
  TCCR2A    = TIMERCTCMODE;   // Only the TCT bit is set
  TCCR2B    = TIMERSTOP;
  OCR2A     = 60;             // Extra delay between channels is 60/62500 = 0.96 ms
  TIMSK2    |= TIMERINTENA;   // Timer interrupt enable
  sei();
}

//--------------------------------------------- Main Loop -----------------------------------
void loop() {
  trackScanning();

  if (millis() > 6000) {    // Stop after 6 sec
    printCounts();
    while (1) ;             // Infinite loop
  }
  synch(1);                 // Wait for the next full ms
}

void trackScanning() {
  switch (state) {
    case  start:                            // At start, schedule first channel
      chNr  = 0;
      scheduleTrigger();
      break;
    case  waitRisingEdge:
      if ((micros() - tTrigger) > TIMEOUT) {
        startNextChannel();                 // Ignore the failing channel
      }   
      break;
    case  waitFallingEdge:
      if ((micros() - tRise) > TIMEOUT) {
        startNextChannel();                 // Ignore the missing reply
      }   
      break;
    case done:
      uint32_t nanoMeters;
      uint32_t now = millis();
      scanDelta     = now - scanStart;      // Calculate the total scanning time
      scanStart     = now;
      Serial.print(scanDelta);
      for (int i = 0; i < CHCOUNT; i++) {   // Show all channel measurments (in mm)
        nanoMeters = NMPERMS * duration[i]; // Convert from us duration into nm
        Serial.print('\t');
        Serial.print(nanoMeters / NMINMM);  // Convert nm into mm
      }
      Serial.println();
      state = start;                        // Start next scanning
      break;
  }
}

void scheduleTrigger() {
  state   = waitTrigger;
  TCCR2B  = TIMERSTART;                    // Schedule next trigger
}

void startNextChannel() {
  chNr++;
  if (chNr < CHCOUNT) {
    scheduleTrigger();                    // Continue with next channel if available
  } else {
    state = done;                         // Otherwise mark all channels scanned
  } 
}

void printCounts() {
  Serial.println("\nCh\tOK\tminF\tmaxF"); // Print header
  for (int i=0;i<CHCOUNT;i++) {           // Print results
    Serial.print(i);
    Serial.print('\t');
    Serial.print(valOK[i]);
    Serial.print('\t');
    Serial.print(minFail[i]);
    Serial.print('\t');
    Serial.print(maxFail[i]);
    Serial.print('\n');
  }
}  

void synch(uint32_t ms) {                 // General purpose synchronization
  uint32_t  now   = millis();             // Used instead of a delay function
  uint32_t  delta = ms - (now % ms);      // Calculate time for the "next" milliseconds
  while ((millis() - now) < delta);
}

//-------------------------------------------- Interrupt Routines ---------------------

ISR(TIMER2_COMPA_vect) {        // TIMER 2 COMPARE A INTERRUPT
  TCCR2B = TIMERSTOP;           // Stop the timer
 
  uint8_t   trigPin = 19 - chNr;
  digitalWrite(trigPin, HIGH);  // Create trigger pulse
  delayMicroseconds(4);
  digitalWrite(trigPin, LOW);
  state     = waitRisingEdge;
  tTrigger  = micros();
}

ISR(PCINT0_vect) {              // PORT B PIN CHANGE INTERRUPT (#0)
  if (state == waitRisingEdge) {
    tRise   = micros();         // Record the rising time
    state   = waitFallingEdge;
  } else if (state == waitFallingEdge) {
    tFall   = micros();         // Record the falling time
    dt  = tFall - tRise;        // Calculate echo pulse length in us
    if (dt < MINTIME) {         // Check the length validity
      minFail[chNr]++;
    } else if (dt > MAXTIME) {
      maxFail[chNr]++;
    } else {
      valOK[chNr]++;
      duration[chNr]   = dt;    // Store the length for current channel
    }
    startNextChannel();         // Start the next channel
  }
}

