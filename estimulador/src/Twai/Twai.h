#pragma once
#include <driver/twai.h>
#include <Arduino.h>

#define WIRESS_GPIO_TX GPIO_NUM_17
#define WIRESS_GPIO_RX GPIO_NUM_16

// #define WIRESS_GPIO_TX GPIO_NUM_5
// #define WIRESS_GPIO_RX GPIO_NUM_4

enum TwaiSendMessageKind : uint8_t
{
    PwmFeedbackEstimulador = 0x6A
};

enum TwaiReceivedMessageKind : uint8_t
{
    FirmwareInvokeReset = 0x01,
    GatewayResetHappened = 0x02,
    WeightTotal = 0x51,
    ResidualWeightTotal = 0x52,
    SetRequestedPwm = 0x61,
    Mese = 0x71,
    MeseMax = 0x72,
    Setpoint = 0x81,
    UseMalhaAberta = 0x82,
    UseMalhaFechada = 0x83,
    SetGainCoefficient = 0xA1,
};

struct TwaiReceivedMessage
{
    TwaiReceivedMessageKind Kind;
    uint16_t ExtraData;
};

void twaiStart();
void twaiSend(TwaiSendMessageKind kind, uint16_t extraData);
esp_err_t twaiReceive(TwaiReceivedMessage *received);
bool twaiIsAvailable();
