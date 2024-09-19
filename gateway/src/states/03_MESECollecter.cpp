#include <Arduino.h>
#include <esp_log.h>
#include <Preferences.h>
#include "../Bluetooth/Bluetooth.h"
#include "../Twai/Twai.h"
#include "../Data.h"
#include "../StateManager.h"
#include "../Scale/Scale.h"

static const char *TAG = "MESECollecter";

#define COLLECTOR_PWM_STEP 5
#define WINDING_DOWN_PWM_STEP 5
#define WINDING_DOWN_INTERVAL_MS 200

static Preferences preferences;

void loadStoredMESE()
{
    preferences.begin("parameters", true);
    if (preferences.isKey("04mese"))
        data.mese = preferences.getUShort("04mese", 0);
    if (preferences.isKey("04mesemax"))
        data.meseMax = preferences.getUShort("04mesemax", 0);
    preferences.end();
}

void storeMESE()
{
    preferences.begin("parameters", false);
    preferences.putUShort("04mese", data.mese);
    preferences.putUShort("04mesemax", data.meseMax);
    preferences.end();
}

static unsigned long lastTwaiSendTime = 0;
static long lastWindingDownTickTime = 0;
static bool isWindingDown = false;

static uint8_t requestedPwm = 0;

void onMESECollecterStateEnter()
{
    requestedPwm = 0;
    lastWindingDownTickTime = 0;
    isWindingDown = false;
    loadStoredMESE();
}

void onMESECollecterStateLoop()
{
    if (!bluetoothIsConnected())
    {
        ESP_LOGE(TAG, "Conexão Bluetooth perdida!");
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

    if (now - lastTwaiSendTime >= 15)
    {
        lastTwaiSendTime = now;
        twaiSend(TwaiSendMessageKind::Trigger, (uint8_t)FlagTrigger::MalhaAberta);
        twaiSend(TwaiSendMessageKind::WeightTotal, scaleGetWeightL() + scaleGetWeightR());
        twaiSend(TwaiSendMessageKind::SetRequestedPwm, requestedPwm);
        twaiSend(TwaiSendMessageKind::Setpoint, data.setpoint);
        twaiSend(TwaiSendMessageKind::Mese, data.mese);
        twaiSend(TwaiSendMessageKind::MeseMax, data.meseMax);
        twaiSend(TwaiSendMessageKind::SetGainCoefficient, data.parameterSetup.gainCoefficient);
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
    case BluetoothControlCode::MESECollecter_Complete:
        stateManager.switchTo(StateKind::ParallelWeight);
        break;
    case BluetoothControlCode::MESECollecter_GoBackToParameterSetup:
        stateManager.switchTo(StateKind::ParameterSetup);
        return;
    case BluetoothControlCode::FirmwareInvokeReset:
        esp_restart();
        return;
    }
}

void onMESECollecterStateExit()
{
    storeMESE();
    data.meseMax = data.mese * 1.2f;
    data.setpoint = data.mese * 0.5f;
}
