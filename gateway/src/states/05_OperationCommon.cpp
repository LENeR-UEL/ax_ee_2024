#include <Arduino.h>
#include "05_OperationCommon.h"
#include "Scale/Scale.h"

uint8_t OPERATION_MESE_MAX_CHANGE_STEP = 5;

// Último momento em que a mudança de estado aconteceu
unsigned long lastWeightClassChangeTime = 0;
unsigned long weightClassTimer = 0;
uint8_t weightClass = 0;

// Dividir o peso lido em grupos desse tamanho. Usado para transições de estado, já que pode haver flutuações na leitura das balanças.
static const uint8_t weightClassSize = 10;

void updateCurrentWeightClass()
{
    unsigned long now = millis();
    uint8_t currentClass = scaleGetTotalWeight() / weightClassSize;

    if (currentClass != weightClass)
    {
        lastWeightClassChangeTime = now;
    }

    weightClass = currentClass;
    weightClassTimer = now - lastWeightClassChangeTime;
}
