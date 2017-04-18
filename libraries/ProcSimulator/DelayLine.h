#ifndef DELAYLINE_H
#define DELAYLINE_H

#include <Arduino.h>

class DelayLine {
public:
    DelayLine();
    DelayLine(uint16_t nrSlots);
    int16_t exchange(int16_t newValue);
    void setOldValues(int16_t oldValue);
    void setDelay(uint16_t nrSlots, int16_t oldValue);
private:
    uint16_t    _nrSlots;
    int16_t     *slots;
    uint16_t    nextSlot;
};

#endif
