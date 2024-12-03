#pragma once

// OVBOXPin is an Arduino Pin used for OVBOX's trigger input.
#define OVBOXPin 34

// Include the Twai/Twai.h library to work with TWAI/CAN Bus protocol.
#include <stdint.h>
#include "Twai/Twai.h"

class Data
{
public:
    // Data class constructor.
    Data();

    // Represents the measured value of 'MESE' (a portuguese abbreviation for a measured variable in the FES context).
    uint16_t mese;

    // The maximum 'MESE' value.
    uint16_t meseMax;

    // The value measured from the left scale.
    uint16_t weightL;

    // The value measured from the right scale.
    uint16_t weightR;

    // The collected weight, which is calculated by summing the measured weights from both scales.
    uint16_t collectedWeight;

    // The set point that the stimulation should reach. This is the target value set by the user or the application.
    uint16_t setpoint;

    // PWM feedback from the hardware stimulator, which represents the actual stimulation output.
    uint16_t pwmFeedback;

    // Parameter setup struct for the stimulator. The structure is packed for easier data packing.
    struct __attribute__((__packed__))
    {
        // Time taken for gradual increase in stimulation output.
        uint16_t gradualIncreaseTime;

        // Time taken for transition from one stimulation value to another.
        uint16_t transitionTime;

        // Time taken for gradual decrease in stimulation output.
        uint16_t gradualDecreaseTime;

        // Time when the stimulation output exceeds the set point.
        uint16_t malhaFechadaAboveSetpointTime;

        // Gain coefficient sent to the stimulator. The range is from 0 to 100, and it's an integer.
        // In the stimulator, this value is divided by 100.
        uint8_t gainCoefficient;
    } parameterSetup;

    // Array to hold the operation state information for the Android application.
    uint8_t mainOperationStateInformApp[6];

    // Last time the data was sent to the BLE (Bluetooth Low Energy) module.
    unsigned long lastBluetoothSendTime;

    unsigned long lastTelemetrySendTime;

    // Function to reset the data to their default values.
    void reset();

    // Function to send the data to the BLE module.
    void sendToBle();

    // Function to debug print the data.
    void debugPrintAll();

    // Function to check if OVBOX flag is set.
    bool isOVBoxFlagSet();
};

// External data variable instance of the Data class.
extern Data data;