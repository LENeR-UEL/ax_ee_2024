#include "../StateManager.h"
#include "../Twai/Twai.h"
#include "../Data.h"
#include "../Modulator.h"
#include <Arduino.h>

static unsigned long lastTwaiSendTime = 0;
static unsigned long lastTwaiRecvTime = 0;

void onWorkingMalhaAbertaStateEnter()
{
    lastTwaiSendTime = millis();
    lastTwaiRecvTime = millis();
}

void onWorkingMalhaAbertaStateLoop()
{
    // Segurança: Se o barramento cair durante a operação, o estimulador deverá tomar uma ação de decremento independente
    if (millis() - lastTwaiRecvTime >= 1000)
    {
        stateManager.switchTo(StateKind::GatewayDownSafetyStopState);
        return;
    }

    modulateLoop(data.requestedPwm);

    unsigned long now_ms = millis();
    if (now_ms - lastTwaiSendTime > 5)
    {
        lastTwaiSendTime = now_ms;

        twaiSend(TwaiSendMessageKind::PwmFeedbackEstimulador, (uint16_t)data.requestedPwm);
    }
}

void onWorkingMalhaAbertaStateTWAIMessage(TwaiReceivedMessage *receivedMessage)
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
    case TwaiReceivedMessageKind::Trigger:
        // data.flagTrigger = receivedMessage->ExtraData > 0 ? FlagTrigger::MalhaFechadaOperacao : FlagTrigger::MalhaAberta;
        break;
    case TwaiReceivedMessageKind::Mese:
        data.mese = receivedMessage->ExtraData;
        break;
    case TwaiReceivedMessageKind::SetGainCoefficient:
        data.gainCoefficient = receivedMessage->ExtraData / 100.0f;
        break;
    }
}

void onWorkingMalhaAbertaStateExit() {}