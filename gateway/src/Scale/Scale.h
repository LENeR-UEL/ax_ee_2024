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
