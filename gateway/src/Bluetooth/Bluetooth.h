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

    Parallel_GoBackToParameterSetup = 0x10,
    Parallel_RegisterWeight = 0x11,
    Parallel_Complete = 0x1f,

    MESECollecter_GoBackToParallel = 0x20,
    MESECollecter_IncreaseOnce = 0x21,
    MESECollecter_DecreaseOnce = 0x22,
    MESECollecter_RegisterMESE = 0x24,
    MESECollecter_Complete = 0x2f,

    MainOperation_GoBackToMESECollecter = 0x30,
    MainOperation_SetSetpoint = 0x31,
    MainOperation_IncreaseMESEMaxOnce = 0x32,
    MainOperation_DecreaseMESEMaxOnce = 0x33,

    ParameterSetup_SetGradualIncreaseInterval = 0x61,
    ParameterSetup_SetGradualIncreaseStep = 0x62,
    ParameterSetup_SetTransitionTime = 0x63,
    ParameterSetup_SetGradualDecreaseInterval = 0x64,
    ParameterSetup_SetGradualDecreaseStep = 0x65,
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
        uint16_t gradualIncreaseInterval;
        uint8_t gradualIncreaseStep;
        uint16_t transitionTime;
        uint16_t gradualDecreaseInterval;
        uint8_t gradualDecreaseStep;
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
