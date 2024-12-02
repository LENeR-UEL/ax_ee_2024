#include "../Data.h"
#include "../Modulator.h"
#include "../StateManager.h"
#include "../Twai/Twai.h"
#include <Arduino.h>

static unsigned long lastTwaiSendTime = 0;
static unsigned long lastTwaiRecvTime = 0;

static int largerPi = 0;
static int integralErro = 0;

int calculatePulseWidth()
{
  // O setpoint é setado no aplicativo considerando apenas um dos lados devido à escala das barras. Para o controle, devemos considerar o setpoint "total" das duas barras.
  const int setpoint = data.setpointKg * 2;

  int pesoControle =
      data.weightTotal - data.residualWeightTotal; // peso sem "ruído"

  int erroControle = pesoControle - setpoint * 0.1f;

  int pi =
      pesoControle * data.gainCoefficient + data.mese + 0.3f * erroControle;

  int maximo = data.meseMax;
  int minimo = maximo * 0.8f;

  if (pi < minimo)
    pi = minimo;
  if (pi > maximo)
    pi = maximo;

  if (largerPi < minimo)
    largerPi = minimo;
  if (largerPi > maximo)
    largerPi = maximo;

  // O PWM nunca será reduzido, apenas aumentado
  if (pi > largerPi)
  {
    largerPi = pi;
  }

  return largerPi;
}

void onWorkingMalhaFechadaStateEnter()
{
  lastTwaiSendTime = millis();
  lastTwaiRecvTime = millis();
  largerPi = 0;
  integralErro = 0;
}

void onWorkingMalhaFechadaStateLoop()
{
  // Segurança: Se o barramento cair durante a operação, o estimulador deverá
  // tomar uma ação de decremento independente
  if (millis() - lastTwaiRecvTime >= 1000)
  {
    stateManager.switchTo(StateKind::GatewayDownSafetyStopState);
    return;
  }

  data.requestedPwm = calculatePulseWidth();
  modulateLoop(data.requestedPwm);

  unsigned long now_ms = millis();
  if (now_ms - lastTwaiSendTime > 5)
  {
    lastTwaiSendTime = now_ms;

    twaiSend(TwaiSendMessageKind::PwmFeedbackEstimulador,
             (uint16_t)data.requestedPwm);
  }
}

void onWorkingMalhaFechadaStateTWAIMessage(
    TwaiReceivedMessage *receivedMessage)
{
  lastTwaiRecvTime = millis();

  switch (receivedMessage->Kind)
  {
  case TwaiReceivedMessageKind::FirmwareInvokeReset:
    esp_restart();
    break;
  case TwaiReceivedMessageKind::WeightTotal:
    data.weightTotal = receivedMessage->ExtraData;
    break;
  case TwaiReceivedMessageKind::ResidualWeightTotal:
    data.residualWeightTotal = receivedMessage->ExtraData;
    break;
  case TwaiReceivedMessageKind::SetRequestedPwm:
    data.requestedPwm = receivedMessage->ExtraData;
    break;
  case TwaiReceivedMessageKind::MeseMax:
    data.meseMax = receivedMessage->ExtraData;
    break;
  case TwaiReceivedMessageKind::Setpoint:
    data.setpointKg = receivedMessage->ExtraData;
    break;
  case TwaiReceivedMessageKind::UseMalhaAberta:
    stateManager.switchTo(StateKind::WorkingMalhaAbertaState);
    break;
  case TwaiReceivedMessageKind::Mese:
    data.mese = receivedMessage->ExtraData;
    break;
  case TwaiReceivedMessageKind::SetGainCoefficient:
    data.gainCoefficient = receivedMessage->ExtraData / 100.0f;
    break;
  case TwaiReceivedMessageKind::GatewayResetHappened:
    ESP_LOGE(stateManager.current->TAG, "O Gateway reiniciou inesperadamente.");
    stateManager.switchTo(StateKind::GatewayDownSafetyStopState);
  }
}

void onWorkingMalhaFechadaStateExit() {}