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

uint16_t weightTotal;
uint16_t residualWeightTotal;
uint16_t requestedPwm;
uint16_t mese;
uint16_t meseMax;
uint16_t setpointKg;
FlagTrigger flagTrigger;

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
    case TwaiReceivedMessageKind::WeightTotal:
      weightTotal = latestMessage.ExtraData;
      break;
    case TwaiReceivedMessageKind::ResidualWeightTotal:
      residualWeightTotal = latestMessage.ExtraData;
      break;
    case TwaiReceivedMessageKind::SetRequestedPwm:
      requestedPwm = latestMessage.ExtraData;
      break;
    case TwaiReceivedMessageKind::MeseMax:
      meseMax = latestMessage.ExtraData;
      break;
    case TwaiReceivedMessageKind::Setpoint:
      setpointKg = latestMessage.ExtraData;
      break;
    case TwaiReceivedMessageKind::Trigger:
      integralErro = 0;
      flagTrigger = latestMessage.ExtraData > 0 ? FlagTrigger::MalhaFechadaOperacao : FlagTrigger::MalhaAberta;
      break;
    case TwaiReceivedMessageKind::Mese:
      mese = latestMessage.ExtraData;
      break;
    }
  }
}

int largerPi = 0;
int calculatePulseWidth()
{
  int erro = weightTotal - setpointKg * 2;
  integralErro += erro;

  // Não deixar integralErro passar de -50 e 50
  integralErro = max(-50, min(50, integralErro));

  int controlWeight = weightTotal - residualWeightTotal;
  int pi = mese + controlWeight * 0.5f;

  // Não deixar pi passar de meseMax
  if (pi < mese) pi = mese;
  if (pi > meseMax) pi = meseMax;
  if (largerPi < mese) largerPi = mese;
  if (largerPi > meseMax) largerPi = meseMax;

  if (pi > largerPi) {
    largerPi = pi;
  }

  return largerPi;
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

  int pulseWidth = 0;
  if (flagTrigger == FlagTrigger::MalhaAberta)
  {
    pulseWidth = requestedPwm;
    // Limpar PI de qualquer operação em malha fechada anterior
    largerPi = 0;
  }
  else if (flagTrigger == FlagTrigger::MalhaFechadaOperacao)
  {
    pulseWidth = calculatePulseWidth();
  }

  // AS etapas de incremento/decremento gradual no gateway não podem ser menores que o valor nessa condição
  if (pulseWidth < 1)
    pulseWidth = 0;

  unsigned long now_micros = micros();
  if (now_micros - lastModulateTime > vDurationMicros)
  {
    modulate(pulseWidth);
    lastModulateTime = now_micros;
    vDurationMicros = (uint32_t)(((1 / 35.0) * 1000000) - ((2 * pulseWidth) - 8));
  }

  unsigned long now_ms = millis();
  if (now_ms - lastTwaiSendTime > 5)
  {
    lastTwaiSendTime = now_ms;

    twaiSend(TwaiSendMessageKind::PwmFeedbackEstimulador, (uint16_t)pulseWidth);
  }

  DEBUG(pulseWidth);
  DEBUG(weightTotal);
  DEBUG(requestedPwm);
  DEBUG(meseMax);
  DEBUG(setpointKg);
  DEBUG(flagTrigger);
  DEBUG(residualWeightTotal);
}
