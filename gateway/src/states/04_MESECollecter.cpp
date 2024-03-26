#include <Arduino.h>
#include <esp_log.h>
#include "../Bluetooth/Bluetooth.h"
#include "../Twai/Twai.h"
#include "../Data.h"
#include "../StateManager.h"
#include "../Scale/Scale.h"

static const char *TAG = "MESECollecter";

#define COLLECTOR_TWAI_SEND_INTERVAL_MS 100
#define COLLECTOR_PWM_STEP 15
#define WINDING_DOWN_PWM_STEP 5
#define WINDING_DOWN_INTERVAL_MS 500

static long lastTwaiSendTime = 0;
static long lastWindingDownTickTime = 0;
static bool isWindingDown = false;

static uint8_t requestedPwm = 0;

void onMESECollecterStateEnter()
{
    requestedPwm = 0;
    lastWindingDownTickTime = 0;
    isWindingDown = false;

    data.pwmFeedback = 0;
    data.mese = 0;
    data.meseMax = 0;
}

void onMESECollecterStateLoop()
{

    if (!espBle.isConnected())
    {
        stateManager.switchTo(StateKind::Disconnected);
        return;
    }

    long now = millis();

    if (isWindingDown && now - lastWindingDownTickTime >= WINDING_DOWN_INTERVAL_MS)
    {
        ESP_LOGD(TAG, "Winding down...");

        lastWindingDownTickTime = now;

        if (requestedPwm <= WINDING_DOWN_PWM_STEP)
        {
            requestedPwm = 0;
            isWindingDown = false;
        }
        else
        {
            requestedPwm -= WINDING_DOWN_PWM_STEP;
        }
    }

    if (now - lastTwaiSendTime >= COLLECTOR_TWAI_SEND_INTERVAL_MS)
    {
        lastTwaiSendTime = now;
        twaiSend(TwaiSendMessageKind::WeightL, scaleGetWeightL());
        twaiSend(TwaiSendMessageKind::WeightR, scaleGetWeightR());
        twaiSend(TwaiSendMessageKind::SetRequestedPwm, requestedPwm);
        twaiSend(TwaiSendMessageKind::Setpoint, 0);
        twaiSend(TwaiSendMessageKind::Mese, 0);
        twaiSend(TwaiSendMessageKind::MeseMax, 0);
        twaiSend(TwaiSendMessageKind::Trigger, (uint8_t)FlagTrigger::MalhaAberta);
    }
}

void onMESECollecterStateTWAIMessage(TwaiReceivedMessage *receivedMessage)
{
    switch (receivedMessage->Kind)
    {
    case TwaiReceivedMessageKind::PwmFeedbackEstimulador:
        data.pwmFeedback = receivedMessage->ExtraData;
        break;
    }
}

void onMESECollecterStateBLEControl(BluetoothControlCode code, uint8_t extraData)
{
    switch (code)
    {
    case BluetoothControlCode::MESECollecter_IncreaseOnce:
        if (requestedPwm >= (255 - COLLECTOR_PWM_STEP))
        {
            requestedPwm = 255;
        }
        else
        {
            requestedPwm += COLLECTOR_PWM_STEP;
        }
        break;
    case BluetoothControlCode::MESECollecter_DecreaseOnce:
        if (requestedPwm <= COLLECTOR_PWM_STEP)
        {
            requestedPwm = 0;
        }
        else
        {
            requestedPwm -= COLLECTOR_PWM_STEP;
        }
        break;
    case BluetoothControlCode::MESECollecter_RegisterMESE:
        lastWindingDownTickTime = millis();
        isWindingDown = true;
        data.mese = requestedPwm;
        data.meseMax = requestedPwm * 1.2f;
        break;
    case BluetoothControlCode::FirmwareInvokeReset:
        esp_restart();
        return;
    }
}

void onMESECollecterStateExit() {}
