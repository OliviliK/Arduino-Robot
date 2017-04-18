/**
 *  Created by Olavi Kamppari on 2/20/2017.
 */

/**
 *  File: ProcSimDemo.ino
 *
 *  Demonstrates the impact of Process Simulator parameters with 7 different settings
 *  In all cases, the process actuator range and process value ranges are same.
 *
 *  The process actuator simulates a typical 10 bit actuator device with control values
 *  from 0 to 1023.  The initial value is set to 300.
 *
 *  The process value simulates a typical 12 bit transmitter.  For example a distance
 *  sensor for the range from 0 to 4000 mm.  The initial value is set to 1000
 *
 *  The first set of parameters are for the actuator.  First one is the actuator delay
 *  in execution cycles.  The second one is the actuator gain to translate the
 *  actuator value to internal actuator target position.  This is given a a percent number.
 *
 *  If the actuator gain is negative, then the actuator value and process value are
 *  moving in the opposite directions.
 *
 *  The second set of paramaters are for the simulated process to translate the target
 *  position to transmitter value.  The first parameter represent a mass that is slowing
 *  down the movmement.  There is a simulated spring between the mass and the target
 *  position.  A longer distance causes bigger force and thus bigger acceleration.
 *
 *  This spring mechanism has inherit vibration characcteristic.  The secon parameter
 *  represents a friction element (in percentage) to slow down the mass movmenet when
 *  the force changes direction.
 *
 *  The third parameter is the transmitter delay in execution cycles. 
 *  
 *  The characteristics of the simulators are:
 *    - The first (ps0) and second (ps1) share the same parameters except the gain
 *    - The ps1 and ps2 share the same parameters, except ps2 has friction
 *    - The ps2 and ps3 share the same parameters, except ps3 mass is much smaller
 *    - The ps3 and ps4 share the same parameters, except ps4 has actuator delay
 *    - The ps5 has larger actuator gain, and more friction
 *    - The ps6 has larger mass and more friction
 *
 *  At the simulation count 10, the Control Value (CV) is increased from 300 to 400.
 *
 *  At the simulated count 110 and 210, an external load is applied to all simulators.
 *
 *  Note 1: The force has an immediate impact to ps4, which has a simulated actuator delay
 *  If a transmitter delay had been used, then the load impact had been delayd.
 *
 *  Note 2: The actutator delay and transmitter delay, increases the oscillation tendency
 *  in a control system.
 *
 *  Note 3: The lack of friction, nakes the tuning of process control more difficult.
 *
 *  The results are shown in Serial Plotter.
 *
 */

#include <ProcSimulator.h>

uint16_t count;

//  ProcSimulator
//  uint16_t actLag, int16_t actGainPct, 
//  uint16_t mass=1, uint16_t frictionPct=0, uint16_t procLag=0,
//  int16_t initCV=0, int16_t minCV=0, int16_t maxCV=100, 
//  int16_t initPV=0, int16_t minPV=0, int16_t maxPV=100

ProcSimulator ps0( 0,-100,   10,  0, 0,   300, 0, 1023,   1000, 0, 2000);
ProcSimulator ps1( 0, 100,   10,  0, 0,   300, 0, 1023,   1000, 0, 2000);
ProcSimulator ps2( 0, 100,   10, 10, 0,   300, 0, 1023,   1000, 0, 2000);
ProcSimulator ps3( 0, 100,    2, 10, 0,   300, 0, 1023,   1000, 0, 2000);
ProcSimulator ps4(20, 100,    2, 10, 0,   300, 0, 1023,   1000, 0, 2000);
ProcSimulator ps5( 0, 400,   10, 20, 0,   300, 0, 1023,   1000, 0, 2000);
ProcSimulator ps6( 0, 400,  100, 90, 0,   300, 0, 1023,   1000, 0, 2000);


void setup() {
    Serial.begin(230400);
    Serial.print("time\tPV\tOP\n");
    count     = 0;
}

void synch(uint32_t timeMs) {
    uint32_t now = millis();
    while (millis() == now);
    while(millis() % timeMs);
}

void loop() {
    count++;
    if (count == 10) {
        ps0.SetCV(400);
        ps1.SetCV(400);
        ps2.SetCV(400);
        ps3.SetCV(400);
        ps4.SetCV(400);
        ps5.SetCV(400);
        ps6.SetCV(400);
    }
    if (count == 110) {
        ps0.SetLoad(100);
        ps1.SetLoad(100);
        ps2.SetLoad(100);
        ps3.SetLoad(100);
        ps4.SetLoad(100);
        ps5.SetLoad(100);
        ps6.SetLoad(100);
    }
    if (count == 210) {
        ps0.SetLoad(-100);
        ps1.SetLoad(-100);
        ps2.SetLoad(-100);
        ps3.SetLoad(-100);
        ps4.SetLoad(-100);
        ps5.SetLoad(-100);
        ps6.SetLoad(-100);
    }

    Serial.print(ps0.PV() +    0);  Serial.print("\t");
    Serial.print(ps1.PV() + 1000);  Serial.print("\t");
    Serial.print(ps2.PV() + 2000);  Serial.print("\t");
    Serial.print(ps3.PV() + 3000);  Serial.print("\t");
    Serial.print(ps4.PV() + 4000);  Serial.print("\t");
    Serial.print(ps5.PV() + 5000);  Serial.print("\t");
    Serial.print(ps6.PV() + 6000);  Serial.print("\t");
    Serial.println();
    
    synch(2);                      // Wait for next time slot
    if (count>500) while(1);  // Stop after 4 seconds
}
