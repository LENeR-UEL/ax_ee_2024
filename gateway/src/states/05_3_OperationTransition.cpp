#include <Arduino.h>
#include <esp_log.h>
#include "../Bluetooth/Bluetooth.h"
#include "../Twai/Twai.h"
#include "../Data.h"
#include "../StateManager.h"
#include "../Scale/Scale.h"
#include "./05_OperationCommon.h"

static const char *TAG = "OperationTransition";
static unsigned long lastTwaiSendTime = 0;

void onOperationTransitionEnter()
{
    lastWeightClassChangeTime = 0;
    weightClassTimer = 0;
}

void onOperationTransitionLoop()
{
    long now = millis();

    if (!espBle.isConnected())
    {
        ESP_LOGE(TAG, "Conexão Bluetooth perdida!");
        stateManager.switchTo(StateKind::OperationStop);
        return;
    }

    updateCurrentWeightClass();

    ESP_LOGD(TAG, "Transição... Aguardando classe de peso 0 durante %dms. Timer: %d Classe atual: %d", data.parameterSetup.transitionTime, weightClassTimer, weightClass);

    if (weightClass == 0 && weightClassTimer >= data.parameterSetup.transitionTime)
    {
        ESP_LOGD(TAG, "Condição atingida.");
        stateManager.switchTo(StateKind::OperationMalhaFechada);
        return;
    }

    if (now - lastTwaiSendTime >= 15)
    {
        lastTwaiSendTime = now;
        twaiSend(TwaiSendMessageKind::SetRequestedPwm, data.mese);
        twaiSend(TwaiSendMessageKind::Trigger, (uint8_t)FlagTrigger::MalhaAberta);
        twaiSend(TwaiSendMessageKind::WeightTotal, scaleGetWeightL() + scaleGetWeightR());
        twaiSend(TwaiSendMessageKind::Setpoint, data.setpoint);
        twaiSend(TwaiSendMessageKind::Mese, data.mese);
        twaiSend(TwaiSendMessageKind::MeseMax, data.meseMax);
    }

    data.mainOperationStateInformApp[0] = (uint8_t)stateManager.currentKind;
    data.mainOperationStateInformApp[1] = weightClass;
    data.mainOperationStateInformApp[2] = weightClassTimer & 0xFF;
    data.mainOperationStateInformApp[3] = (weightClassTimer >> 8) & 0xFF;
    data.mainOperationStateInformApp[4] = 0;
    data.mainOperationStateInformApp[5] = 0;
}

void onOperationTransitionTWAIMessage(TwaiReceivedMessage *receivedMessage)
{
    switch (receivedMessage->Kind)
    {
    case TwaiReceivedMessageKind::PwmFeedbackEstimulador:
        data.pwmFeedback = receivedMessage->ExtraData;
        break;
    }
}

void onOperationTransitionBLEControl(BluetoothControlCode code, uint8_t extraData)
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
    case BluetoothControlCode::MainOperation_EmergencyStop:
        stateManager.switchTo(StateKind::OperationStop);
        return;
    case BluetoothControlCode::FirmwareInvokeReset:
        esp_restart();
        return;
    }
}

void onOperationTransitionExit() {}
