#include <Arduino.h>
#include <esp_log.h>
#include <Preferences.h>
#include "../Bluetooth/Bluetooth.h"
#include "../Twai/Twai.h"
#include "../Data.h"
#include "../StateManager.h"
#include "../Scale/Scale.h"

static const char *TAG = "ParallelWeight";
static unsigned long lastTwaiSendTime = 0;

static Preferences preferences;

void loadStoredWeight()
{
    preferences.begin("parameters", true);
    if (preferences.isKey("03weight"))
    {
        data.collectedWeight = preferences.getUShort("03weight", 0);
    }
    preferences.end();
}

void storeWeight()
{
    preferences.begin("parameters", false);
    preferences.putUShort("03weight", data.collectedWeight);
    preferences.end();
}

void onParallelWeightStateEnter()
{
    loadStoredWeight();
}

void onParallelWeightStateLoop()
{
    if (!espBle.isConnected())
    {
        stateManager.switchTo(StateKind::Disconnected);
        return;
    }

    long now = millis();
    if (now - lastTwaiSendTime >= 15)
    {
        lastTwaiSendTime = now;
        twaiSend(TwaiSendMessageKind::WeightTotal, scaleGetWeightL() + scaleGetWeightR());
        twaiSend(TwaiSendMessageKind::SetRequestedPwm, 0);
        twaiSend(TwaiSendMessageKind::Setpoint, 0);
        twaiSend(TwaiSendMessageKind::Mese, 0);
        twaiSend(TwaiSendMessageKind::MeseMax, 0);
        twaiSend(TwaiSendMessageKind::Trigger, (uint8_t)FlagTrigger::MalhaAberta);
    }
}

void onParallelWeightStateTWAIMessage(TwaiReceivedMessage *receivedMessage)
{
    switch (receivedMessage->Kind)
    {
    case TwaiReceivedMessageKind::PwmFeedbackEstimulador:
        data.pwmFeedback = receivedMessage->ExtraData;
        break;
    }
}

void onParallelWeightStateBLEControl(BluetoothControlCode code, uint8_t extraData)
{
    switch (code)
    {
    case BluetoothControlCode::Parallel_RegisterWeight:
        ESP_LOGI(TAG, "Peso corporal coletado");
        data.collectedWeight = data.weightL + data.weightR;
        return;
    case BluetoothControlCode::Parallel_Complete:
        stateManager.switchTo(StateKind::MESECollecter);
        return;
    case BluetoothControlCode::Parallel_GoBackToParameterSetup:
        stateManager.switchTo(StateKind::ParameterSetup);
        return;
    case BluetoothControlCode::FirmwareInvokeReset:
        esp_restart();
        return;
    }
}

void onParallelWeightStateExit()
{
    storeWeight();
}
