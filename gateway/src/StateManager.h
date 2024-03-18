#pragma once
#include <unordered_map>

enum class StateKind
{
    Disconnected,
    StateB,
    StateC,
    StateD
};

typedef struct State
{
    void (*onEnter)();
    void (*onLoop)();
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
};

// Definição dos estados
void onEnter_Disconnected();
void onLoop_Disconnected();
void onExit_Disconnected();
