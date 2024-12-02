#include <Arduino.h>
#include <esp_log.h>
#include "Twai/Twai.h"
#include "StateManager.h"
#include "Data.h"

#define DEBUG(variable) ESP_LOGI(TAG, #variable ": %d", variable)

static const char *TAG = "main";

StateManager stateManager;
Data data;

void setup()
{
  dataReset();

  Serial.begin(115200);
  twaiStart();

  pinMode(2, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(32, OUTPUT);
  pinMode(33, OUTPUT);

  digitalWrite(2, LOW);
  digitalWrite(4, LOW);
  digitalWrite(32, LOW);
  digitalWrite(33, LOW);

  stateManager.setup(StateKind::WorkingMalhaAbertaState);
}

void loop()
{
  // Receber todas as mensagens na fila do CAN
  TwaiReceivedMessage latestMessage;
  while (twaiReceive(&latestMessage) == ESP_OK)
  {
    stateManager.onTWAIMessage(&latestMessage);
  }

  // twaiSend(TwaiSendMessageKind::PwmFeedbackEstimulador, 0);

  stateManager.loop();
}
