/**
 * O ESP-32 de desenvolvimento tem uma pinagem diferente para o CAN do ESP-32 no laboratório do LENeR.
 */
#define USE_DEVELOPMENT_CAN_PINOUT true

/**
 * Deixa o trigger recebido no GPIO34 como sempre HIGH
 * Normalmente usado em conjunto com o modo de desenvolvedor, mas já usamos essa flag separadamente no LENeR, então fica a opção aqui.
 */
#define OVERRIDE_OVBOX_TRIGGER_ALWAYS_HIGH true

/**
 * No ESP-32 de desenvolvimento, não temos as células de carga HX711.
 * Nesse caso, lemos o valor de um potenciômetro, simulando as células de cargas.
 * Ao ativar essa opção, o arquivo "Scale.cpp" é desativado, e o "ScaleStub.cpp" é usado em seu lugar, provendo a mesma API ao resto do firmware.
 */
#define USE_EMULATED_SCALES_POTENTIOMETER true