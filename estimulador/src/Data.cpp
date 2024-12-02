#include "Data.h"
#include <memory.h>

// Redefine os valores da estrutura global `data` para padrões sanos.
void dataReset()
{
    memset(&data, 0, sizeof(data));

    data.gainCoefficient = 0.5f;
}