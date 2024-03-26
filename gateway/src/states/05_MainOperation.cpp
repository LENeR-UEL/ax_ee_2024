#include <Arduino.h>
#include <esp_log.h>
#include "../Bluetooth/Bluetooth.h"
#include "../Twai/Twai.h"
#include "../Data.h"
#include "../StateManager.h"
#include "../Scale/Scale.h"

static const char *TAG = "MainOperation";

#define MAIN_OPERATION_TWAI_SEND_INTERVAL_MS 100
#define MAIN_OPERATION_MESE_MAX_STEP 5

static long lastTwaiSendTime;

void onMainOperationStateEnter()
{
    lastTwaiSendTime = 0;
}

void onMainOperationStateLoop()
{
    if (!espBle.isConnected())
    {
        stateManager.switchTo(StateKind::Disconnected);
        return;
    }

    long now = millis();

    if (now - lastTwaiSendTime >= MAIN_OPERATION_TWAI_SEND_INTERVAL_MS)
    {
        lastTwaiSendTime = now;
        twaiSend(TwaiSendMessageKind::WeightL, scaleGetWeightL());
        twaiSend(TwaiSendMessageKind::WeightR, scaleGetWeightR());
        // Não usado pelo estimulador; ele calcula seu próprio PWM quando Trigger = MalhaFechadaOperacao
        // twaiSend(TwaiSendMessageKind::SetRequestedPwm, 0);
        twaiSend(TwaiSendMessageKind::Setpoint, data.setpoint);
        twaiSend(TwaiSendMessageKind::Mese, data.mese);
        twaiSend(TwaiSendMessageKind::MeseMax, data.meseMax);
        twaiSend(TwaiSendMessageKind::Trigger, (uint8_t)FlagTrigger::MalhaFechadaOperacao);
    }
}

void onMainOperationStateTWAIMessage(TwaiReceivedMessage *receivedMessage)
{
    switch (receivedMessage->Kind)
    {
    case TwaiReceivedMessageKind::PwmFeedbackEstimulador:
        data.pwmFeedback = receivedMessage->ExtraData;
        break;
    }
}

void onMainOperationStateBLEControl(BluetoothControlCode code, uint8_t extraData)
{
    switch (code)
    {
    case BluetoothControlCode::MainOperation_SetSetpoint:
        data.setpoint = extraData;
        break;
    case BluetoothControlCode::MainOperation_IncreaseMESEMaxOnce:
        data.meseMax += MAIN_OPERATION_MESE_MAX_STEP;
        break;
    case BluetoothControlCode::MainOperation_DecreaseMESEMaxOnce:
        if (data.meseMax <= MAIN_OPERATION_MESE_MAX_STEP)
        {
            data.meseMax = 0;
        }
        else
        {
            data.meseMax -= MAIN_OPERATION_MESE_MAX_STEP;
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

void onMainOperationStateExit() {}
