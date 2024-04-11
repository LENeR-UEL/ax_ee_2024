#include "Scale.h"

#ifndef SCALE_USE_STUB
#include <HX711.h>

static const char *TAG = "RealScale";

HX711 scale;

// Valores iniciais da balança (offset), quando não há peso sobre elas
float baseReading[] = {0.0f, 0.0f, 0.0f, 0.0f};
// Valor atual da balança (medição mais recente), excluindo o offset respectivo
float currentReading[] = {0.0f, 0.0f, 0.0f, 0.0f};
// Valor atual da balança em kg
float correctedReadingKg[] = {0.0f, 0.0f, 0.0f, 0.0f};

const float CORRECAO[] = {
    18884.8f, // A
    19846.8f, // B
    5297.6f,  // C
    19660.0f, // D
};

void switchScale(Scale scaleId)
{
    switch (scaleId)
    {
    case Scale::A:
        scale.begin(23, 22);
        break;
    case Scale::B:
        scale.begin(21, 22);
        break;
    case Scale::C:
        scale.begin(18, 22);
        break;
    case Scale::D:
        scale.begin(17, 22);
        break;
    }
}

void readScale(Scale scaleId)
{
    switchScale(scaleId);

    currentReading[scaleId] = scale.read() - baseReading[scaleId];
    correctedReadingKg[scaleId] = (currentReading[scaleId] / CORRECAO[scaleId]);

    if (correctedReadingKg[scaleId] < 0)
    {
        correctedReadingKg[scaleId] = 0;
    }
}

void scaleBeginOrDie()
{
    ESP_LOGI(TAG, "Scale setup");

    // Obter os valores iniciais (offset) das balanças, quando não há peso sobre elas
    switchScale(Scale::A);
    baseReading[Scale::A] = scale.read_average();

    switchScale(Scale::B);
    baseReading[Scale::B] = scale.read_average();

    switchScale(Scale::C);
    baseReading[Scale::C] = scale.read_average();

    switchScale(Scale::D);
    baseReading[Scale::D] = scale.read_average();
}

// scaleUpdate reads all the scales values and saves them.
void scaleUpdate()
{
    readScale(Scale::A);
    readScale(Scale::B);
    readScale(Scale::C);
    readScale(Scale::D);
}

int scaleGetMeasurement(Scale whichOne)
{
    return correctedReadingKg[whichOne];
}

int scaleGetWeightL()
{
    return scaleGetMeasurement(Scale::C) + scaleGetMeasurement(Scale::D);
}

int scaleGetWeightR()
{
    return scaleGetMeasurement(Scale::A) + scaleGetMeasurement(Scale::B);
}

int scaleGetTotalWeight()
{
    return scaleGetWeightL() + scaleGetWeightR();
}
#endif
