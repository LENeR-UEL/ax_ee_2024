#include <Arduino.h>
#include <esp_log.h>
#include "../Bluetooth/Bluetooth.h"
#include "../Twai/Twai.h"
#include "../Data.h"
#include "../StateManager.h"
#include "../Scale/Scale.h"
#include "./05_OperationCommon.h"

static const char *TAG = "OperationMalhaFechada";
static unsigned long lastTwaiSendTime = 0;

// Instante mais recente onde o valor de erro era negativo
// Usado para calcular quanto tempo o erro está positivo, no estado de malha fechada
static long calculatedErrorValueLastNegativeTime = 0;

void onOperationMalhaFechadaEnter()
{
    calculatedErrorValueLastNegativeTime = millis();
}

void onOperationMalhaFechadaLoop()
{
    long now = millis();

    if (!espBle.isConnected())
    {
        ESP_LOGE(TAG, "Conexão Bluetooth perdida!");
        stateManager.switchTo(StateKind::OperationStop);
        return;
    }

    short currentErrorValue = scaleGetTotalWeight() - data.setpoint * 2;
    if (currentErrorValue < 0)
    {
        calculatedErrorValueLastNegativeTime = now;
    }

    ESP_LOGD(TAG, "Operação... Aguardando erro negativo durante %dms. Delta = %d ms, erro = %d", data.parameterSetup.malhaFechadaAboveSetpointTime, now - calculatedErrorValueLastNegativeTime, currentErrorValue);

    // Erro positivo durante 2000ms?
    unsigned short delta = now - calculatedErrorValueLastNegativeTime;
    if (delta >= data.parameterSetup.malhaFechadaAboveSetpointTime)
    {
        ESP_LOGD(TAG, "Condição atingida.");
        stateManager.switchTo(StateKind::OperationStop);
        return;
    }

    if (now - lastTwaiSendTime >= 15)
    {
        lastTwaiSendTime = now;
        // Malha fechada; PWM enviado não importa; é calculado pelo firmware do estimulador
        // twaiSend(TwaiSendMessageKind::SetRequestedPwm, data.pwmFeedback);
        twaiSend(TwaiSendMessageKind::Trigger, (uint8_t)FlagTrigger::MalhaFechadaOperacao);
        twaiSend(TwaiSendMessageKind::MeseMax, data.meseMax);
        twaiSend(TwaiSendMessageKind::WeightTotal, scaleGetWeightL() + scaleGetWeightR());
        twaiSend(TwaiSendMessageKind::Setpoint, data.setpoint);
        twaiSend(TwaiSendMessageKind::Mese, data.mese);
    }

    data.mainOperationStateInformApp[0] = (uint8_t)stateManager.currentKind;
    // little-endian
    data.mainOperationStateInformApp[1] = currentErrorValue & 0xFF;
    data.mainOperationStateInformApp[2] = (currentErrorValue >> 8) & 0xFF;
    data.mainOperationStateInformApp[3] = delta & 0xFF;
    data.mainOperationStateInformApp[4] = (delta >> 8) & 0xFF;
    data.mainOperationStateInformApp[5] = 0;
}

void onOperationMalhaFechadaTWAIMessage(TwaiReceivedMessage *receivedMessage)
{
    switch (receivedMessage->Kind)
    {
    case TwaiReceivedMessageKind::PwmFeedbackEstimulador:
        data.pwmFeedback = receivedMessage->ExtraData;
        break;
    }
}

void onOperationMalhaFechadaBLEControl(BluetoothControlCode code, uint8_t extraData)
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

void onOperationMalhaFechadaExit() {}
