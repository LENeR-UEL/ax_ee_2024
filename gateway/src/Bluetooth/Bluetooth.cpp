#include "Bluetooth.h"

BLEService service("ab04");
BLECharacteristic characteristicStatusFeedback("ff01", BLERead | BLENotify, 12);
BLEShortCharacteristic characteristicControl("ff0f", BLEWriteWithoutResponse | BLEWrite | BLENotify);

Bluetooth espBle("ESP-32 LILAT2");

// Advertising parameters should have a global scope. Do NOT define them in 'setup' or in 'loop'
const uint8_t completeRawAdvertisingData[] = {0x02, 0x01, 0x06};

Bluetooth::Bluetooth(const char *advertisingName) {
  this->AdvertisingName = advertisingName;
  this->onControlReceivedCallback = nullptr;
}

void Bluetooth::Update() {
  BLE.poll();
}

bool Bluetooth::isConnected() {
  return BLE.connected();
}

void Bluetooth::startOrDie() {
  while (!BLE.begin()) {
    Serial.println("failed to initialize BLE!");
  }

  service.addCharacteristic(characteristicStatusFeedback);
  service.addCharacteristic(characteristicControl);

  BLE.addService(service);

  // Build advertising data packet
  BLEAdvertisingData advData;

  // If a packet has a raw data parameter, then all the other parameters of the packet will be ignored
  advData.setRawData(completeRawAdvertisingData, sizeof(completeRawAdvertisingData));

  // Copy set parameters in the actual advertising packet
  BLE.setAdvertisingData(advData);

  // Build scan response data packet
  BLEAdvertisingData scanData;
  scanData.setLocalName(this->AdvertisingName);

  BLE.setScanResponseData(scanData);
  BLE.advertise();

  printf("Advertising! Bluetooth service UUID is %s\n", service.uuid());

  characteristicControl.setEventHandler(BLECharacteristicEvent::BLEWritten,
                                        [](BLEDevice device, BLECharacteristic characteristic) {
                                            uint16_t fullPayload = characteristicControl.value();

                                            if (espBle.onControlReceivedCallback != nullptr) {
                                              espBle.onControlReceivedCallback(fullPayload);
                                            }
                                        });

  characteristicControl.subscribe();
}

void Bluetooth::writeStatusData(BleStatusPacket *packet) {
  characteristicStatusFeedback.writeValue(packet, sizeof(BleStatusPacket));
}