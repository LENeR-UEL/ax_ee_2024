#pragma once

#include <cstdint>
#include <ArduinoBLE.h>

extern BLEService service;

extern BLECharacteristic characteristicStatusFeedback;
extern BLEShortCharacteristic characteristicControl;

enum class BluetoothControlCode
{
    /**
     * Invoca um reset no gateway e no estimulador.
     */
    FirmwareInvokeReset = 0x00,

    MESECollecter_GoBackToParameterSetup = 0x20,
    MESECollecter_IncreaseOnce = 0x21,
    MESECollecter_DecreaseOnce = 0x22,
    MESECollecter_RegisterMESE = 0x24,
    MESECollecter_Complete = 0x2f,

    Parallel_GoBackToMESECollecter = 0x10,
    Parallel_RegisterWeight = 0x11,
    Parallel_SetWeightFromArgument = 0x16,
    Parallel_Complete = 0x1f,

    MainOperation_GoBackToParallel = 0x30,
    MainOperation_SetSetpoint = 0x31,
    MainOperation_IncreaseMESEMaxOnce = 0x32,
    MainOperation_DecreaseMESEMaxOnce = 0x33,
    MainOperation_EmergencyStop = 0x38,

    ParameterSetup_SetGradualIncreaseTime = 0x61,
    ParameterSetup_SetTransitionTime = 0x63,
    ParameterSetup_SetGradualDecreaseTime = 0x64,
    ParameterSetup_SetMalhaFechadaAboveSetpointTime = 0x66,
    ParameterSetup_SetGainCoefficient = 0x67,
    ParameterSetup_Reset = 0x6D,
    ParameterSetup_Save = 0x6E,
    ParameterSetup_Complete = 0x6F,
};

typedef struct __attribute__((__packed__))
{
    uint16_t pwm;
    uint8_t weightL;
    uint8_t weightR;
    uint16_t collectedWeight;
    uint16_t mese;
    uint16_t meseMax;
    uint16_t setpoint;

    struct __attribute__((__packed__))
    {
        unsigned isEEGFlagSet : 1;
        unsigned isCANAvailable : 1;
        unsigned reserved6 : 1;
        unsigned reserved5 : 1;
        unsigned reserved4 : 1;
        unsigned reserved3 : 1;
        unsigned reserved2 : 1;
        unsigned reserved1 : 1;
    } status_flags;

    struct __attribute__((__packed__))
    {
        uint16_t gradualIncreaseTime;
        uint16_t transitionTime;
        uint16_t gradualDecreaseTime;
        uint16_t malhaFechadaAboveSetpointTime;

        // Coeficiente de ganho enviado ao estimulador. Range [0, 100], inteiro. No estimulador, é dividido por 100.
        uint8_t gainCoefficient;
    } parameterSetup;

    // Informar o app do estado atual da operação
    uint8_t mainOperationStateInformApp[6];
} BleStatusPacket;

/// Bluetooth class for handling Bluetooth operations
class Bluetooth
{
private:
    const char *AdvertisingName; ///< Advertising name for Bluetooth device

public:
    /// Callback when the phone writes to the Control characteristic.
    void (*onControlReceivedCallback)(uint16_t fullPayload);

    static void Update();

    /// Method to start the Bluetooth service
    /// \return True if it started successfully
    void startOrDie();

    /// Method to check if the Bluetooth device is connected
    /// \return True if connected, false otherwise
    static bool isConnected();

    /// Method to write status data to the Bluetooth device
    void writeStatusData(BleStatusPacket *packet);

    explicit Bluetooth(const char *advertisingName);
};

extern Bluetooth espBle;
