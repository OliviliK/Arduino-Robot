/**
 *  Created by Olavi Kamppari on 7/7/2016.
 */

#include <Arduino.h>

#ifndef HC_SR04_H
#define HC_SR04_H

class HC_SR04 {
  public:
    HC_SR04   (uint8_t selectionMask);
    HC_SR04   ();
    void      selectSensors(uint8_t selectionMask);
    uint8_t   selectionMask();
    uint32_t  readSensor(uint8_t sensorNumber);
  protected:
};

#endif


