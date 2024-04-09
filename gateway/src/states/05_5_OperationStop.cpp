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

static const uint8_t DECREASE_PWM_STEP = 1;
static const uint16_t PWM_STEP_INTERVAL_MS = 50;
static unsigned long lastStepTime = 0;

void onOperationStopEnter()
{
    lastStepTime = millis();
}

void onOperationStopLoop()
{
    long now = millis();

    ESP_LOGD(TAG, "PWM: %d/0", data.pwmFeedback, 0);

    int requestedPwm = data.pwmFeedback - DECREASE_PWM_STEP;
    if (requestedPwm < 0)
    {
        requestedPwm = 0;
    }

    if (now - lastStepTime >= PWM_STEP_INTERVAL_MS)
    {
        twaiSend(TwaiSendMessageKind::SetRequestedPwm, requestedPwm);
        twaiSend(TwaiSendMessageKind::Trigger, (uint8_t)FlagTrigger::MalhaAberta);
        lastStepTime = now;
    }
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
