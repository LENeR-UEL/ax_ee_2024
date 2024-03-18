#include <unordered_map>
#include "StateManager.h"
#include <Arduino.h>
#include "esp_log.h"

static const char *TAG = "StateManager";

StateManager::StateManager()
{

    this->states = {
        {
            StateKind::Disconnected,
            State{
                .onEnter = onEnter_Disconnected,
                .onLoop = onLoop_Disconnected,
                .onExit = onExit_Disconnected,
            },
        },
    };
}

void StateManager::setup(StateKind initial)
{
    ESP_LOGI(TAG, "Starting at state %d", initial);

    this->current = &this->states[initial];
    this->current->onEnter();
}

void StateManager::switchTo(StateKind to)
{
    if (!this->states.count(to))
    {
        while (1)
            ESP_LOGE(TAG, "State not found: %d", to);
    }

    ESP_LOGI(TAG, "Switching to state %d", to);

    this->current->onExit();
    this->current = &this->states[to];
    this->current->onEnter();
}

void StateManager::loop()
{
    this->current->onLoop();
}
