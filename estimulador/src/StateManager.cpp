#include <unordered_map>
#include "StateManager.h"
#include <Arduino.h>
#include "esp_log.h"

static const char *TAG = "StateManager";

StateManager::StateManager()
{

    this->states = {
        {
            StateKind::WorkingMalhaAbertaState,
            State{
                .TAG = "WorkingMalhaAbertaState",
                .onEnter = onWorkingMalhaAbertaStateEnter,
                .onLoop = onWorkingMalhaAbertaStateLoop,
                .onTWAIMessage = onWorkingMalhaAbertaStateTWAIMessage,
                .onExit = onWorkingMalhaAbertaStateExit,
            },
        },
        {
            StateKind::WorkingMalhaFechadaState,
            State{
                .TAG = "WorkingMalhaFechadaState",
                .onEnter = onWorkingMalhaFechadaStateEnter,
                .onLoop = onWorkingMalhaFechadaStateLoop,
                .onTWAIMessage = onWorkingMalhaFechadaStateTWAIMessage,
                .onExit = onWorkingMalhaFechadaStateExit,
            },
        },
        {
            StateKind::GatewayDownSafetyStopState,
            State{
                .TAG = "GatewayDownSafetyStopState",
                .onEnter = onGatewayDownSafetyStopStateEnter,
                .onLoop = onGatewayDownSafetyStopStateLoop,
                .onTWAIMessage = onGatewayDownSafetyStopStateTWAIMessage,
                .onExit = onGatewayDownSafetyStopStateExit,
            },
        },
    };
}

void StateManager::setup(StateKind initial)
{
    this->current = &this->states[initial];
    this->currentKind = initial;

    ESP_LOGI(TAG, "State %s enter...", this->current->TAG);
    if (this->current->onEnter != nullptr)
    {
        this->current->onEnter();
    }
}

void StateManager::switchTo(StateKind to)
{
    if (!this->states.count(to))
    {
        while (1)
            ESP_LOGE(TAG, "State not found: %d", to);
    }

    ESP_LOGI(TAG, "State %s exit...", this->current->TAG);
    if (this->current->onExit != nullptr)
    {
        this->current->onExit();
    }

    this->current = &this->states[to];
    this->currentKind = to;

    ESP_LOGI(TAG, "State %s enter...", this->current->TAG);
    if (this->current->onEnter != nullptr)
    {
        this->current->onEnter();
    }
}

void StateManager::loop()
{
    if (this->current->onLoop != nullptr)
    {
        this->current->onLoop();
    }
}

void StateManager::onTWAIMessage(TwaiReceivedMessage *receivedMessage)
{
    if (this->current->onTWAIMessage != nullptr)
    {
        this->current->onTWAIMessage(receivedMessage);
    }
}