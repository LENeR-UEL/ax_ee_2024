#include <Arduino.h>
#include <esp_log.h>
#include "../Bluetooth/Bluetooth.h"
#include "../Twai/Twai.h"
#include "../Data.h"
#include "../StateManager.h"
#include "../Scale/Scale.h"

static const char *TAG = "DisconnectedState";
static unsigned long lastTwaiSendTime = 0;

void onDisconnectedStateEnter()
{
    data.reset();
}

void onDisconnectedStateLoop()
{
    // Ao conectar, sair do estado Disconnected
    if (bluetoothIsConnected())
    {
        ESP_LOGI(TAG, "Conexão Bluetooth obtida!");
        stateManager.switchTo(StateKind::ParameterSetup);
        return;
    }

    long now = millis();
    if (now - lastTwaiSendTime >= 15)
    {
        lastTwaiSendTime = now;
        twaiSend(TwaiSendMessageKind::UseMalhaAberta, 0);
        twaiSend(TwaiSendMessageKind::WeightTotal, scaleGetWeightL() + scaleGetWeightR());
        twaiSend(TwaiSendMessageKind::SetRequestedPwm, 0);
        twaiSend(TwaiSendMessageKind::Setpoint, 0);
        twaiSend(TwaiSendMessageKind::Mese, 0);
        twaiSend(TwaiSendMessageKind::MeseMax, 0);
        twaiSend(TwaiSendMessageKind::SetGainCoefficient, data.parameterSetup.gainCoefficient);
    }
}

void onDisconnectedStateTWAIMessage(TwaiReceivedMessage *receivedMessage)
{
    switch (receivedMessage->Kind)
    {
    case TwaiReceivedMessageKind::PwmFeedbackEstimulador:
        data.pwmFeedback = receivedMessage->ExtraData;
        break;
    }
}

void onDisconnectedStateBLEControl(BluetoothControlCode code, uint8_t extraData) {}

void onDisconnectedStateExit() {}
