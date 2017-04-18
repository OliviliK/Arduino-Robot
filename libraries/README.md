#Arduino-Robot Libraries
All these libraries use the best Arduino practices.  This includes the following
 * All h-files have the #ifndef protection
 * All h-files have only declarations and no data allocations
 * The h-file and cpp-file have the same name as the library
 * There is a h-file related keyrords.txt in every library with the following logic
    * KEYWORD1 for datatypes and class names
    * KEYWORD2 for methods and functions
    * KEYWORD3 for enumerations
    * LITERAL1 for constants
 * There is examples directory with ino-files to demonstrate the library usage
  
##TM1638 with 8 Buttons, 8 LEDs, and 8 7-segment Displays

This library allows the applications to use TM1638 based LED&KEY control panel.

##Vnh2sp30 Motor Controller

This library allows the applications to use Vnh2sp30 based motor controllers that support up to 30 A current from 5.5 - 16 V DC power supply.

##HC_SR04 Ultrasonic Sensor

This library allows the applications to use multiple, up to 7, ultrasonic distance sensors.  The distance range is from 30 mm up to 3 m.  If the target has good sound reflection, the readings are very stabile and accurate.  If the target is small or sound absorbing, then the readings are not very reliable.

##ProcSimulator

This library supports the learning of the tuning of a PID controller.
The simulated process is a mass connected with a spring to a moving point.
Without proper tuning this process has a tendency to oscillate.
The oscillation can be reduced with simulated friction.

##iPID Integer PID Controller

This controller is implmented using integer arithmetics due to missing floating point support in Arduino Mega
and similar low level MCUs. The functionality in this controller has features common in process control applications,
such as windup avoidance, support of AUTO/MANUAL modes, setpoint tracking, and on-process tuning parameter changes.

##WH_Rover Interface

This library provides symbolic access to Wissahickon Rover sensors and motors.
There is a new function to scale the Sharp Optical Distance sensor to be compatible with the millimeter values
used in the Ultrasonic Sensor.
