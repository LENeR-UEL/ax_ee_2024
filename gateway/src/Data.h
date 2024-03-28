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

    // Informar o app do estado atual da operação
    uint8_t mainOperationStateInformApp[4];

    unsigned long lastBluetoothSendTime;

    void sendToBle(const Bluetooth &ble);

    void debugPrintAll();
};

extern Data data;
