#include "Bluetooth.h"
#include <esp_log.h>

static const char *TAG = "Bluetooth";
static const uint8_t completeRawAdvertisingData[] = {0x02, 0x01, 0x06};
static BLEService service("ab04");
static BLECharacteristic characteristicStatusFeedback("ff01", BLERead | BLENotify,
                                                      sizeof(BleStatusPacket));
static BLEShortCharacteristic characteristicControl("ff0f", BLEWriteWithoutResponse |
                                                                BLEWrite | BLENotify);

static BluetoothControlCallback controlCallback = nullptr;

static unsigned long lastAlivePacketTime = 0;
static const unsigned long TIMEOUT = 9000;
static bool deviceReady = false;

void onDeviceConnected(BLEDevice device)
{
  ESP_LOGI(TAG, "Conexão Bluetooth estabelecida!");
  lastAlivePacketTime = millis() + 10000;
  deviceReady = false;
}

void onControlWritten(BLEDevice device, BLECharacteristic characteristic)
{
  uint16_t fullPayload = characteristicControl.value();

  BluetoothControlCode code = (BluetoothControlCode)(fullPayload & 0x00FF);
  uint8_t extraData = (fullPayload & 0xFF00) >> 8;

  unsigned long now = millis();

  if (deviceReady && now - lastAlivePacketTime >= TIMEOUT)
  {
    // Tarde demais, a conexão já está considerada terminada. Não notificar o resto do firmware.
    ESP_LOGE(TAG, "Aplicativo enviou um comando após timeout de conexão (%lu ms após timeout). A conexão já foi considerada terminada.", now - lastAlivePacketTime);
    return;
  }

  if (code == BluetoothControlCode::StillAlive)
  {
    lastAlivePacketTime = now;
    deviceReady = true;
  }

  if (controlCallback != nullptr)
  {
    controlCallback(code, extraData);
  }
}

void bluetoothSetup()
{
  ESP_LOGI(TAG, "BLE setup");
  while (!BLE.begin())
  {
    Serial.println("failed to initialize BLE!");
  }

  // O tempo de conexão é o tempo entre um evento de rádio numa determinada ligação e o próximo
  // evento de rádio na mesma ligação. Os dispositivos BLE acordam neste intervalo no início da
  // conexão (e podem renegociá-lo mais tarde). Isto significa que, em cada intervalo de
  // ligação, a central envia um pacote (mesmo que seja um pacote vazio) e o periférico liga o seu
  // rádio e fica à escuta. Se houver dados, os pacotes são enviados durante algum tempo, depois o
  // rádio desliga-se até ao início do próximo evento de rádio (que é um intervalo de ligação após
  // o início do último).
  BLE.setConnectionInterval(9, 12); // Taxa de atualizaçao entre 9 e 12 ms.

  service.addCharacteristic(characteristicStatusFeedback);
  service.addCharacteristic(characteristicControl);

  BLE.addService(service);

  // Build advertising data packet
  BLEAdvertisingData advData;

  // If a packet has a raw data parameter, then all the other parameters of the
  // packet will be ignored
  advData.setRawData(completeRawAdvertisingData,
                     sizeof(completeRawAdvertisingData));

  // Copy set parameters in the actual advertising packet
  BLE.setAdvertisingData(advData);

  // Build scan response data packet
  BLEAdvertisingData scanData;
  scanData.setLocalName("LENeR Gateway");

  BLE.setScanResponseData(scanData);
  BLE.advertise();
  ESP_LOGI(TAG, "Advertising! Bluetooth service UUID is %s", service.uuid());

  BLE.setEventHandler(BLEDeviceEvent::BLEConnected, onDeviceConnected);
  characteristicControl.setEventHandler(BLECharacteristicEvent::BLEWritten, onControlWritten);
  characteristicControl.subscribe();
}

bool libConnected = false;

void bluetoothLoop()
{
  BLE.poll();
  libConnected = BLE.connected();

  unsigned long now = millis();
  if (libConnected && deviceReady && now - lastAlivePacketTime >= TIMEOUT)
  {
    ESP_LOGW(TAG, "O dispositivo continua conectado para a ArduinoBLE, mas passou o tempo de timeout de StillAlive. Desconectando...");
    ESP_LOGW(TAG, "Desconexão do mecanismo StillAlive foi removido do código.");
    // BLE.disconnect();
  }
}

bool bluetoothIsConnected()
{
  unsigned long now = millis();
  return libConnected && deviceReady && now - lastAlivePacketTime < TIMEOUT;
}

void bluetoothWriteStatusData(BleStatusPacket *packet)
{

  characteristicStatusFeedback.writeValue(packet, sizeof(BleStatusPacket));
}

void bluetoothSetControlCallback(BluetoothControlCallback callback)
{
  controlCallback = callback;
}