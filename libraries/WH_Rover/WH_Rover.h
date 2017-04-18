#ifndef WH_ROVER_H_H
#define WH_ROVER_H_H

#include <Arduino.h>

typedef enum ODSPins {
    ODS_L = A6,
    ODS_R = A7
} ODSPin;

typedef enum USChannels {
    US_FL = 0,
    US_FF = 1,
    US_FR = 2,
    US_BR = 3,
    US_BB = 4,
    US_BL = 5
} USChannel;

typedef enum IRPins {
    IR_LF = 18,
    IR_FL = 19,
    IR_FR = 20,
    IR_RF = 21,
    IR_RB = 22,
    IR_BR = 23,
    IR_BL = 24,
    IR_LB = 25
} IRPin;

void initWH_Rover();

void    runMotors(int16_t leftPower, int16_t rightPower);
void    moveForward(int16_t targetPower, int32_t rampDuration);
void    moveBackward(int16_t targetPower, int32_t rampDuration);
void    turnLeft(int16_t leftSpeed, int32_t turnDuration);
void    turnRight(int16_t rightSpeed, int32_t turnDuration);
void    brakeToZero(int32_t brakeDuration);
void    moveUntil(bool condition(void), int32_t maxDuration);
void    executeWhile(void actionLoop(bool), bool condition(void), int32_t maxDuration);
void    stopMotors();
void    stopAll();

int16_t getODS(ODSPin pinNr);
void    enableUS(USChannel channelNr);
void    disableUS(USChannel channelNr);
int16_t getUS(USChannel channelNr);
bool    getIR(IRPin pinNr);

#endif