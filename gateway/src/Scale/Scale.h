// Esse define remove o código de leitura das balanças, e o troca por dados
// simulados. Ele é usado para desenvolvimento apenas, quando não há uma
// balança física presente.
//
// Comente-o caso for usar as balanças reais.
#define SCALE_USE_STUB 1

enum Scale
{
    A,
    B,
    C,
    D
};

void scaleBeginOrDie();
void scaleUpdate();
int scaleGetMeasurement(Scale whichOne);
int scaleGetWeightL();
int scaleGetWeightR();
