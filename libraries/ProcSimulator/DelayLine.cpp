/**
 *  Created by Olavi Kamppari on 2/14/2017.
 */

/**
 *  File: DelayLine.cpp
 *
 *  Flexible FIFO construction to allow set a new value into FIFO
 *  and take out the oldest value from the FIFO.
 *
 *  FIFO size 0 is supported.  This implies no delay.
 */

#include "DelayLine.h"
#include <stdlib.h>

DelayLine::DelayLine() {
    _nrSlots = 0;    
}

DelayLine::DelayLine(uint16_t nrSlots) {
    _nrSlots = nrSlots;
    if (_nrSlots) {
        slots = (int16_t*)calloc(_nrSlots,sizeof(int16_t));
    }
    nextSlot = 0;
}

int16_t DelayLine::exchange(int16_t newValue) {
    if (_nrSlots) {
        int16_t oldValue = slots[nextSlot];
        slots[nextSlot] = newValue;
        nextSlot++;
        if (nextSlot == _nrSlots) nextSlot = 0;
        return oldValue;
    } else {
        return newValue;
    }
}

void DelayLine::setOldValues(int16_t oldValue) {
    for (int i=0;i<_nrSlots;i++) {
        slots[i] = oldValue;
    }
}

void DelayLine::setDelay(uint16_t nrSlots, int16_t oldValue) {
    if (_nrSlots) free(slots);
    _nrSlots = nrSlots;
    if (_nrSlots) {
        slots = (int16_t*)calloc(_nrSlots,sizeof(int16_t));
    }
    nextSlot = 0;
    if (oldValue) setOldValues(oldValue);    
}

