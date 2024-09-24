#include "../StateManager.h"
#include "../Twai/Twai.h"
#include "../Data.h"
#include "../Modulator.h"
#include <Arduino.h>

static unsigned long lastTwaiSendTime = 0;
static unsigned long lastTwaiRecvTime = 0;

static int largerPi = 0;
static int integralErro = 0;

int calculatePulseWidth()
{
    int erro = data.weightTotal - data.setpointKg * 2;
    integralErro += erro;

    // Saturar integralErro em [-50 e 50]
    integralErro = max(-50, min(50, integralErro));

    int controlWeight = data.weightTotal - data.residualWeightTotal;
    int pi = data.mese + controlWeight * data.gainCoefficient;

    // Saturar pi em [mese, meseMax]
    if (pi < data.mese)
        pi = data.mese;
    if (pi > data.meseMax)
        pi = data.meseMax;
    if (largerPi < data.mese)
        largerPi = data.mese;
    if (largerPi > data.meseMax)
        largerPi = data.meseMax;

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
    // Segurança: Se o barramento cair durante a operação, o estimulador deverá tomar uma ação de decremento independente
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

        twaiSend(TwaiSendMessageKind::PwmFeedbackEstimulador, (uint16_t)data.requestedPwm);
    }
}

void onWorkingMalhaFechadaStateTWAIMessage(TwaiReceivedMessage *receivedMessage)
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