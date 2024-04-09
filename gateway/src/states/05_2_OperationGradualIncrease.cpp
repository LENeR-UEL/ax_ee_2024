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

static const uint8_t PWM_STEP = 1;
static const uint16_t PWM_STEP_INTERVAL_MS = 100;
static unsigned long lastStepTime = 0;

void onOperationGradualIncreaseEnter()
{
    lastWeightClassChangeTime = millis();
    weightClassTimer = 0;
    lastStepTime = millis();
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

    if (now - lastStepTime >= PWM_STEP_INTERVAL_MS)
    {
        twaiSend(TwaiSendMessageKind::SetRequestedPwm, data.pwmFeedback + PWM_STEP);
        twaiSend(TwaiSendMessageKind::Trigger, (uint8_t)FlagTrigger::MalhaAberta);
        lastStepTime = now;
    }

    if (now - lastTwaiSendTime >= 100)
    {
        lastTwaiSendTime = now;
        twaiSend(TwaiSendMessageKind::WeightL, scaleGetWeightL());
        twaiSend(TwaiSendMessageKind::WeightR, scaleGetWeightR());
        twaiSend(TwaiSendMessageKind::Setpoint, 0);
        twaiSend(TwaiSendMessageKind::Mese, 0);
        twaiSend(TwaiSendMessageKind::MeseMax, 0);
    }
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
    case BluetoothControlCode::MainOperation_GoBackToMESECollecter:
        stateManager.switchTo(StateKind::MESECollecter);
        return;
    case BluetoothControlCode::FirmwareInvokeReset:
        esp_restart();
        return;
    }
}

void onOperationGradualIncreaseExit() {}
