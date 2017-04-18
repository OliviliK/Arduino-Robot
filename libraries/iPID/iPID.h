#ifndef IPID_H
#define IPID_H

#include <Arduino.h>

class iPID {
public:
    iPID(   int16_t* ProcessValue, int16_t* ControlValue, int16_t* SetPoint,
            uint16_t pFactorPct = 100, uint16_t iFactor = 0, uint16_t dFactor = 0, 
            uint16_t executeInterval = 100, bool isReverse = false);
    void    SetMode(bool isAutomatic);
    bool    Execute();
    void    SetCvLimits(int16_t minOP, int16_t maxOP);
    void    SetTuning(uint16_t pFactorPct, uint16_t iFactor, uint16_t dFactor);
    void    SetDirection(bool isReverse);
    void    SetInterval(uint16_t executeInterval);

    uint16_t    Kp();
    uint16_t    Ki();
    uint16_t    Kd();
    int16_t     PTerm();
    int16_t     ITerm();
    int16_t     DTerm();
    bool        IsRevDirection();
    bool        IsAutoMode();

private:
    void        setSigns();
    void        initPID();
    void        calcITerm(int32_t weightedError);
    
    uint16_t    pFactor,iFactor,dFactor;
    int16_t     kp,ki,kd;
    int16_t     minOut,maxOut;
    uint32_t    execInterval;
    bool        isRev,isAuto;

    int16_t     *PVptr,*OPptr,*SPptr;
    int16_t     lastOP,lastError;
    int32_t     iTermF3;
    int16_t     pTerm,iTerm,dTerm;
    uint32_t    lastExecTime;
};
#endif
