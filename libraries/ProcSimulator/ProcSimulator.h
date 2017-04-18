#ifndef PROCSIMULATOR_H
#define PROCSIMULATOR_H

#include "DelayLine.h"
#include <Arduino.h>

class ProcSimulator {
public:
    ProcSimulator(uint16_t actLag, int16_t actGainPct, 
                uint16_t mass=1, uint16_t friction=0, uint16_t procLag=0,
                int16_t initCV=0, int16_t minCV=0, int16_t maxCV=100, 
                int16_t initPV=0, int16_t minPV=0, int16_t maxPV=100);

    void SetCV(int16_t CV);
    void SetLoad(int16_t newLoad);
    int16_t CV();
    int16_t PV();
    int16_t ActGainPct();
    int16_t MinCV();
    int16_t MaxCV();
    int32_t Distance();
    int32_t Force();
    int32_t Acceleration();
    int32_t Speed();
    int32_t Position();
private:
    void initSimulator();
    void applyFriction();
    void simulate();
    void dumpSetup();
    void dumpCurrent();
    
    uint16_t    actLag, procLag;
    int32_t     actGainF3, mass;
    int16_t     frictionCnt,frictionInit;
    int16_t     initCV, minCV, maxCV;
    int32_t     initPVF3, minPVF3, maxPVF3;
    DelayLine   controlValue, processValue;
    int32_t     procGainF3;
    int32_t     currentCV;
    int32_t     currentPositionF3;
    int32_t     currentSpeedF3;
    int32_t     currentAccelerationF3;
    int32_t     prevAccelerationF3;
    int32_t     currentDistanceF3;
    int32_t     currentForceF3;
    int32_t     latestCV;
    int32_t     actPositionF3;
    int32_t     baseValueF3;
    int32_t     loadF3;
    int16_t     transmitter;
};

#endif
