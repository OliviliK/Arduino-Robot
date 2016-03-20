#ifndef VNH2SP30_H
#define VNH2SP30_H

#include <Arduino.h>

typedef enum  motorStates {initError,isRunning,isBreaking,isCoasting} MotorState;

class Vnh2sp30 {
  public:
    Vnh2sp30(   const uint8_t enaPin,   const uint8_t aPin,   const uint8_t bPin, 
                const uint8_t pwmPin,   const uint8_t csPin,  const uint8_t inv);
    void        readCurrent();
    uint16_t    current();
    uint16_t    maxCurrent();
    void        run(int16_t power);   // -1023 .. + 1023
    int16_t     power();
    void        stop();
    void        coast();
    MotorState  state();
  private:
    uint8_t     _enaPin, _aPin, _bPin, _pwmPin, _csPin;
    uint16_t    _maxAcc;              // [ms] to accelerate 1023 steps in power
    int16_t     _power;               // -1023 .. + 1023
    uint16_t    _current;             // from CS input
    uint16_t    _maxCurrent;          // max current during run time
    MotorState  _state;               // motor state
};
#endif
