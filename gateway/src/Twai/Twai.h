#pragma once

#include <driver/twai.h>
#include <Arduino.h>

// ESP-32 de desenvolvimento
#define WIRESS_GPIO_TX GPIO_NUM_5
#define WIRESS_GPIO_RX GPIO_NUM_4

// ESP-32 gateway no LENeR
// #define WIRESS_GPIO_TX GPIO_NUM_16
// #define WIRESS_GPIO_RX GPIO_NUM_25

enum TwaiSendMessageKind : uint8_t {
    WeightL = 0x51,
    WeightR = 0x52,
    SetRequestedPwm = 0x61,
    Mese = 0x71,
    MeseMax = 0x72,
    Setpoint = 0x81,
    Trigger = 0x82
};

enum TwaiReceivedMessageKind : uint8_t {
    PwmFeedbackEstimulador = 0x6A
};

struct TwaiReceivedMessage {
    TwaiReceivedMessageKind Kind;
    uint16_t ExtraData;
};

void twaiStart();

void twaiSend(TwaiSendMessageKind kind, uint16_t extraData);

esp_err_t twaiReceive(TwaiReceivedMessage *received);