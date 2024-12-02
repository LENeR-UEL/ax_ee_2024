#pragma once
#include <stdint.h>

struct Data
{
    uint16_t weightTotal;
    uint16_t residualWeightTotal;
    uint16_t requestedPwm;
    uint16_t mese;
    uint16_t meseMax;
    uint16_t setpointKg;
    float gainCoefficient;
};

extern Data data;

void dataReset();