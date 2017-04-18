/**
 * Created by Olavi Kamppari on 1/8/2017.
 */

/**
 *  File: WH_Rover.cpp
 *
 * Wissahickon Rover API for
 *  - VNH2SP30 motor controller
 *  - SHARP GP2Y0A21 Optical Distance Sensors (ODS_x)
 *  - HC_SR04 UltraSound Distance Sensors (US_xx)
 *  - FC-51 InfraRed Collision Detectors (IR_xx)
 *
 * The API for MPU9255 Gyroscope, Accelerometer, and Magnetormeter
 * will be added in a future version
 *
 */

#include <Vnh2sp30.h>
#include <HC_SR04.h>
#include <GP2Y0A21.h>
#include <WH_Rover.h>

#define USCOUNT         6
#define USINITVALUE     9999
#define FILTER_WINDOW   5
#define USIDLETIME      3000
#define LOGINTERVAL     20

//              ENA A   B   PWM   CS    inv
Vnh2sp30  mtrL( A0, 7,  8,  5,    A2,   0);             // Left side straight
Vnh2sp30  mtrR( A1, 4,  9,  6,    A3,   1);             // Right side reversed

//..............Start only US_FF to detect the distance in front
HC_SR04 ultraSound(1 << US_FF);

// Local Variables
int16_t     currentPower, leftMultiplier, rightMultiplier;
int32_t     loggerTime;
uint32_t    US_Prev[USCOUNT];       // Used to detect changes in ultra sound values
uint16_t    US_Changes[USCOUNT];    // Count the changes
int32_t     US_Time[USCOUNT];       // Last time when used

void dataLoggerHeader() {           // TBD

}

void dataLogger() {
    int32_t currentTime = millis();
    if (currentTime - loggerTime > LOGINTERVAL) {
        loggerTime  = currentTime;
                                    // Logging actions TBD
    }
}

void initWH_Rover() {
    currentPower    = 0;            // Range = -1023 .. 1023
    leftMultiplier  = 100;          // Range = -100 .. 100
    rightMultiplier = 100;          // Range = -100 .. 100
    loggerTime      = millis();
    dataLoggerHeader();

    for (int i=0;i<USCOUNT;i++) {
        US_Prev[i]      = USINITVALUE;
        US_Changes[i]   = 0;
        US_Time[i]      = 0;
    }
    for (int i=0;i<8;i++) {         // Setup IR pins starting from IR_LF
        pinMode(IR_LF + i, INPUT_PULLUP);
    }
}

void runMotors(int16_t leftPower, int16_t rightPower) {
    mtrL.run(leftPower);
    mtrR.run(rightPower);
    dataLogger();
}

int32_t interpolate(int32_t x, int32_t dx, int32_t dy, int16_t y0) {
    if (dx == 0) {
        return y0 + dy;
    } else {
        return y0 + dy * x / dx;
    }
}

void rampPower( int16_t targetPower,    int16_t rampDuration,
                int16_t leftTarget,     int16_t rightTarget) {
    int32_t startTime   = millis();
    int32_t deltaTime;
    int32_t deltaPower  = targetPower   - currentPower;
    int32_t deltaLeft   = leftTarget    - leftMultiplier;
    int32_t deltaRight  = rightTarget   - rightMultiplier;
    int32_t rampPower, rampLeft, rampRight, leftPower, rightPower;
    do {
        deltaTime   = millis() - startTime;
        rampPower   = interpolate(deltaTime, rampDuration, deltaPower,  currentPower);
        rampLeft    = interpolate(deltaTime, rampDuration, deltaLeft,   leftMultiplier);
        rampRight   = interpolate(deltaTime, rampDuration, deltaRight,  rightMultiplier);
        runMotors(rampPower * rampLeft / 100, rampPower * rampRight / 100);
    } while (deltaTime < rampDuration);

    currentPower    = targetPower;
    leftMultiplier  = leftTarget;
    rightMultiplier = rightTarget; 
}

void moveForward(int16_t targetPower, int32_t rampDuration) {
    rampPower(targetPower, rampDuration, 100, 100);
}

void moveBackward(int16_t targetPower, int32_t rampDuration) {
    rampPower(-targetPower, rampDuration, 100, 100);
}

void turnLeft(int16_t leftSpeed, int32_t turnDuration) {
    if (currentPower == 0) {
        rampPower(1023 * (int32_t) leftSpeed / 100, turnDuration, -100, 100);
    } else {
        rampPower(currentPower, turnDuration, leftSpeed, 100);
    }
}

void turnRight(int16_t rightSpeed, int32_t turnDuration) {
    if (currentPower == 0) {
        rampPower(1023 * (int32_t) rightSpeed / 100, turnDuration, 100, -100);
    } else {
        rampPower(currentPower, turnDuration, 100, rightSpeed);
    }
}

void brakeToZero(int32_t brakeDuration) {
    rampPower(0,brakeDuration,leftMultiplier,rightMultiplier);
}

void moveUntil(bool condition(), int32_t maxDuration) {
    if (maxDuration < 1) {
        while (!condition()) {         // No time limit
            dataLogger();
        }
    } else {
        int32_t startTime = millis();
        int32_t deltaTime;
        do {
            deltaTime = millis() - startTime;
            dataLogger();
        } while (!condition() && (deltaTime < maxDuration));
    }
}

void executeWhile(void actionLoop(bool), bool condition(void), int32_t maxDuration) {
    int32_t     startTime = millis();
    int32_t     actionTime;
    int32_t     deltaTime;

    actionLoop(true);
    while (condition()) {
        actionTime      = millis();
        while (actionTime == millis()) {
                                    // Wait for the next ms
        }
        actionLoop(false);
        dataLogger();
        if (maxDuration > 0) {      // Check explicit time limit
            deltaTime   = millis() - startTime;
            if (deltaTime >= maxDuration) return;
        }
    }
}

void stopMotors() {
    currentPower = 0;
    mtrL.stop();                        // Stop both motors
    mtrR.stop();
}

void stopAll() {
    stopMotors();
    while(1);                           // Stop looping    
}

int16_t getODS(ODSPin pinNr) {
    return GP2Y0A21distance(analogRead(pinNr));
}

void enableUS(USChannel channelNr) {
    ultraSound.selectSensors(ultraSound.selectionMask() | (1 << channelNr));
}

void disableUS(USChannel channelNr) {
    ultraSound.selectSensors(ultraSound.selectionMask() & ~(1 << channelNr));
}

int16_t getUS(USChannel channelNr) {
    return ultraSound.readSensor(channelNr);
}

bool getIR(IRPin pinNr) {
    return !digitalRead(pinNr);
}