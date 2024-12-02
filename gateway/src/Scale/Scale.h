#include "../Props.h"

// Esse define remove o código de leitura das balanças, e o troca por dados
// simulados. Ele é usado para desenvolvimento apenas, quando não há uma
// balança física presente.
#ifdef IS_DEVELOPMENT
#define SCALE_USE_STUB 1
#endif

enum Scale
{
    A,
    B,
    C,
    D
};

void scaleBeginOrDie();
void scaleUpdate();

// Public API
int scaleGetWeightL();
int scaleGetWeightR();
int scaleGetTotalWeight();
