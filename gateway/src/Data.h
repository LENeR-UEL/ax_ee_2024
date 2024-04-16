#pragma once

#include <stdint.h>
#include "Twai/Twai.h"
#include "Bluetooth/Bluetooth.h"

enum class FlagTrigger
{
    MalhaAberta,
    MalhaFechadaOperacao
};

class Data
{
public:
    Data();

    uint16_t mese;
    uint16_t meseMax;
    uint16_t weightL;
    uint16_t weightR;
    uint16_t collectedWeight;
    uint16_t setpoint;

    // PWM atual indicado pelo hardware estimulador.
    uint16_t pwmFeedback;

    struct __attribute__((__packed__))
    {
        uint16_t gradualIncreaseInterval;
        uint8_t gradualIncreaseStep;
        uint16_t transitionTime;
        uint16_t gradualDecreaseInterval;
        uint8_t gradualDecreaseStep;
        uint16_t malhaFechadaAboveSetpointTime;
    } parameterSetup;

    // Informar o app do estado atual da operação
    uint8_t mainOperationStateInformApp[6];

    unsigned long lastBluetoothSendTime;

    void sendToBle(const Bluetooth &ble);

    void debugPrintAll();
};

extern Data data;
