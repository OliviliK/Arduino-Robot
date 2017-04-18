/**
 * Created by Olavi Kamppari on 1/8/2017.
 */

/**
 * Demonstrate the capabilities of Vnh2sp30 dual channel motor controller
 *  - shield on Arduino Mega 2560
 *  - use functions for different move types and predefined conditions
 *    > forward
 *    > turn right
 *    > turn left
 *    > slow down
 *    > turn in place CW
 *    > slow down
 *    > turn in place CCW
 *    > backward
 *    > forward
 *    > move unntil either of the front IR is activated
 *    > follow a target using the tow ODS sensors
 *
 * For the IR sensors use WH_Rover API
 */

#include <WH_Rover.h>
#define  LED     13

bool obstacleDetected() {
    return getIR(IR_FL) || getIR(IR_FR);
}

bool targetInRange() {
    int16_t odsL    getODS(ODS_L);
    int16_t odsR    getODS(ODS_R);
    if (odsL < 100) return false;
    if (odsL > 500) return false;
    if (odsR < 100) return false;
    if (odsR > 500) return false;
    return true;
}

void followTarget(bool startLoop) {
    int16_t odsL = getODS(ODS_L);
    int16_t odsR = getODS(ODS_R);
    static int16_t target;

    if (startLoop) {
       if (odsL > odsR) {
          if (odsL < 800) {
              target = (odsL + odsR)/2;
          } else {
              target = odsR;
          }
       } else {
          if (odsR < 800) {
              target = (odsL + odsR)/2;
          } else {
              target = odsL;
          }
       }
    }
    runMotors(100*(target - odsL), 100*(target - odsR));
}

void setup() {
    initWH_Rover();
    pinMode(LED,OUTPUT);
    digitalWrite(LED,HIGH);
}

void loop() {
    moveForward(1023,1000);     // Accelerate to full speed (1023) in 1 sec
    moveForward(600,100);       // Slow to normal speed in 0.1 s
    turnLeft(50,500);           // Start slowly to turn left for 0.5 s
    turnRight(0,500);           // Stop the right wheels for 0.5 s
    brakeToZero(1000);          // SlowDown to halt in 1 sec
    turnRight(100,1000);        // Turn CW in 1 sec for full speed
    brakeToZero(1000);          // SlowDown to halt in 1 sec
    turnLeft(50,1000);          // Turn CCW in 1 sec for half speed
    stopMotors();               // Immediate stop without slow down
    moveBackward(900,2000);     // In 2 s start to go backwards
    moveForward(700,1000);      // Change direction again in 1 s
    
                                // Wait for detected obstacle,
                                //  but not more than 5 s
                                //  Keep moving at power 700
    moveUntil(obstacleDetected,5000);
    
                                // Follow the target for 1 minute
    executeWhile(followTarget,targetInRange,60*1000);
    digitalWrite(LED,LOW);
    stopAll();                  // Once obstacle is detected or timeout
                                //  stop both motors and looping
}
