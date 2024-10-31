#include "Scale.h"

#ifndef SCALE_USE_STUB
#include <HX711.h>

#define SCALE_RING_BUFFER_SIZE 5

static const char *TAG = "RealScale";

HX711 scale;

// Valores iniciais da balança (offset), quando não há peso sobre elas
float baseReading[] = {0.0f, 0.0f, 0.0f, 0.0f};
// Valor atual da balança (medição mais recente), excluindo o offset respectivo
float currentReading[] = {0.0f, 0.0f, 0.0f, 0.0f};

const float CORRECAO[] = {
    18884.8f, // A
    19846.8f, // B
    5297.6f,  // C
    19660.0f, // D
};

float readingRing[4][SCALE_RING_BUFFER_SIZE];
uint8_t ringIndex = 0;

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
    float value = (currentReading[scaleId] / CORRECAO[scaleId]);

    if (value < 0)
    {
        value = 0;
    }

    readingRing[scaleId][ringIndex] = value;
}

float scaleGetMeasurement(Scale scaleId)
{
    // Retornar o menor valor
    float menor = 99999.0f;

    for (int i = 0; i < SCALE_RING_BUFFER_SIZE; i++) {
        float atual = readingRing[scaleId][i];
        if (atual < menor) menor = atual;
    }

    return menor;
}

void scaleBeginOrDie()
{
    ESP_LOGI(TAG, "Scale setup");

    memset(&readingRing, 0, sizeof(readingRing));

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

    ringIndex++;
    if (ringIndex >= SCALE_RING_BUFFER_SIZE) {
        ringIndex = 0;
    }
}

int scaleGetWeightL()
{
    return scaleGetMeasurement(Scale::A) + scaleGetMeasurement(Scale::B);
}

int scaleGetWeightR()
{
    return scaleGetMeasurement(Scale::C) + scaleGetMeasurement(Scale::D);
}

int scaleGetTotalWeight()
{
    return scaleGetWeightL() + scaleGetWeightR();
}
#endif
