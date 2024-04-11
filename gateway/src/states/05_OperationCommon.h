#pragma once
#include <stdint.h>

// Há quantos milisegundos estamos na classe de peso atual
extern unsigned long weightClassTimer;
extern unsigned long lastWeightClassChangeTime;

// A classe de peso atual
extern uint8_t weightClass;

extern void updateCurrentWeightClass();

extern uint8_t OPERATION_MESE_MAX_CHANGE_STEP;
