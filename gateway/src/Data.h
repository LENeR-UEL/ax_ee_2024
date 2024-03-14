#pragma once

#include <stdint.h>
#include "Twai/Twai.h"
#include "Bluetooth/Bluetooth.h"

enum class FlagTrigger {
    MalhaAberta,
    MalhaFechadaOperacao
};

class Data {
public:
    Data();

    uint16_t pwm;
    uint16_t mese;
    uint16_t meseMax;
    uint16_t weightL;
    uint16_t weightR;
    uint16_t collectedWeight;
    uint16_t setpoint;
    FlagTrigger trigger;

    // PWM atual indicado pelo hardware estimulador.
    uint16_t pwmFeedback;

    // No aplicativo, ao setar o MESE na aba Malha Aberta, o PWM deve ser gradualmente zerado.
    // Na função loop(), se essa flag for true, o PWM é diminuído em 5 unidades a cada 250ms.
    // Quando PWM=0, essa flag é setada para false.
    bool isDecreasingPwm;

    unsigned long lastBluetoothSendTime;

    void sendToTwai() const;

    void sendToBle(const Bluetooth &ble);

    void debugPrintAll();
};

extern Data data;