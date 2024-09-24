#include "../StateManager.h"
#include "../Twai/Twai.h"
#include "../Data.h"
#include "../Modulator.h"
#include <Arduino.h>

static unsigned long lastDecreaseTime = 0;
static uint16_t currentPulseWidth = 0;

void onGatewayDownSafetyStopStateEnter()
{
    lastDecreaseTime = millis();
    currentPulseWidth = data.requestedPwm;
}

void onGatewayDownSafetyStopStateLoop()
{
    unsigned long now_ms = millis();
    if (now_ms - lastDecreaseTime >= 50)
    {
        if (currentPulseWidth >= 1)
        {
            currentPulseWidth = currentPulseWidth - 1;
        }
        else
        {
            currentPulseWidth = 0;
        }

        lastDecreaseTime = now_ms;
        ESP_LOGW(stateManager.current->TAG, "Barramento caiu. PWM: %u\n", (unsigned int)requestedPwm);
    }

    modulateLoop(currentPulseWidth);
}

void onGatewayDownSafetyStopStateTWAIMessage(TwaiReceivedMessage *receivedMessage) {}

void onGatewayDownSafetyStopStateExit() {}