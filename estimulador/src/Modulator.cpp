#include "Modulator.h"
#include <Arduino.h>

static unsigned long lastModulateTime = 0;
static uint32_t vDurationMicros = 0;

void modulateOnce(int pulseWidthMicros)
{
    if (pulseWidthMicros >= 10)
    {
        digitalWrite(2, HIGH);
        digitalWrite(32, HIGH);
        delayMicroseconds(pulseWidthMicros);
        digitalWrite(2, LOW);
        digitalWrite(32, LOW);
        delayMicroseconds(4);

        digitalWrite(4, HIGH);
        digitalWrite(33, HIGH);
        delayMicroseconds(pulseWidthMicros);
        digitalWrite(4, LOW);
        digitalWrite(33, LOW);
        delayMicroseconds(4);
    }
    else
    {
        digitalWrite(2, LOW);
        digitalWrite(4, LOW);
        digitalWrite(32, LOW);
        digitalWrite(33, LOW);
        delayMicroseconds(8);
    }
}

void modulateLoop(int pulseWidthMicros)
{
    if (pulseWidthMicros < 1)
        pulseWidthMicros = 0;

    unsigned long now_micros = micros();
    if (now_micros - lastModulateTime > vDurationMicros)
    {
        modulateOnce(pulseWidthMicros);
        lastModulateTime = now_micros;
        vDurationMicros = (uint32_t)(((1 / 35.0) * 1000000) - ((2 * pulseWidthMicros) - 8));
    }
}