
#include "Data.h"

#define DEBUG(variable) ESP_LOGD(TAG, #variable ": %d", variable)

static const char *TAG = "Data";

Data::Data()
{
  this->mese = 0;
  this->meseMax = 0;
  this->weightL = 0;
  this->weightR = 0;
  this->collectedWeight = 0;
  this->setpoint = 0;
  this->pwmFeedback = 0;

  this->lastBluetoothSendTime = 0;
}

void Data::sendToBle(const Bluetooth &ble)
{
  if (!espBle.isConnected())
  {
    return;
  }

  // Se enviarmos o payload pelo Bluetooth tod0 instante, o frontend ficará sobrecarregado
  // Enviamos o payload com o status mais recente a cada N milissegundos.
  unsigned long now = millis();
  if (now - data.lastBluetoothSendTime < 120)
  {
    return;
  }

  data.lastBluetoothSendTime = now;

  BleStatusPacket status;
  memset(&status, 0, sizeof(status));

  status.pwm = data.pwmFeedback;
  status.mese = data.mese;
  status.meseMax = data.meseMax;
  status.weightL = data.weightL;
  status.weightR = data.weightR;
  status.collectedWeight = data.collectedWeight;
  status.setpoint = data.setpoint;

  espBle.writeStatusData(&status);
}

void Data::debugPrintAll()
{
  ESP_LOGI(TAG, "");
  DEBUG(this->weightL);
  DEBUG(this->weightR);
  DEBUG(this->collectedWeight);
  DEBUG(this->pwmFeedback);
  DEBUG(this->mese);
  DEBUG(this->meseMax);
  DEBUG(this->setpoint);
  ESP_LOGI(TAG, "");
}
