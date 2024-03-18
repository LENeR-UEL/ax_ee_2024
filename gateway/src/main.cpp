#include <Arduino.h>
#include <esp_log.h>
#include <BluetoothSerial.h>
#include <ArduinoBLE.h>
#include <driver/twai.h>
#include <Bluetooth/Bluetooth.h>
#include "Twai/Twai.h"
#include "Scale/Scale.h"
#include "Data.h"
#include "onBluetoothControl.h"
#include "StateManager.h"

#define DEBUG(variable) ESP_LOGD(TAG, #variable ": %d\n", variable)
#define ONBOARD_LED 2

static const char *TAG = "main";

StateManager stateManager;
FlagTrigger trigger;
Data data;

void onTwaiMessage(TwaiReceivedMessage *receivedTwaiMessage)
{
  switch (receivedTwaiMessage->Kind)
  {
  case TwaiReceivedMessageKind::PwmFeedbackEstimulador:
    data.pwmFeedback = receivedTwaiMessage->ExtraData;
    break;
  default:
    break;
  }
}

void setup()
{
  // pinMode(ONBOARD_LED, OUTPUT);

  espBle.onControlReceivedCallback = &onBluetoothControl;

  Serial.begin(115200);
  scaleBeginOrDie();
  espBle.startOrDie();
  twaiStart();
  stateManager.setup(StateKind::Disconnected);
}

void loop()
{
  // Na aba "Paralela", após setar o MESE, o PWM é diminuído gradualmente.
  if (data.isDecreasingPwm)
  {
    delay(250);

    if (data.pwm <= 5)
    {
      data.pwm = 0;
      data.isDecreasingPwm = false;
    }
    else
    {
      data.pwm -= 5;
    }
  }

  digitalWrite(ONBOARD_LED, espBle.isConnected() ? HIGH : LOW);
  espBle.Update();

  // Decodar todas as mensagens na fila do CAN
  TwaiReceivedMessage twaiMessage;
  while (twaiReceive(&twaiMessage) == ESP_OK)
  {
    onTwaiMessage(&twaiMessage);
  }

  // Coletar dados das balanças
  scaleUpdate();
  data.weightL = scaleGetMeasurement(Scale::C) + scaleGetMeasurement(Scale::D);
  data.weightR = scaleGetMeasurement(Scale::A) + scaleGetMeasurement(Scale::B);

  // Enviar dados para o estimulador
  data.sendToTwai();

  // Enviar dados para o telefone
  data.sendToBle(espBle);

  // data.debugPrintAll();

  stateManager.loop();
}
