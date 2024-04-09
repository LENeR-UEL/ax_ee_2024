#pragma once
#include <unordered_map>
#include "Twai/Twai.h"
#include "Bluetooth/Bluetooth.h"

enum class StateKind
{
    Disconnected,
    ParameterSetup,
    ParallelWeight,
    MESECollecter,
    MainOperation,
    OperationStart,
    OperationGradualIncrease,
    OperationTransition,
    OperationMalhaFechada,
    OperationStop
};

typedef struct State
{
    const char *TAG;
    void (*onEnter)();
    void (*onLoop)();
    void (*onTWAIMessage)(TwaiReceivedMessage *receivedMessage);
    void (*onBLEControl)(BluetoothControlCode code, uint8_t extraData);
    void (*onExit)();
} State;

class StateManager
{
private:
    std::unordered_map<StateKind, State> states;

public:
    State *current;
    StateManager();
    void setup(StateKind initial);
    void switchTo(StateKind to);
    void loop();
    void onTWAIMessage(TwaiReceivedMessage *receivedMessage);
    void onBLEControl(BluetoothControlCode code, uint8_t extraData);
};

extern StateManager stateManager;

// Definição dos estados
void onDisconnectedStateEnter();
void onDisconnectedStateLoop();
void onDisconnectedStateTWAIMessage(TwaiReceivedMessage *receivedMessage);
void onDisconnectedStateBLEControl(BluetoothControlCode code, uint8_t extraData);
void onDisconnectedStateExit();

void onParameterSetupStateEnter();
void onParameterSetupStateLoop();
void onParameterSetupStateTWAIMessage(TwaiReceivedMessage *receivedMessage);
void onParameterSetupStateBLEControl(BluetoothControlCode code, uint8_t extraData);
void onParameterSetupStateExit();

void onParallelWeightStateEnter();
void onParallelWeightStateLoop();
void onParallelWeightStateTWAIMessage(TwaiReceivedMessage *receivedMessage);
void onParallelWeightStateBLEControl(BluetoothControlCode code, uint8_t extraData);
void onParallelWeightStateExit();

void onMESECollecterStateEnter();
void onMESECollecterStateLoop();
void onMESECollecterStateTWAIMessage(TwaiReceivedMessage *receivedMessage);
void onMESECollecterStateBLEControl(BluetoothControlCode code, uint8_t extraData);
void onMESECollecterStateExit();

void onMainOperationStateEnter();
void onMainOperationStateLoop();
void onMainOperationStateTWAIMessage(TwaiReceivedMessage *receivedMessage);
void onMainOperationStateBLEControl(BluetoothControlCode code, uint8_t extraData);
void onMainOperationStateExit();

void onOperationStartEnter();
void onOperationStartLoop();
void onOperationStartTWAIMessage(TwaiReceivedMessage *receivedMessage);
void onOperationStartBLEControl(BluetoothControlCode code, uint8_t extraData);
void onOperationStartExit();

void onOperationGradualIncreaseEnter();
void onOperationGradualIncreaseLoop();
void onOperationGradualIncreaseTWAIMessage(TwaiReceivedMessage *receivedMessage);
void onOperationGradualIncreaseBLEControl(BluetoothControlCode code, uint8_t extraData);
void onOperationGradualIncreaseExit();

void onOperationTransitionEnter();
void onOperationTransitionLoop();
void onOperationTransitionTWAIMessage(TwaiReceivedMessage *receivedMessage);
void onOperationTransitionBLEControl(BluetoothControlCode code, uint8_t extraData);
void onOperationTransitionExit();

void onOperationMalhaFechadaEnter();
void onOperationMalhaFechadaLoop();
void onOperationMalhaFechadaTWAIMessage(TwaiReceivedMessage *receivedMessage);
void onOperationMalhaFechadaBLEControl(BluetoothControlCode code, uint8_t extraData);
void onOperationMalhaFechadaExit();

void onOperationStopEnter();
void onOperationStopLoop();
void onOperationStopTWAIMessage(TwaiReceivedMessage *receivedMessage);
void onOperationStopBLEControl(BluetoothControlCode code, uint8_t extraData);
void onOperationStopExit();
