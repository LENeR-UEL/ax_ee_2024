#include "Scale.h"

#ifdef SCALE_USE_STUB

#include <random>
#include <Arduino.h>

#define POTENTIOMETER_INPUT_PIN 35

static const char *TAG = "RealScale";

// Valor atual da balança em kg
float correctedReadingKg[] = {0.0f, 0.0f, 0.0f, 0.0f};

const float CORRECAO[] = {
    1.3f, // A
    1.0f, // B
    0.3f, // C
    1.2f, // D
};

int random(int min, int max) // range : [min, max]
{
  return min + rand() % ((max + 1) - min);
}

void switchScale(Scale scaleId)
{
  // noop
}

void readScale(Scale scaleId)
{
  int medida = map(analogRead(POTENTIOMETER_INPUT_PIN), 0, 4095, 0, 30);
  correctedReadingKg[scaleId] = medida + CORRECAO[scaleId];
}

void scaleBeginOrDie()
{

  ESP_LOGI(TAG, "Scale setup");
  pinMode(POTENTIOMETER_INPUT_PIN, INPUT);
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

#endif
