/**
 *  Created by Olavi Kamppari on 2/14/2017.
 */

/**
 *  File: ProcSimulator.cpp
 *
 *  Process Simulator for process control development with first order physics engine.
 *
 *  The simulation has three stages:
 *  1) Delay Line for the Actuator Movments - usage is rare
 *  2) First order movement, where
 *      - Force depends on the delta between current position and actuator position
 *      - Acceleration depends on the force and mass
 *      - Speed cumulates the acceleration values
 *      - Position cumulates the speed values
 *  3) Delay Line between postion and transmitter value - usage is rare
 *
 *  The simulation parameters are:
 *      Actuator Lag in units of execution cycles
 *          Long lag makes the tuning of controller difficult
 *      Actuator Gain in percents to translate the actuator position into force
 *          Negative Gain reverses the response direction
 *      Mass to slow down the movments
 *      Friction in percents to slow down the speed when acceleration is close to zero
 *          Zero friction make the tuning of control difficult
 *      Process Lag in units of execution cycles
 *          Long lag makes the tuning of controller difficult
 *      Control Value (actuator position) default value and range
 *      Process Value (transmitter) default value and range
 *
 *  During the initialization a transmitter offset is calculated to keep the acceleration
 *  and speed at zero with the initialization values.  The ratio between control value range
 *  and process value range together with actuator gain is used to calculate the ratio
 *  between position and process value.
 *
 *  The simulator stimulus is given with
 *      - SetCV method, which sets the control value
 *      - SetLoad method, which sets an extra positive or negative forc
 *  The simulation is executed with
 *      - PV method, which returns the process value.
 *
 */

#include <ProcSimulator.h>

int16_t clamp(int16_t value, int16_t minV, int16_t maxV) {
    if (value < minV) return minV;
    if (value > maxV) return maxV;
    return value;
}

int32_t clamp(int32_t value, int32_t minV, int32_t maxV) {
    if (value < minV) return minV;
    if (value > maxV) return maxV;
    return value;
}

void fixRange(int16_t *initV, int16_t *minV, int16_t *maxV) {
    if (*minV > *maxV) {
        int16_t t = *minV;
        *minV   = *maxV;
        *maxV   = t;
    }
    if (*minV > *initV) {
        *initV  = *minV;
    } else if (*maxV < *initV) {
        *initV  = *maxV;
    }
}

void fixRange(int32_t *initV, int32_t *minV, int32_t *maxV) {
    if (*minV > *maxV) {
        int32_t t = *minV;
        *minV   = *maxV;
        *maxV   = t;
    }
    if (*minV > *initV) {
        *initV  = *minV;
    } else if (*maxV < *initV) {
        *initV  = *maxV;
    }
}

void printF3(int32_t valueF3) {
    if (valueF3 == 0) {
        Serial.print("0.000");
        return;
    }
    if (valueF3 < 0) {
        valueF3 = - valueF3;
        Serial.print("-");
    }
    Serial.print(valueF3 / 1000L);
    uint32_t decimals = valueF3 % 1000;
    Serial.print(".");
    if (decimals < 100) {
        Serial.print("0");
    }
    if (decimals < 10) {
        Serial.print("0");
    }
    Serial.print(decimals);
}

int32_t multF3(int32_t a, int32_t b) {
    if (a == 0) return 0;
    if (b == 0) return 0;
    if (abs(a) < INT32_MAX / abs(b)) {
        return (a * b) / 1000L;                   // No overflow
    }
    int64_t x = (int64_t)a * (int64_t)b;
    return x / 1000LL;
}

ProcSimulator::ProcSimulator(uint16_t actLag, int16_t actGainPct,
                    uint16_t mass, uint16_t friction, uint16_t procLag,
                    int16_t initCV, int16_t minCV, int16_t maxCV,
                    int16_t initPV, int16_t minPV, int16_t maxPV) {
    this->actLag        = actLag;
    this->actGainF3     = actGainPct * 10L;
    this->mass          = mass;
    this->frictionInit  = friction;
    this->procLag       = procLag;
    this->initCV        = initCV;
    this->minCV         = minCV;
    this->maxCV         = maxCV;
    this->initPVF3      = initPV * 1000L;
    this->minPVF3       = minPV * 1000L;
    this->maxPVF3       = maxPV * 1000L;

    initSimulator();
}

void ProcSimulator::initSimulator() {
    if (mass < 1) mass = 1;                     // Set minimum mass

    fixRange(&initCV,   &minCV,     &maxCV);
    fixRange(&initPVF3, &minPVF3,   &maxPVF3);

    controlValue.setDelay(actLag,initCV);
    processValue.setDelay(procLag,initPVF3 / 1000L);

    uint32_t    rangePVF3   = maxPVF3 - minPVF3;
    uint32_t    rangeCV     = maxCV - minCV;
    procGainF3         = (1000 * (rangePVF3 / rangeCV)) / abs(actGainF3);

    currentSpeedF3          = 0;
    currentAccelerationF3   = 0;
    prevAccelerationF3      = 0;
    currentDistanceF3       = 0;
    currentForceF3          = 0;
    latestCV                = initCV;
    actPositionF3           = latestCV * actGainF3;
    currentPositionF3       = actPositionF3;
    int32_t pvF3            = actPositionF3 * procGainF3 / 1000L;
    baseValueF3             = initPVF3 - pvF3;
    transmitter             = initPVF3 / 1000L;
    loadF3                  = 0;
    frictionCnt             = 0;
}

void ProcSimulator::applyFriction() {
    if (currentAccelerationF3 * prevAccelerationF3 <= 0) {
        frictionCnt = frictionInit;
    }
    if (frictionCnt) {
        frictionCnt--;
        currentSpeedF3    = 99L * currentSpeedF3 / 100L;    // Reduce speed by 1 %
    }
    prevAccelerationF3 = currentAccelerationF3;
}

void ProcSimulator::simulate() {
    currentCV               = controlValue.exchange(latestCV);
    actPositionF3           = actGainF3 * currentCV;
    currentDistanceF3       = actPositionF3 - currentPositionF3;
    currentForceF3          = currentDistanceF3 - loadF3;
    currentAccelerationF3   = currentForceF3 / mass;
    currentSpeedF3         += currentAccelerationF3;
    if (frictionInit) applyFriction();
    currentPositionF3      += currentSpeedF3;
    int32_t pvF3            = baseValueF3 + multF3(procGainF3,currentPositionF3);
    if (pvF3 > maxPVF3) {
        pvF3                = maxPVF3;
        currentSpeedF3      = 0;
    } else if (pvF3 < minPVF3) {
        pvF3                = minPVF3;
        currentSpeedF3      = 0;
    }
    transmitter             = processValue.exchange(pvF3 / 1000L);
}

void ProcSimulator::dumpSetup() {
    Serial.print("\nactLag = ");        Serial.print(actLag);
    Serial.print("\nprocLag = ");       Serial.print(procLag);
    Serial.print("\nactGainPct = ");    printF3(actGainF3);
    Serial.print("\nprocGainPct = ");   printF3(procGainF3);
    Serial.print("\nmass = ");          Serial.print(mass);
    Serial.print("\nfriction = ");      printF3(frictionInit);
    Serial.print("\ninitCV = ");        Serial.print(initCV);
    Serial.print("\nminCV = ");         Serial.print(minCV);
    Serial.print("\nmaxCV = ");         Serial.print(maxCV);
    Serial.print("\ninitPV = ");        printF3(initPVF3);
    Serial.print("\nminPV = ");         printF3(minPVF3);
    Serial.print("\nmaxPV = ");         printF3(maxPVF3);
    Serial.print("\nbaseValue = ");     printF3(baseValueF3);
    Serial.println();
}

void ProcSimulator::dumpCurrent() {
    Serial.print("\ncurrentCV = ");             Serial.print(currentCV);
    Serial.print("\ncurrentPosition = ");       printF3(currentPositionF3);
    Serial.print("\ncurrentSpeed = ");          printF3(currentSpeedF3);
    Serial.print("\ncurrentAcceleration = ");   printF3(currentAccelerationF3);
    Serial.print("\ncurrentDistance = ");       printF3(currentDistanceF3);
    Serial.print("\ncurrentForce = ");          printF3(currentForceF3);
    Serial.print("\nlatestCV = ");              Serial.print(latestCV);
    Serial.print("\nactPosition = ");           printF3(actPositionF3);
    Serial.print("\ntransmitter = ");           Serial.print(transmitter);
    Serial.println();
}

void ProcSimulator::SetCV(int16_t CV) {
//    dumpSetup();
    latestCV    = clamp(CV,minCV,maxCV);
}

void ProcSimulator::SetLoad(int16_t newLoad) {
    loadF3 = newLoad;
    loadF3 *= 1000L;
}

int16_t ProcSimulator::CV() {
    return    latestCV;
}

int16_t ProcSimulator::PV() {
    simulate();
//    dumpCurrent();
    return    transmitter;
}

int16_t ProcSimulator::ActGainPct()     {return actGainF3/10L;}
int16_t ProcSimulator::MinCV()          {return minCV;}
int16_t ProcSimulator::MaxCV()          {return maxCV;}
int32_t ProcSimulator::Distance()       {return currentDistanceF3 / 1000L;}
int32_t ProcSimulator::Force()          {return currentForceF3 / 1000L;}
int32_t ProcSimulator::Acceleration()   {return currentAccelerationF3 / 1000L;}
int32_t ProcSimulator::Speed()          {return currentSpeedF3 / 1000L;}
int32_t ProcSimulator::Position()       {return currentPositionF3 / 1000L;}
