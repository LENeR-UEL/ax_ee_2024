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

#define PARAMETERS_DEFAULT_GRADUAL_INCREASE_INTERVAL 100
#define PARAMETERS_DEFAULT_GRADUAL_INCREASE_STEP 1
#define PARAMETERS_DEFAULT_TRANSITION_TIME 1500
#define PARAMETERS_DEFAULT_GRADUAL_DECREASE_INTERVAL 100
#define PARAMETERS_DEFAULT_GRADUAL_DECREASE_STEP 1
#define PARAMETERS_DEFAULT_MALHA_FECHADA_ABOVE_SETPOINT_TIME 2000

static Preferences preferences;

void reloadData(bool resetToDefaults)
{
    preferences.begin("parameters", false);
    if (resetToDefaults)
        preferences.clear();
    data.parameterSetup.gradualIncreaseInterval = preferences.getUShort("a", PARAMETERS_DEFAULT_GRADUAL_INCREASE_INTERVAL);
    data.parameterSetup.gradualIncreaseStep = preferences.getUChar("b", PARAMETERS_DEFAULT_GRADUAL_INCREASE_STEP);
    data.parameterSetup.transitionTime = preferences.getUShort("c", PARAMETERS_DEFAULT_TRANSITION_TIME);
    data.parameterSetup.gradualDecreaseInterval = preferences.getUShort("d", PARAMETERS_DEFAULT_GRADUAL_DECREASE_INTERVAL);
    data.parameterSetup.gradualDecreaseStep = preferences.getUChar("e", PARAMETERS_DEFAULT_GRADUAL_DECREASE_STEP);
    data.parameterSetup.malhaFechadaAboveSetpointTime = preferences.getUShort("f", PARAMETERS_DEFAULT_MALHA_FECHADA_ABOVE_SETPOINT_TIME);
    preferences.end();
}

void saveData()
{
    preferences.begin("parameters", false);
    preferences.putUShort("a", data.parameterSetup.gradualIncreaseInterval);
    preferences.putUChar("b", data.parameterSetup.gradualIncreaseStep);
    preferences.putUShort("c", data.parameterSetup.transitionTime);
    preferences.putUShort("d", data.parameterSetup.gradualDecreaseInterval);
    preferences.putUChar("e", data.parameterSetup.gradualDecreaseStep);
    preferences.putUShort("f", data.parameterSetup.malhaFechadaAboveSetpointTime);
    preferences.end();
}

void onParameterSetupStateEnter()
{
    reloadData(false);
}

void onParameterSetupStateLoop()
{
    if (!espBle.isConnected())
    {
        stateManager.switchTo(StateKind::Disconnected);
        return;
    }

    ESP_LOGI(TAG, "Loop");

    long now = millis();
    if (now - lastTwaiSendTime >= 40)
    {
        twaiSend(TwaiSendMessageKind::WeightTotal, scaleGetWeightL() + scaleGetWeightR());
        twaiSend(TwaiSendMessageKind::SetRequestedPwm, 0);
        twaiSend(TwaiSendMessageKind::Setpoint, 0);
        twaiSend(TwaiSendMessageKind::Mese, 0);
        twaiSend(TwaiSendMessageKind::MeseMax, 0);
        twaiSend(TwaiSendMessageKind::Trigger, (uint8_t)FlagTrigger::MalhaAberta);
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
    case BluetoothControlCode::ParameterSetup_SetGradualIncreaseStep:
        data.parameterSetup.gradualIncreaseStep = extraData;
        break;
    case BluetoothControlCode::ParameterSetup_SetGradualIncreaseInterval:
        data.parameterSetup.gradualIncreaseInterval = extraData * 50;
        break;
    case BluetoothControlCode::ParameterSetup_SetTransitionTime:
        data.parameterSetup.transitionTime = extraData * 100;
        break;
    case BluetoothControlCode::ParameterSetup_SetGradualDecreaseInterval:
        data.parameterSetup.gradualDecreaseInterval = extraData * 50;
        break;
    case BluetoothControlCode::ParameterSetup_SetGradualDecreaseStep:
        data.parameterSetup.gradualDecreaseStep = extraData;
        break;
    case BluetoothControlCode::ParameterSetup_SetMalhaFechadaAboveSetpointTime:
        data.parameterSetup.malhaFechadaAboveSetpointTime = extraData * 100;
        break;
    case BluetoothControlCode::ParameterSetup_Reset:
        reloadData(true);
        break;
    case BluetoothControlCode::ParameterSetup_Save:
        // Salvar preferências na memória
        saveData();
        break;
    case BluetoothControlCode::ParameterSetup_Complete:
        stateManager.switchTo(StateKind::ParallelWeight);
        return;
    case BluetoothControlCode::FirmwareInvokeReset:
        esp_restart();
        return;
    }
}

void onParameterSetupStateExit()
{
}
