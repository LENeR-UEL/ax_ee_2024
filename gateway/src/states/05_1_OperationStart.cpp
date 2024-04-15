#include <Arduino.h>
#include <esp_log.h>
#include "../Bluetooth/Bluetooth.h"
#include "../Twai/Twai.h"
#include "../Data.h"
#include "../StateManager.h"
#include "../Scale/Scale.h"
#include "./05_OperationCommon.h"

static const char *TAG = "OperationStart";
static unsigned long lastTwaiSendTime = 0;

void onOperationStartEnter()
{
    lastWeightClassChangeTime = millis();
    weightClassTimer = 0;
    data.setpoint = data.collectedWeight / 4;
}

void onOperationStartLoop()
{
    long now = millis();

    if (!espBle.isConnected())
    {
        ESP_LOGE(TAG, "Conexão Bluetooth perdida!");
        stateManager.switchTo(StateKind::OperationStop);
        return;
    }

    updateCurrentWeightClass();

    ESP_LOGD(TAG, "Peso: %d/%d", scaleGetTotalWeight(), data.setpoint * 2);

    // Aguardar peso total = setpoint * 2
    if (scaleGetTotalWeight() >= data.setpoint * 2)
    {
        ESP_LOGD(TAG, "Condição atingida.");
        stateManager.switchTo(StateKind::OperationGradualIncrease);
        return;
    }

    if (now - lastTwaiSendTime >= 100)
    {
        lastTwaiSendTime = now;
        twaiSend(TwaiSendMessageKind::SetRequestedPwm, 0);
        twaiSend(TwaiSendMessageKind::Trigger, (uint8_t)FlagTrigger::MalhaAberta);
        twaiSend(TwaiSendMessageKind::WeightL, scaleGetWeightL());
        twaiSend(TwaiSendMessageKind::WeightR, scaleGetWeightR());
        twaiSend(TwaiSendMessageKind::Setpoint, 0);
        twaiSend(TwaiSendMessageKind::Mese, 0);
        twaiSend(TwaiSendMessageKind::MeseMax, 0);
    }

    data.mainOperationStateInformApp[0] = (uint8_t)stateManager.currentKind;
    data.mainOperationStateInformApp[1] = 0;
    data.mainOperationStateInformApp[2] = 0;
    data.mainOperationStateInformApp[3] = 0;
    data.mainOperationStateInformApp[4] = 0;
    data.mainOperationStateInformApp[5] = 0;
}

void onOperationStartTWAIMessage(TwaiReceivedMessage *receivedMessage)
{
    switch (receivedMessage->Kind)
    {
    case TwaiReceivedMessageKind::PwmFeedbackEstimulador:
        data.pwmFeedback = receivedMessage->ExtraData;
        break;
    }
}

void onOperationStartBLEControl(BluetoothControlCode code, uint8_t extraData)
{
    switch (code)
    {
    case BluetoothControlCode::MainOperation_SetSetpoint:
        data.setpoint = extraData;
        break;
    case BluetoothControlCode::MainOperation_IncreaseMESEMaxOnce:
        data.meseMax += OPERATION_MESE_MAX_CHANGE_STEP;
        break;
    case BluetoothControlCode::MainOperation_DecreaseMESEMaxOnce:
        if (data.meseMax <= OPERATION_MESE_MAX_CHANGE_STEP)
        {
            data.meseMax = 0;
        }
        else
        {
            data.meseMax -= OPERATION_MESE_MAX_CHANGE_STEP;
        }
        break;
    case BluetoothControlCode::MainOperation_GoBackToMESECollecter:
        stateManager.switchTo(StateKind::MESECollecter);
        return;
    case BluetoothControlCode::FirmwareInvokeReset:
        esp_restart();
        return;
    }
}

void onOperationStartExit() {}
