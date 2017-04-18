/**
 *  Created by Olavi Kamppari on 2/27/2017.
 */

/**
 *  File: iPID.cpp
 *
 *  Proportional-Integral-Derivative (PID) control is the most common control 
 *  algorithm used in industrial and general control.  The basic principles are
 *   - The Proportioanl term tries to move the current process value towards setpoint
 *   - Te Integral term tries to move the average process value towards setpoint
 *   - The Derivative term tries to resist the process value movements
 *
 *  This library implements PID control without using floating point numbers in order
 *   - To have efficient implementation in MCU without H/W floating point support
 *   - To eliminate the loading of floating point S/W library into a small memory
 *
 *  The logic for the integral term requires floating point type of functionality
 *  to eliminate the digitization jitter.  This is implemented as a fixed point
 *  representation where last 3 digits of a 32 bit integer are interpreted as
 *  decimal part of the value.  For example, 12345 stands for 12.345.
 *
 *  The 16-bit integers have enough dynamic range for the sensor with 10 - 14 bit
 *  resolution and actuators with 8 - 12 bit capabilities.
 *
 *  This PID controller has the following characteristics
 *   - The goal is to cahnge the Control Value (CV) in such a way that
 *     the Process Value (PV) will be equal to the target Setpoint Value (SP)
 *    -The controller has two modes
 *     1) Manual mode, where user or application can change the CV directly
 *        and the SP is tracking the PV to allow bumpless transfer
 *     2) Auto mode, where user or application can change the SP and the
 *        controller is changing the CV
 *   - During initialization, the integral term is set to the CV value to
 *     allow bumpless transfer (this is caled back initialization)
 *   - To enable changes in data flow for the SP and CV values, the implementation
 *     uses pointers to external 16-bit signed integers representing these.
 *   - To allow preprocessing, such as linearization, clamping, and filtering
 *     of the PV, also the PV value is implemented using pointers
 *   - There are tuning parameters for the 3 terms used to calculate the CV
 *     1) pTerm: based on the delta between SP and SP and pFactor tuning parameter
 *     2) iTerm based on the cumulative delta between SP and SP weighted with the
 *        actual time between executions and the iFactor tuning parameter
 *     3) dTerm based on the speed of change in PV value, execution interval, and
 *        dFactor tuning value
 *   - The other tuning parameters are
 *     4) Execution interval in millieseconds
 *     5) Indication if CV and PV are moving in the same or reverse directions
 *
 *  The pFactor is shown in percentages.  For example
 *   100: Implies that a 100x delta between SP and PV will cause 100x delta in CV
 *    50: Implies that a 100x delta between SP and PV will cause 50x delta in CV
 *
 *  The main method in iPID is excute(), which have no parameters.  It returns true
 *  only in case a new CV value is calculated.  This requires that the control is in
 *  auto mode and that the time from the previous calculation is equeal or more than
 *  the execution interval.
 *
 *  The iTerm calculation includes a method to prevent windup, where the iTerm could
 *  wander outside of the CV limits. Without the windup prevention, there would be an
 *  unecessary delay in case where the process returns to a normal state after the
 *  actuator has been in a saturated state.
 *
 *  Compared to industrial controllers, the following functionalities are not implmeneted
 *   - deadband when the actuator direction (controlled by CV) changes
 *   - stiction when the actuator doen't start to move with small CV changes
 *   - SP ramping which is used to prevent the high CV spike caused by the dTerm
 *   - feed-forward when a PV change can be anticipated from external information
 *   - cascade mode where SP is coming from a CV of another controller
 *   - split control where a control action is divided among different controllers
 */

#include <iPID.h>
#include <stdlib.h>

iPID::iPID(   int16_t* ProcessValue, int16_t* ControlValue, int16_t* SetPoint,
            uint16_t pFactorPct, uint16_t iFactor, uint16_t dFactor, 
            uint16_t executeInterval, bool isReverse){
    PVptr   = ProcessValue;
    OPptr   = ControlValue;
    SPptr   = SetPoint;
    if (executeInterval == 0) executeInterval = 100;    // Use default exec rate
    execInterval    = executeInterval;
    isRev           = isReverse;
    isAuto          = false;
    SetCvLimits(0,1023);                                // Set default CV range
    SetTuning(pFactorPct,iFactor,dFactor);
    initPID();                 
}

void    iPID::initPID() {
    lastError   = 0;
    *SPptr      = *PVptr;               // Do SP tracking when in manual

    pTerm       = 0;
    iTerm       = *OPptr;               // Do OP tracking for bumpless transfer
    dTerm       = 0;
          
    if (iTerm > maxOut) iTerm = maxOut;
    if (iTerm < minOut) iTerm = minOut;
    iTermF3 = (int32_t)iTerm * 1000L;
}

void    iPID::SetMode(bool isAutomatic){
    if (isAuto == isAutomatic) return;
    isAuto = isAutomatic;
    if (isAuto) {
        initPID();
    }
}

bool    iPID::Execute(){
    uint32_t    now = millis();
    int16_t     dt  = now - lastExecTime;           // Delta Time in ms
    if (!isAuto || dt < execInterval) return false; // Wait for next execution time
    
    lastExecTime = now;
    int32_t error   = *SPptr - *PVptr;

    if (kp) {
        pTerm = (int32_t)kp * error / 100;          // 100 is the scaling for percentage 
    }
    if (ki) {
        calcITerm(dt * error);                      // Compensate drift and load
    }
    if (kd) {
        int32_t de  = lastError - error;            // Delta Error
        lastError   = error;
        
        dTerm = (int32_t)(kd * de) / (int32_t)dt;   // Oppose PV movement based on
                                                    //  first derivative of PV
    }
    lastOP  = pTerm + iTerm + dTerm;
    if (lastOP > maxOut) lastOP = maxOut;           // Keep inside actuator assumed range
    if (lastOP < minOut) lastOP = minOut;
    
    *OPptr = lastOP;
    return true;
}

void    iPID::calcITerm(int32_t weightedError) {
    iTermF3     += weightedError * (int32_t) iFactor / 10L;
    iTerm       = iTermF3 / 1000L;
    if (iTerm > maxOut)  {                          // Prevent wind-up of the iTerm
        iTerm   = maxOut;
        iTermF3 = iTerm * 1000L;
    } else if (iTerm < minOut) {
        iTerm   =  minOut;
        iTermF3 = iTerm * 1000L;
    }
}

void    iPID::SetCvLimits(int16_t minOP, int16_t maxOP){
    if (minOP == maxOP) {
        minOut = 0;
        maxOut = 255;    
    } else if (minOP < maxOP) {
        minOut = minOP;
        maxOut = maxOP;
    } else {
        minOut = maxOP;
        maxOut = minOP;        
    }
}

void    iPID::setSigns() {
    if (isRev) {
        kp  = -(int32_t)pFactor;
        ki  = - iFactor;
        kd  = dFactor;
    } else {
        kp  = pFactor;
        ki  = iFactor;
        kd  = - (int32_t)dFactor;        
    }    
}

void    iPID::SetTuning(uint16_t origKp, uint16_t origKi, uint16_t origKd){
    pFactor = origKp;
    iFactor = origKi;
    dFactor = origKd;
    if (pFactor == 0) pFactor = 100;    // Use 1.00 as default P-factor
    setSigns();                         // Set kp, ki, kd signs by the direction
}

void    iPID::SetDirection(bool isReverse){
    isRev = isReverse;
    setSigns();  
}

void    iPID::SetInterval(uint16_t executeInterval){
    if (execInterval == executeInterval) return;
    if (executeInterval == 0) executeInterval = 100;
    execInterval    = executeInterval;
}

uint16_t    iPID::Kp()      {return pFactor;}
uint16_t    iPID::Ki()      {return iFactor;}
uint16_t    iPID::Kd()      {return dFactor;}
int16_t     iPID::PTerm()   {return pTerm;}
int16_t     iPID::ITerm()   {return iTerm;}
int16_t     iPID::DTerm()   {return dTerm;}
bool        iPID::IsRevDirection()  {return isRev;}
bool        iPID::IsAutoMode()      {return isAuto;}

