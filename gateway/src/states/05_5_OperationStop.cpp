#include <Arduino.h>
#include <esp_log.h>
#include "../Bluetooth/Bluetooth.h"
#include "../Twai/Twai.h"
#include "../Data.h"
#include "../StateManager.h"
#include "../Scale/Scale.h"
#include "./05_OperationCommon.h"

static const char *TAG = "OperationStop";
static unsigned long lastTwaiSendTime = 0;
static unsigned long lastStepTime = 0;

static uint16_t gradualDecreaseInterval;

void onOperationStopEnter()
{
    lastStepTime = millis();
    gradualDecreaseInterval = data.parameterSetup.gradualDecreaseTime / data.mese;
}

void onOperationStopLoop()
{
    long now = millis();

    ESP_LOGD(TAG, "PWM: %d/0", data.pwmFeedback, 0);

    int requestedPwm = data.pwmFeedback - 1;
    if (requestedPwm < 0)
    {
        requestedPwm = 0;
    }

    unsigned int pwmDecreaseTimeDelta = now - lastStepTime;
    if (pwmDecreaseTimeDelta >= gradualDecreaseInterval)
    {
        twaiSend(TwaiSendMessageKind::SetRequestedPwm, requestedPwm);
        twaiSend(TwaiSendMessageKind::Trigger, (uint8_t)FlagTrigger::MalhaAberta);
        lastStepTime = now;
    }

    data.mainOperationStateInformApp[0] = (uint8_t)stateManager.currentKind;
    data.mainOperationStateInformApp[1] = pwmDecreaseTimeDelta & 0xFF;
    data.mainOperationStateInformApp[2] = (pwmDecreaseTimeDelta >> 8) & 0xFF;
    data.mainOperationStateInformApp[3] = 0;
    data.mainOperationStateInformApp[4] = 0;
    data.mainOperationStateInformApp[5] = 0;
}

void onOperationStopTWAIMessage(TwaiReceivedMessage *receivedMessage)
{
    switch (receivedMessage->Kind)
    {
    case TwaiReceivedMessageKind::PwmFeedbackEstimulador:
        data.pwmFeedback = receivedMessage->ExtraData;
        break;
    }
}

void onOperationStopBLEControl(BluetoothControlCode code, uint8_t extraData)
{
    switch (code)
    {
    case BluetoothControlCode::MainOperation_GoBackToMESECollecter:
        stateManager.switchTo(StateKind::MESECollecter);
        return;
    case BluetoothControlCode::FirmwareInvokeReset:
        esp_restart();
        return;
    }
}

void onOperationStopExit() {}
