#include <Arduino.h>
#include <esp_log.h>
#include "Twai/Twai.h"

#define DEBUG(variable) ESP_LOGI(TAG, #variable ": %d", variable)

static const char *TAG = "main";

TwaiReceivedMessage latestMessage;

enum class FlagTrigger
{
  MalhaAberta,
  MalhaFechadaOperacao
};

uint16_t weightL;
uint16_t weightR;
uint16_t requestedPwm;
uint16_t meseMax;
uint16_t setpointKg;
FlagTrigger flagTrigger;

const float KP = 1.4;
const float KI = 2.8;

int integralErro = 0;

unsigned long lastTwaiSendTime = 0;
unsigned long lastModulateTime = 0;
uint32_t vDurationMicros = 0;

void setup()
{
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
}

void readEverythingFromTwai()
{
  // Receber todas as mensagens na fila do CAN
  while (twaiReceive(&latestMessage) == ESP_OK)
  {
    // Árvore de decisão com base no controle das mensagens
    switch (latestMessage.Kind)
    {
    case WeightL:
      weightL = latestMessage.ExtraData;
      break;
    case WeightR:
      weightR = latestMessage.ExtraData;
      break;
    case SetRequestedPwm:
      requestedPwm = latestMessage.ExtraData;
      break;
    case MeseMax:
      meseMax = latestMessage.ExtraData;
      break;
    case Setpoint:
      setpointKg = latestMessage.ExtraData;
      break;
    case Trigger:
      integralErro = 0;
      flagTrigger = latestMessage.ExtraData > 0 ? FlagTrigger::MalhaFechadaOperacao : FlagTrigger::MalhaAberta;
      break;
    case Mese:
      break;
    }
  }
}

int calculateFrequency()
{
  int pesoTotal = weightL + weightR;
  int erro = pesoTotal - setpointKg;
  integralErro += erro;

  // Não deixar integralErro passar de -50 e 50
  integralErro = max(-50, min(50, integralErro));

  int pi = KP * erro + KI * integralErro;

  // Não deixar pi passar de meseMax
  pi = min(pi, (int)meseMax);

  return pi;
}

void modulate(int pwm)
{
  if (pwm >= 10)
  {
    digitalWrite(2, HIGH);
    digitalWrite(32, HIGH);
    delayMicroseconds(pwm);
    digitalWrite(2, LOW);
    digitalWrite(32, LOW);
    delayMicroseconds(4);

    digitalWrite(4, HIGH);
    digitalWrite(33, HIGH);
    delayMicroseconds(pwm);
    digitalWrite(4, LOW);
    digitalWrite(33, LOW);
    delayMicroseconds(4);
  }
  else
  {
    digitalWrite(2, LOW);
    digitalWrite(4, LOW);
    digitalWrite(32, LOW);
    digitalWrite(33, LOW);
    delayMicroseconds(8);
  }
}

void loop()
{
  readEverythingFromTwai();

  int freq = 0;
  if (flagTrigger == FlagTrigger::MalhaAberta)
  {
    freq = requestedPwm;
  }
  else if (flagTrigger == FlagTrigger::MalhaFechadaOperacao)
  {
    freq = calculateFrequency();
  }

  if (freq < 5)
    freq = 0;

  unsigned long now_micros = micros();
  if (now_micros - lastModulateTime > vDurationMicros)
  {
    modulate(freq);
    lastModulateTime = now_micros;
    vDurationMicros = (uint32_t)(((1 / 35.0) * 1000000) - ((2 * freq) - 8));
  }

  unsigned long now_ms = millis();
  if (now_ms - lastTwaiSendTime > 10)
  {
    lastTwaiSendTime = now_ms;

    twaiSend(TwaiSendMessageKind::PwmFeedbackEstimulador, (uint16_t)freq);
  }

  DEBUG(freq);
  DEBUG(weightL);
  DEBUG(weightR);
  DEBUG(requestedPwm);
  DEBUG(meseMax);
  DEBUG(setpointKg);
  DEBUG(flagTrigger);
  ESP_LOGI(TAG, "\n");
}
