#include <Arduino.h>
#include <esp_log.h>
#include "../Bluetooth/Bluetooth.h"
#include "../Twai/Twai.h"
#include "../Data.h"
#include "../StateManager.h"
#include "../Scale/Scale.h"

static const char *TAG = "ParameterSetup";

static unsigned long lastTwaiSendTime = 0;

void onParameterSetupStateEnter()
{
    data.mese = 0;
    data.meseMax = 0;
    data.setpoint = 0;
    data.collectedWeight = 0;
}

void onParameterSetupStateLoop()
{
    if (!espBle.isConnected())
    {
        stateManager.switchTo(StateKind::Disconnected);
        return;
    }

    ESP_LOGI(TAG, "Loop");

    long now = millis();
    if (now - lastTwaiSendTime >= 100)
    {
        twaiSend(TwaiSendMessageKind::WeightL, scaleGetWeightL());
        twaiSend(TwaiSendMessageKind::WeightR, scaleGetWeightR());
        twaiSend(TwaiSendMessageKind::SetRequestedPwm, 0);
        twaiSend(TwaiSendMessageKind::Setpoint, 0);
        twaiSend(TwaiSendMessageKind::Mese, 0);
        twaiSend(TwaiSendMessageKind::MeseMax, 0);
        twaiSend(TwaiSendMessageKind::Trigger, (uint8_t)FlagTrigger::MalhaAberta);
    }
}

void onParameterSetupStateTWAIMessage(TwaiReceivedMessage *receivedMessage)
{
    switch (receivedMessage->Kind)
    {
    case TwaiReceivedMessageKind::PwmFeedbackEstimulador:
        data.pwmFeedback = receivedMessage->ExtraData;
        break;
    }
}

void onParameterSetupStateBLEControl(BluetoothControlCode code, uint8_t extraData)
{
    switch (code)
    {
    case BluetoothControlCode::ParameterSetup_Complete:
        stateManager.switchTo(StateKind::ParallelWeight);
        return;
    case BluetoothControlCode::FirmwareInvokeReset:
        esp_restart();
        return;
    }
}

void onParameterSetupStateExit() {}
