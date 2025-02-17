#include "../Bluetooth/Bluetooth.h"
#include "../Data.h"
#include "../Scale/Scale.h"
#include "../StateManager.h"
#include "../Twai/Twai.h"
#include <Arduino.h>
#include <Preferences.h>
#include <esp_log.h>

static const char *TAG = "ParallelWeight";
static unsigned long lastTwaiSendTime = 0;

static Preferences preferences;

void loadStoredWeight()
{
  preferences.begin("parameters", true);
  if (preferences.isKey("03weight"))
  {
    data.collectedWeight = preferences.getUShort("03weight", 0);
  }
  preferences.end();
}

void storeWeight()
{
  preferences.begin("parameters", false);
  preferences.putUShort("03weight", data.collectedWeight);
  preferences.end();
}

void onParallelWeightStateEnter() { loadStoredWeight(); }

void onParallelWeightStateLoop()
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
    lastTwaiSendTime = now;
    twaiSend(TwaiSendMessageKind::UseMalhaAberta, 0);
    twaiSend(TwaiSendMessageKind::WeightTotal,
             scaleGetWeightL() + scaleGetWeightR());
    twaiSend(TwaiSendMessageKind::SetRequestedPwm, 0);
    twaiSend(TwaiSendMessageKind::Setpoint, 0);
    twaiSend(TwaiSendMessageKind::Mese, 0);
    twaiSend(TwaiSendMessageKind::MeseMax, 0);
    twaiSend(TwaiSendMessageKind::SetGainCoefficient, data.parameterSetup.gainCoefficient);
  }
}

void onParallelWeightStateTWAIMessage(TwaiReceivedMessage *receivedMessage)
{
  switch (receivedMessage->Kind)
  {
  case TwaiReceivedMessageKind::PwmFeedbackEstimulador:
    data.pwmFeedback = receivedMessage->ExtraData;
    break;
  }
}

void onParallelWeightStateBLEControl(BluetoothControlCode code,
                                     uint8_t extraData)
{
  switch (code)
  {
  case BluetoothControlCode::Parallel_RegisterWeight:
    ESP_LOGI(TAG, "Peso corporal coletado");
    data.collectedWeight = data.weightL + data.weightR;
    return;
  case BluetoothControlCode::Parallel_Complete:
    stateManager.switchTo(StateKind::OperationStart);
    return;
  case BluetoothControlCode::Parallel_GoBackToMESECollecter:
    stateManager.switchTo(StateKind::MESECollecter);
    return;
  case BluetoothControlCode::Parallel_SetWeightFromArgument:
    ESP_LOGI(TAG, "Peso corporal definido pelo aplicativo");
    data.collectedWeight = extraData;
    return;
  case BluetoothControlCode::FirmwareInvokeReset:
    esp_restart();
    return;
  }
}

void onParallelWeightStateExit() { storeWeight(); }
