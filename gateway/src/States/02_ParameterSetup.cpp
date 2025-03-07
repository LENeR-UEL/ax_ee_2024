#include <Arduino.h>
#include <esp_log.h>
#include "../Bluetooth/Bluetooth.h"
#include "../Twai/Twai.h"
#include "../Data.h"
#include "../StateManager.h"
#include "../Scale/Scale.h"
#include "Preferences.h"

static const char *TAG = "ParameterSetup";

static unsigned long lastTwaiSendTime = 0;

#define PARAMETERS_DEFAULT_GRADUAL_INCREASE_TIME 1500
#define PARAMETERS_DEFAULT_TRANSITION_TIME 5000
#define PARAMETERS_DEFAULT_GRADUAL_DECREASE_TIME 1500
#define PARAMETERS_DEFAULT_MALHA_FECHADA_ABOVE_SETPOINT_TIME 2000
#define PARAMETERS_DEFAULT_GAIN 50

static Preferences preferences;

void reloadData(bool resetToDefaults)
{
    preferences.begin("parameters", false);
    if (resetToDefaults)
        preferences.clear();
    data.parameterSetup.gradualIncreaseTime = preferences.getUShort("a", PARAMETERS_DEFAULT_GRADUAL_INCREASE_TIME);
    data.parameterSetup.transitionTime = preferences.getUShort("b", PARAMETERS_DEFAULT_TRANSITION_TIME);
    data.parameterSetup.gradualDecreaseTime = preferences.getUShort("c", PARAMETERS_DEFAULT_GRADUAL_DECREASE_TIME);
    data.parameterSetup.malhaFechadaAboveSetpointTime = preferences.getUShort("d", PARAMETERS_DEFAULT_MALHA_FECHADA_ABOVE_SETPOINT_TIME);
    data.parameterSetup.gainCoefficient = preferences.getUChar("e", PARAMETERS_DEFAULT_GAIN);
    preferences.end();
}

void saveData()
{
    preferences.begin("parameters", false);
    preferences.putUShort("a", data.parameterSetup.gradualIncreaseTime);
    preferences.putUShort("b", data.parameterSetup.transitionTime);
    preferences.putUShort("c", data.parameterSetup.gradualDecreaseTime);
    preferences.putUShort("d", data.parameterSetup.malhaFechadaAboveSetpointTime);
    preferences.putUChar("e", data.parameterSetup.gainCoefficient);
    preferences.end();
}

void onParameterSetupStateEnter()
{
    reloadData(false);
}

void onParameterSetupStateLoop()
{
    if (!bluetoothIsConnected())
    {
        ESP_LOGE(TAG, "Conexão Bluetooth perdida!");
        stateManager.switchTo(StateKind::Disconnected);
        return;
    }

    long now = millis();
    if (now - lastTwaiSendTime >= 15)
    {
        twaiSend(TwaiSendMessageKind::UseMalhaAberta, 0);
        twaiSend(TwaiSendMessageKind::WeightTotal, scaleGetWeightL() + scaleGetWeightR());
        twaiSend(TwaiSendMessageKind::SetRequestedPwm, 0);
        twaiSend(TwaiSendMessageKind::Setpoint, 0);
        twaiSend(TwaiSendMessageKind::Mese, 0);
        twaiSend(TwaiSendMessageKind::MeseMax, 0);
        twaiSend(TwaiSendMessageKind::SetGainCoefficient, data.parameterSetup.gainCoefficient);
    }
}

void onParameterSetupStateTWAIMessage(TwaiReceivedMessage *receivedMessage)
{
    switch (receivedMessage->Kind)
    {
    case TwaiReceivedMessageKind::PwmFeedbackEstimulador:
        data.pwmFeedback = receivedMessage->ExtraData;
        break;
    }
}

void onParameterSetupStateBLEControl(BluetoothControlCode code, uint8_t extraData)
{
    switch (code)
    {
    case BluetoothControlCode::ParameterSetup_SetGradualIncreaseTime:
        data.parameterSetup.gradualIncreaseTime = max(extraData * 100, 1);
        break;
    case BluetoothControlCode::ParameterSetup_SetTransitionTime:
        data.parameterSetup.transitionTime = max(extraData * 100, 1);
        break;
    case BluetoothControlCode::ParameterSetup_SetGradualDecreaseTime:
        data.parameterSetup.gradualDecreaseTime = max(extraData * 100, 1);
        break;
    case BluetoothControlCode::ParameterSetup_SetMalhaFechadaAboveSetpointTime:
        data.parameterSetup.malhaFechadaAboveSetpointTime = max(extraData * 100, 1);
        break;
    case BluetoothControlCode::ParameterSetup_SetGainCoefficient:
        ESP_LOGI(TAG, "Coeficiente de ganho definido pelo aplicativo: %f", extraData / 100.0f);
        data.parameterSetup.gainCoefficient = extraData;
        break;
    case BluetoothControlCode::ParameterSetup_Reset:
        reloadData(true);
        break;
    case BluetoothControlCode::ParameterSetup_Save:
        // Salvar preferências na memória
        saveData();
        break;
    case BluetoothControlCode::ParameterSetup_Complete:
        stateManager.switchTo(StateKind::MESECollecter);
        return;
    case BluetoothControlCode::FirmwareInvokeReset:
        esp_restart();
        return;
    }
}

void onParameterSetupStateExit()
{
}
