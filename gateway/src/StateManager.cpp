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
                .TAG = "DisconnectedState",
                .onEnter = onDisconnectedStateEnter,
                .onLoop = onDisconnectedStateLoop,
                .onTWAIMessage = onDisconnectedStateTWAIMessage,
                .onBLEControl = onDisconnectedStateBLEControl,
                .onExit = onDisconnectedStateExit,
            },
        },
        {
            StateKind::ParameterSetup,
            State{
                .TAG = "ParameterSetup",
                .onEnter = onParameterSetupStateEnter,
                .onLoop = onParameterSetupStateLoop,
                .onTWAIMessage = onParameterSetupStateTWAIMessage,
                .onBLEControl = onParameterSetupStateBLEControl,
                .onExit = onParameterSetupStateExit,
            },
        },
        {
            StateKind::ParallelWeight,
            State{
                .TAG = "ParallelWeight",
                .onEnter = onParallelWeightStateEnter,
                .onLoop = onParallelWeightStateLoop,
                .onTWAIMessage = onParallelWeightStateTWAIMessage,
                .onBLEControl = onParallelWeightStateBLEControl,
                .onExit = onParallelWeightStateExit,
            },
        },
        {
            StateKind::MESECollecter,
            State{
                .TAG = "MESECollecter",
                .onEnter = onMESECollecterStateEnter,
                .onLoop = onMESECollecterStateLoop,
                .onTWAIMessage = onMESECollecterStateTWAIMessage,
                .onBLEControl = onMESECollecterStateBLEControl,
                .onExit = onMESECollecterStateExit,
            },
        },
        {
            StateKind::MainOperation,
            State{
                .TAG = "MainOperation",
                .onEnter = onMainOperationStateEnter,
                .onLoop = onMainOperationStateLoop,
                .onTWAIMessage = onMainOperationStateTWAIMessage,
                .onBLEControl = onMainOperationStateBLEControl,
                .onExit = onMainOperationStateExit,
            },
        }};
}

void StateManager::setup(StateKind initial)
{
    this->current = &this->states[initial];

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

void StateManager::onBLEControl(BluetoothControlCode code, uint8_t extraData)
{
    if (this->current->onBLEControl != nullptr)
    {
        this->current->onBLEControl(code, extraData);
    }
}
