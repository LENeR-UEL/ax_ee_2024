#pragma once
#include <unordered_map>
#include "Twai/Twai.h"

enum class StateKind : uint8_t
{
    WorkingMalhaAbertaState,
    WorkingMalhaFechadaState,
    GatewayDownSafetyStopState
};

typedef struct State
{
    const char *TAG;
    void (*onEnter)();
    void (*onLoop)();
    void (*onTWAIMessage)(TwaiReceivedMessage *receivedMessage);
    void (*onExit)();
} State;

class StateManager
{
private:
    std::unordered_map<StateKind, State> states;

public:
    State *current;
    StateKind currentKind;
    StateManager();
    void setup(StateKind initial);
    void switchTo(StateKind to);
    void loop();
    void onTWAIMessage(TwaiReceivedMessage *receivedMessage);
};

extern StateManager stateManager;

// Definição dos estados
void onWorkingMalhaAbertaStateEnter();
void onWorkingMalhaAbertaStateLoop();
void onWorkingMalhaAbertaStateTWAIMessage(TwaiReceivedMessage *receivedMessage);
void onWorkingMalhaAbertaStateExit();

void onWorkingMalhaFechadaStateEnter();
void onWorkingMalhaFechadaStateLoop();
void onWorkingMalhaFechadaStateTWAIMessage(TwaiReceivedMessage *receivedMessage);
void onWorkingMalhaFechadaStateExit();

void onGatewayDownSafetyStopStateEnter();
void onGatewayDownSafetyStopStateLoop();
void onGatewayDownSafetyStopStateTWAIMessage(TwaiReceivedMessage *receivedMessage);
void onGatewayDownSafetyStopStateExit();