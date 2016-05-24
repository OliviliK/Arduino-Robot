#Arduino-Robot Projects
##Ultrasonic Sensor Array

Demonstrate how the Pin Interrupt mechanism is used to monitor the echo signal from ultrasonic sensors.
The delta time between the rising and falling edge of the signal is used to calculate the delta time which is directly proportional to the distance.

A timer interrupt is used to provide a delay between the channels in the sensor array.
In practice the whole sensor interface is implemented in the interrupt routines done in the background at maximal speed and efficiency.

###Ultrasonic Sensor Array PortK
A variant of the sensor array using different pins.  This variant is used in Wissahickon Rover robot.

##Cable and Wire Testers

###Cable Tester
Simplistic 3 wire tester for servo and sensor cables.

###8 Wire Tester
A general purpose cable tester up to 8 wires.
For example, two Ultrasonic Sensor Cables can be tested at the same time.

##2 Optical Distance Sensor Tester
Two SHARP 2Y0A21 optical disance sensors are used to measure the distance to objects in front of the Wissahickon Rover Robot.  The results are shown in the LED & KEY panel and Serial Plotter.  The Serial Plotter demonstrates the characteristic noise in the raw signal.

##8 IR Collision Detector Tester
Eight IR sensors (FC-51) are in the corners of the Wissahickon Rover Robot.  This tester shows in the LED & KEY panel which sensors are active.
