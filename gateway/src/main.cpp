#include <Arduino.h>
#include <esp_log.h>
#include <BluetoothSerial.h>
#include <ArduinoBLE.h>
#include <driver/twai.h>
#include <Bluetooth/Bluetooth.h>
#include "Twai/Twai.h"
#include "Scale/Scale.h"
#include "Data.h"
#include "StateManager.h"

#define ONBOARD_LED 2

static const char *TAG = "main";

StateManager stateManager;
Data data;

void setup()
{
  Serial.begin(115200);
  scaleBeginOrDie();
  bluetoothSetup();
  twaiStart();

  twaiSend(TwaiSendMessageKind::GatewayResetHappened, 0);

  stateManager.setup(StateKind::Disconnected);

  bluetoothSetControlCallback([](BluetoothControlCode code, uint8_t extraData)
                              {
    ESP_LOGI(TAG, "Control! Code=%X ExtraData=%d\n", code, extraData);
    stateManager.onBLEControl(code, extraData); });
}

void loop()
{
  // Coletar dados das balanças
  scaleUpdate();
  data.weightL = scaleGetWeightL();
  data.weightR = scaleGetWeightR();

  bluetoothLoop();
  digitalWrite(ONBOARD_LED, bluetoothIsConnected() ? HIGH : LOW);

  // Decodar todas as mensagens na fila do CAN
  TwaiReceivedMessage twaiMessage;
  while (twaiReceive(&twaiMessage) == ESP_OK)
  {
    stateManager.onTWAIMessage(&twaiMessage);
  }

  // Spin da máquina de estados
  stateManager.loop();

  // Feedback para o telefone
  data.sendToBle();
}
