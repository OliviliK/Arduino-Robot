#include <ProcSimulator.h>
#include <iPID.h>

uint16_t count;
int16_t procValue,outPut,setPoint;
bool    executed;

//  ProcSimulator
//  uint16_t actLag, int16_t actGainPct, 
//  uint16_t mass=1, uint16_t frictionPct=0, uint16_t procLag=0,
//  int16_t initCV=0, int16_t minCV=0, int16_t maxCV=100, 
//  int16_t initPV=0, int16_t minPV=0, int16_t maxPV=100

ProcSimulator ps(
    1,100,
    100,100,1,
    300, 0, 1023,
    1000, 0, 2000);

//  iPID
//  int16_t* ProcessValue, int16_t* ControlValue, int16_t* SetPoint,
//  uint16_t pFactorPct = 100, uint16_t iFactor = 0, uint16_t dFactor = 0, 
//  uint16_t executeInterval = 100, bool isReverse = false

iPID ctrl(&procValue, &outPut, &setPoint,
    40,35,50,
    30);

void setup() {
    Serial.begin(230400);
    while (!Serial);
    Serial.print("SP\tPV\tOP\tpT\tiT\tdT\n");

    ctrl.SetDirection(ps.ActGainPct() > 0?0:1); // Track the simulator gain
    ctrl.SetCvLimits(ps.MinCV(),ps.MaxCV());
    
    setPoint  = 0;
    count     = 0;
    procValue = ps.PV();
    setPoint  = procValue;
    outPut    = ps.CV();
}

void synch(uint32_t timeMs) {
    uint32_t now = millis();
    while (millis() == now);
    while(millis() % timeMs);
}

void loop() {
    count++;
    if (count == 3)     ctrl.SetMode(1);
    if (count == 10)    setPoint = 1100;
    if (count == 100)   ps.SetLoad(100);
    if (count == 250)   ps.SetLoad(-100);
    
    procValue   = ps.PV();          // Excute simulation
    outPut      = ps.CV();
    
    executed    = ctrl.Execute();   // Execute control
    ps.SetCV(outPut);               // Prepare next simulation step
    
                                    // Show results
    Serial.print(setPoint);         Serial.print("\t");
    Serial.print(procValue);        Serial.print("\t");
    Serial.print(outPut);           Serial.print("\t");
    Serial.print(ctrl.PTerm());     Serial.print("\t");
    Serial.print(ctrl.ITerm());     Serial.print("\t");
    Serial.print(ctrl.DTerm());     Serial.print("\t");
    Serial.println();
    
    synch(10);                      // Wait for next time slot
    if (millis() > 4000) while(1);  // Stop after 4 seconds
}
