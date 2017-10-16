# Arduino-Robot Projects

----

##  Ultrasonic Sensor Array

Demonstrate how the Pin Interrupt mechanism is used to monitor the echo signal from ultrasonic sensors.
The delta time between the rising and falling edge of the signal is used to calculate the delta time which is directly proportional to the distance. Additional info is available at http://olliesworkshops.blogspot.com/2016/02/ultrasonic-sensor-array.html

A timer interrupt is used to provide a delay between the channels in the sensor array.
In practice the whole sensor interface is implemented in the interrupt routines done in the background at maximal speed and efficiency.
![Setup using a breadboard](https://3.bp.blogspot.com/-A7OYEk-cwWw/VrwCKxZmpcI/AAAAAAAACk4/ox09Ucv1NTI/s1600/JH8C6626.jpg)

### Ultrasonic Sensor Array PortK
A variant of the sensor array using different pins.  This variant is used in Wissahickon Rover robot.
![HC-SR04](https://3.bp.blogspot.com/-eV-Yprw0mto/Vrv7LpAml1I/AAAAAAAACj0/uARwGaCTOEk/s1600/HC-SR04.png)

The wiring is done using Arduino Mega sensor shield
![Sensor Shield](http://g02.a.alicdn.com/kf/HTB1HQ2AKpXXXXa7XXXXq6xXFXXX7/Sensor-Shield-for-Arduino-Mega-2560-with-SD-card-logger-Assembled-.jpg_200x200.jpg)

----

## Cable and Wire Testers

### Cable Tester
Simplistic 3 wire tester for servo and sensor cables.

### 8 Wire Tester
A general purpose cable tester up to 8 wires.
For example, two Ultrasonic Sensor Cables can be tested at the same time.

----

## 2 Optical Distance Sensor Tester
Two SHARP 2Y0A21 optical disance sensors are used to measure the distance to objects in front of the Wissahickon Rover Robot.  The results are shown in the LED & KEY panel and Serial Plotter.  The Serial Plotter demonstrates the characteristic noise in the raw signal.

![ODS](http://www.robotpark.com/image/cache/data/PRO/92432/92432_01-700x700.jpg)

---

## 8 IR Collision Detector Tester
Eight IR sensors (FC-51) are in the corners of the Wissahickon Rover Robot.  This tester shows in the LED & KEY panel which sensors are active.

![IR](http://g03.a.alicdn.com/kf/HTB1jf6NHpXXXXX_XVXXq6xXFXXXa/222401262/HTB1jf6NHpXXXXX_XVXXq6xXFXXXa.jpg)
