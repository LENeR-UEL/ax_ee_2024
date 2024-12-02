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
static unsigned long timer;

void onOperationTransitionEnter()
{
    timer = millis();
}

void onOperationTransitionLoop()
{
    long now = millis();

    if (!bluetoothIsConnected())
    {
        ESP_LOGE(TAG, "Conexão Bluetooth perdida!");
        stateManager.switchTo(StateKind::OperationStop);
        return;
    }

    unsigned long delta = now - timer;
    ESP_LOGD(TAG, "Transição... Aguardando %dms. Timer: %d", data.parameterSetup.transitionTime, delta);
    if (delta >= data.parameterSetup.transitionTime)
    {
        ESP_LOGD(TAG, "Condição atingida.");
        stateManager.switchTo(StateKind::OperationMalhaFechada);
        return;
    }

    if (now - lastTwaiSendTime >= 15)
    {
        lastTwaiSendTime = now;

        // Peso residual: peso coletado no final da etapa de transição
        // Mandamos a todo instante durante a etapa de transição, e ao mudar para o próximo estado,
        // teremos o peso residual do final da etapa de transição
        twaiSend(TwaiSendMessageKind::ResidualWeightTotal, scaleGetTotalWeight());

        twaiSend(TwaiSendMessageKind::SetRequestedPwm, data.mese);
        twaiSend(TwaiSendMessageKind::UseMalhaAberta, 0);
        twaiSend(TwaiSendMessageKind::WeightTotal, scaleGetTotalWeight());
        twaiSend(TwaiSendMessageKind::Setpoint, data.setpoint);
        twaiSend(TwaiSendMessageKind::Mese, data.mese);
        twaiSend(TwaiSendMessageKind::MeseMax, data.meseMax);
        twaiSend(TwaiSendMessageKind::SetGainCoefficient, data.parameterSetup.gainCoefficient);
    }

    data.mainOperationStateInformApp[0] = (uint8_t)stateManager.currentKind;
    data.mainOperationStateInformApp[1] = delta & 0xFF;
    data.mainOperationStateInformApp[2] = (delta >> 8) & 0xFF;
    data.mainOperationStateInformApp[3] = 0;
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

void onOperationTransitionExit() {}
