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

    ParameterSetup_Complete = 0x2F,

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

    ResetPwmImmediate = 0x80,
    DecreasePwmStep = 0x81,
    IncreasePwmStep = 0x82,
    ResetPwmGradual = 0x83,
    CollectWeight = 0x8A,
    SaveMese = 0x90,
    DecreaseMeseMaxStep = 0xA1,
    IncreaseMeseMaxStep = 0xA2,
    SetSetpoint = 0xB1,
    // Na transição de telas de "Paralela" para "Operação", o app envia essa flag
    SetTrigger = 0xC0,
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
