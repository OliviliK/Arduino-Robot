/**
 * Created by Olavi Kamppari on 3/19/2016.
 */

/**
 * Demonstrate the capabilities of Vnh2sp30 dual channel motor controller
 *  - shield on Arduino Mega 2560
 *  - repeat for 10 seconds macrosteps lasting 500 ms each
 *    > forward
 *    > turn right
 *    > turn left
 *    > turn in place CW
 *    > turn in place CCW
 *    > backward
 */

#include <Vnh2sp30.h>
 
//              ENA A   B   PWM   CS    inv
Vnh2sp30  mtrL( A0, 7,  8,  5,    A2,   0);             // Left side straight
Vnh2sp30  mtrR( A1, 4,  9,  6,    A3,   1);             // Right side reversed

void setup() {                          // No setup required
}

void loop() {
  if (millis() > 10000) {               // Stop after 10 seconds
    mtrL.stop();                        // Stop both motors
    mtrR.stop();
    while(1);                           // Stop looping
  }

  mtrL.run(1023);                      // Full speed forward
  mtrR.run(1023);
  delay(500);

  mtrL.run(500);                      // Half speed turn right
  mtrR.run(0);
  delay(500);

  mtrL.run(0);                        // Half speed turn left
  mtrR.run(500);
  delay(500);

  mtrL.run( 500);                      // Turn in place CW
  mtrR.run(-500);
  delay(500);

  mtrL.run(-500);                      // Turn in place CCW
  mtrR.run( 500);
  delay(500);

  mtrL.run(-800);                      // Semi fast backwards
  mtrR.run(-800);
  delay(500);
  
}
