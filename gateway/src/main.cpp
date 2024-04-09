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
FlagTrigger trigger;
Data data;

void onBluetoothControl(uint16_t fullPayload)
{
  BluetoothControlCode code = (BluetoothControlCode)(fullPayload & 0x00FF);
  uint8_t extraData = (fullPayload & 0xFF00) >> 8;

  ESP_LOGI(TAG, "Control! Code=%X ExtraData=%d\n", code, extraData);

  stateManager.onBLEControl(code, extraData);
}

void setup()
{
  espBle.onControlReceivedCallback = &onBluetoothControl;

  Serial.begin(115200);
  scaleBeginOrDie();
  espBle.startOrDie();
  twaiStart();

  stateManager.setup(StateKind::Disconnected);
}

void loop()
{
  // Coletar dados das balanças
  scaleUpdate();
  data.weightL = scaleGetMeasurement(Scale::C) + scaleGetMeasurement(Scale::D);
  data.weightR = scaleGetMeasurement(Scale::A) + scaleGetMeasurement(Scale::B);

  espBle.Update();
  digitalWrite(ONBOARD_LED, espBle.isConnected() ? HIGH : LOW);

  // Decodar todas as mensagens na fila do CAN
  TwaiReceivedMessage twaiMessage;
  while (twaiReceive(&twaiMessage) == ESP_OK)
  {
    stateManager.onTWAIMessage(&twaiMessage);
  }

  // Spin da máquina de estados
  stateManager.loop();

  // Feedback para o telefone
  data.sendToBle(espBle);
}
