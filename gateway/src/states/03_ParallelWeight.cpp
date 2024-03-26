#include <Arduino.h>
#include <esp_log.h>
#include "../Bluetooth/Bluetooth.h"
#include "../Twai/Twai.h"
#include "../Data.h"
#include "../StateManager.h"
#include "../Scale/Scale.h"

static const char *TAG = "ParallelWeight";
static long lastTwaiSendTime = 0;

void onParallelWeightStateEnter()
{
}

void onParallelWeightStateLoop()
{
    if (!espBle.isConnected())
    {
        stateManager.switchTo(StateKind::Disconnected);
        return;
    }

    long now = millis();
    if (now - lastTwaiSendTime >= 100)
    {
        lastTwaiSendTime = now;
        twaiSend(TwaiSendMessageKind::WeightL, scaleGetWeightL());
        twaiSend(TwaiSendMessageKind::WeightR, scaleGetWeightR());
        twaiSend(TwaiSendMessageKind::SetRequestedPwm, 0);
        twaiSend(TwaiSendMessageKind::Setpoint, 0);
        twaiSend(TwaiSendMessageKind::Mese, 0);
        twaiSend(TwaiSendMessageKind::MeseMax, 0);
        twaiSend(TwaiSendMessageKind::Trigger, (uint8_t)FlagTrigger::MalhaAberta);
    }
}

void onParallelWeightStateTWAIMessage(TwaiReceivedMessage *receivedMessage) {}

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
    case BluetoothControlCode::FirmwareInvokeReset:
        esp_restart();
        return;
    }
}

void onParallelWeightStateExit() {}
