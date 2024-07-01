#include <Arduino.h>
#include <esp_log.h>
#include "../Bluetooth/Bluetooth.h"
#include "../Twai/Twai.h"
#include "../Data.h"
#include "../StateManager.h"
#include "../Scale/Scale.h"
#include "./05_OperationCommon.h"

static const char *TAG = "OperationGradualIncrease";
static unsigned long lastTwaiSendTime = 0;
static unsigned long lastStepTime = 0;

static uint16_t gradualIncreaseInterval;

void onOperationGradualIncreaseEnter()
{
    lastWeightClassChangeTime = millis();
    weightClassTimer = 0;
    lastStepTime = millis();
    gradualIncreaseInterval = data.parameterSetup.gradualIncreaseTime / data.mese;
}

void onOperationGradualIncreaseLoop()
{
    long now = millis();

    if (!espBle.isConnected())
    {
        ESP_LOGE(TAG, "Conexão Bluetooth perdida!");
        stateManager.switchTo(StateKind::OperationStop);
        return;
    }

    updateCurrentWeightClass();

    ESP_LOGD(TAG, "PWM: %d/%d", data.pwmFeedback, data.mese);

    if (data.pwmFeedback >= data.mese)
    {
        ESP_LOGD(TAG, "Condição atingida.");
        stateManager.switchTo(StateKind::OperationTransition);
        return;
    }

    ESP_LOGI(TAG, "Interval: %d\n", gradualIncreaseInterval);

    unsigned int pwmIncreaseTimeDelta = now - lastStepTime;
    if (pwmIncreaseTimeDelta >= gradualIncreaseInterval)
    {
        twaiSend(TwaiSendMessageKind::SetRequestedPwm, data.pwmFeedback + 1);
        twaiSend(TwaiSendMessageKind::Trigger, (uint8_t)FlagTrigger::MalhaAberta);
        lastStepTime = now;
    }

    if (now - lastTwaiSendTime >= 15)
    {
        lastTwaiSendTime = now;
        twaiSend(TwaiSendMessageKind::WeightTotal, scaleGetWeightL() + scaleGetWeightR());
        twaiSend(TwaiSendMessageKind::Setpoint, 0);
        twaiSend(TwaiSendMessageKind::Mese, 0);
        twaiSend(TwaiSendMessageKind::MeseMax, 0);
    }

    data.mainOperationStateInformApp[0] = (uint8_t)stateManager.currentKind;
    data.mainOperationStateInformApp[1] = pwmIncreaseTimeDelta & 0xFF;
    data.mainOperationStateInformApp[2] = (pwmIncreaseTimeDelta >> 8) & 0xFF;
    data.mainOperationStateInformApp[3] = 0;
    data.mainOperationStateInformApp[4] = 0;
    data.mainOperationStateInformApp[5] = 0;
}

void onOperationGradualIncreaseTWAIMessage(TwaiReceivedMessage *receivedMessage)
{
    switch (receivedMessage->Kind)
    {
    case TwaiReceivedMessageKind::PwmFeedbackEstimulador:
        data.pwmFeedback = receivedMessage->ExtraData;
        break;
    }
}

void onOperationGradualIncreaseBLEControl(BluetoothControlCode code, uint8_t extraData)
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
    case BluetoothControlCode::MainOperation_GoBackToParallel:
        stateManager.switchTo(StateKind::ParallelWeight);
        return;
    case BluetoothControlCode::MainOperation_EmergencyStop:
        stateManager.switchTo(StateKind::OperationStop);
        return;
    case BluetoothControlCode::FirmwareInvokeReset:
        esp_restart();
        return;
    }
}

void onOperationGradualIncreaseExit() {}
