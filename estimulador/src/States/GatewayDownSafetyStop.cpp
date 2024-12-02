#include "../StateManager.h"
#include "../Twai/Twai.h"
#include "../Data.h"
#include "../Modulator.h"
#include <Arduino.h>

static unsigned long lastDecreaseTime = 0;
static uint16_t currentPulseWidth = 0;
static bool gatewayResetHappened = false;

void onGatewayDownSafetyStopStateEnter()
{
    gatewayResetHappened = false;
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
        ESP_LOGW(stateManager.current->TAG, "Barramento caiu. PWM: %u\n", (unsigned int)currentPulseWidth);
    }

    if (gatewayResetHappened && currentPulseWidth == 0)
    {
        ESP_LOGI(stateManager.current->TAG, "Recuperando...");
        esp_restart();
    }

    modulateLoop(currentPulseWidth);
}

/**
 * Talvez o barramento tenha recuperado. Lidar com pedidos de reset.
 */
void onGatewayDownSafetyStopStateTWAIMessage(TwaiReceivedMessage *receivedMessage)
{
    switch (receivedMessage->Kind)
    {
    case TwaiReceivedMessageKind::GatewayResetHappened:
        ESP_LOGI(stateManager.current->TAG, "O Gateway reiniciou.");
        gatewayResetHappened = true;
        break;
    }
}

void onGatewayDownSafetyStopStateExit() {}