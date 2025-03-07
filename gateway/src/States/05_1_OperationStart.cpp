#include "../Bluetooth/Bluetooth.h"
#include "../Data.h"
#include "../Scale/Scale.h"
#include "../StateManager.h"
#include "../Twai/Twai.h"
#include "./05_OperationCommon.h"
#include <Arduino.h>
#include <esp_log.h>

static const char *TAG = "OperationStart";
static unsigned long lastTwaiSendTime = 0;

void onOperationStartEnter()
{
  data.meseMax = data.mese * 1.2f;
  data.setpoint = data.collectedWeight * 0.5f;
}

void onOperationStartLoop()
{
  long now = millis();

  if (!bluetoothIsConnected())
  {
    ESP_LOGE(TAG, "Conexão Bluetooth perdida!");
    stateManager.switchTo(StateKind::OperationStop);
    return;
  }

  const unsigned short targetWeight = (data.setpoint * 2) * 0.2f;

  ESP_LOGD(TAG, "Peso: %d/%d && isOVBoxFlagSet = %s", scaleGetTotalWeight(),
           targetWeight, data.isOVBoxFlagSet() ? "sim" : "não");

  if (scaleGetTotalWeight() >= targetWeight && data.isOVBoxFlagSet())
  {
    ESP_LOGD(TAG, "Condição atingida.");
    stateManager.switchTo(StateKind::OperationGradualIncrease);
    return;
  }

  if (now - lastTwaiSendTime >= 15)
  {
    lastTwaiSendTime = now;
    twaiSend(TwaiSendMessageKind::SetRequestedPwm, 0);
    twaiSend(TwaiSendMessageKind::UseMalhaAberta, 0);
    twaiSend(TwaiSendMessageKind::WeightTotal,
             scaleGetWeightL() + scaleGetWeightR());
    twaiSend(TwaiSendMessageKind::Setpoint, 0);
    twaiSend(TwaiSendMessageKind::Mese, 0);
    twaiSend(TwaiSendMessageKind::MeseMax, 0);
    twaiSend(TwaiSendMessageKind::SetGainCoefficient, data.parameterSetup.gainCoefficient);
  }

  data.mainOperationStateInformApp[0] = (uint8_t)stateManager.currentKind;
  data.mainOperationStateInformApp[1] = targetWeight & 0xFF;
  data.mainOperationStateInformApp[2] = (targetWeight >> 8) & 0xFF;
  data.mainOperationStateInformApp[3] = 0;
  data.mainOperationStateInformApp[4] = 0;
  data.mainOperationStateInformApp[5] = 0;
}

void onOperationStartTWAIMessage(TwaiReceivedMessage *receivedMessage)
{
  switch (receivedMessage->Kind)
  {
  case TwaiReceivedMessageKind::PwmFeedbackEstimulador:
    data.pwmFeedback = receivedMessage->ExtraData;
    break;
  }
}

void onOperationStartBLEControl(BluetoothControlCode code, uint8_t extraData)
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

void onOperationStartExit() {}
