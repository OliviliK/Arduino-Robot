/**
 *  Created by Olavi Kamppari on 3/19/2016.
 *
 *	Updated by Olavi Kamppari on 3/21/2017
 *  - remove exp filter due to
 *    > ramping in WH_Rover
 *	  > PID control in FTC motor controller
 */

/**
 *  STMicroelectronic (www.st.com) VNH2SP30-E s a full bridge motor driver.
 *  http://www.st.com/web/catalog/sense_power/FM1965/SC1039/PF145473
 *
 *  It is intended for automotive applications.  The 4 MOSFET transistors
 *  are controlled by four 5 V inputs
 *    > Enable
 *    > Direction A
 *    > Direction B
 *    > PWM power level up to 20 kHz
 *  There is one -3V .. +15V current sense output
 *
 *  If Enable is off, all 4 transistors are off
 *  If the two direction signals have the same value, the driver is stopping
 *  the motor by short circuit either to ground or supply voltage.
 *
 *  The current sense has negative voltage when the motor is autogenerating.
 *
 *  In this library, the PWM signal is controlled with a timer with values
 *  from 0 (no power) to 1023 (full power)
 * 
 *  A typical Vnh2sp30 boards are
 *  1. Single motor http://www.aliexpress.com/item/30A-Mini-VNH2SP30-Stepper-Motor-Driver-Monster-Moto-Shield-module-For-Arduino/32464304011.html
 *  2. Duals motors http://www.aliexpress.com/item/Free-Shipping-Monster-Moto-Shield-VNH2SP30-stepper-motor-driver-module-high-current-30A-for-arduino/1992619346.html
 */
 
#include <Vnh2sp30.h>

#define MAXDIGPIN 53
#define MINANAPIN A0
#define MAXENAPIN A1

Vnh2sp30::Vnh2sp30( const uint8_t enaPin,   const uint8_t aPin,   const uint8_t bPin, 
                    const uint8_t pwmPin,   const uint8_t csPin,  const uint8_t inv) {
  _enaPin   = enaPin;                             // Store the Enable pin number
  _aPin     = (inv == 0)? aPin: bPin;             //           A direction
  _bPin     = (inv == 0)? bPin: aPin;             //           B direction
  _pwmPin   = pwmPin;                             //           Pulse Width Modulater power
  _csPin    = csPin;                              //           Current Sense Analog Input

  if (_enaPin > MAXENAPIN                         // Check parameter ranges
  ||  _aPin   > MAXDIGPIN
  ||  _bPin   > MAXDIGPIN
  ||  _pwmPin > MAXDIGPIN
  ||  _csPin  < MINANAPIN ) {
    _state  = initError;
  } else {
    _state  = isRunning;
  }

  _power       = 0;                               // Init local variables
  _current     = 0;
  _maxCurrent  = 0;

  if (_state != initError) {
    pinMode(_enaPin, OUTPUT);                     // set the output pins
    pinMode(_aPin,   OUTPUT);
    pinMode(_bPin,   OUTPUT);
    pinMode(_pwmPin, OUTPUT);

    stop();                                       // Stop the motor
  }
}

void Vnh2sp30::readCurrent() {
  _current    = analogRead(_csPin);
  if (_maxCurrent < _current) _maxCurrent = _current;
}

uint16_t Vnh2sp30::current() {return _current;}

uint16_t Vnh2sp30::maxCurrent() {return _maxCurrent;}

void Vnh2sp30::run(int16_t power) {     // 0 = no power, 1023 = full power
  if (_state  == initError) return;

  if (_state  == isCoasting) {
    _state    = isRunning;
    digitalWrite(_enaPin, HIGH);
  }

  if (power > 1023)   power = 1023;     // Clamp the setpoints
  if (power < -1023)  power = -1023;
  _power = power;

  if (_power > 0) {
    analogWrite(_pwmPin, _power);       // Forward command
    digitalWrite(_aPin, HIGH);
    digitalWrite(_bPin, LOW);
  } else {
    analogWrite(_pwmPin, -_power);      // Backward command
    digitalWrite(_aPin, LOW);
    digitalWrite(_bPin, HIGH);
  }
}

int16_t Vnh2sp30::power() {return _power;}

void Vnh2sp30::stop() {
  if (_state != initError) {
    _state    = isBreaking;
    _power    = 0;
    digitalWrite(_enaPin, HIGH);
    digitalWrite(_aPin, LOW);
    digitalWrite(_bPin, LOW);       // Short circuit outputs together
    analogWrite(_pwmPin,0);
  }
}

void Vnh2sp30::coast() {
  if (_state != initError) {
    _state    = isCoasting;
    _power    = 0;
    digitalWrite(_enaPin, LOW);     // Turn outputs to high impedance
    analogWrite(_pwmPin,0);
  }
}

MotorState  Vnh2sp30::state() {return _state;}
