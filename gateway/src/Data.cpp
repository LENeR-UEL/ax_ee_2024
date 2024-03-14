
#include "Data.h"

#define DEBUG(variable) ESP_LOGD(TAG, #variable ": %d", variable)

static const char *TAG = "Data";

Data::Data() {
  this->pwm = 0;
  this->mese = 0;
  this->meseMax = 0;
  this->weightL = 0;
  this->weightR = 0;
  this->collectedWeight = 0;
  this->setpoint = 0;
  this->pwmFeedback = 0;
  this->trigger = FlagTrigger::MalhaAberta;
  this->isDecreasingPwm = false;

  this->lastBluetoothSendTime = 0;
}

void Data::sendToTwai() const {
  twaiSend(TwaiSendMessageKind::WeightL, data.weightL);
  twaiSend(TwaiSendMessageKind::WeightR, data.weightR);
  twaiSend(TwaiSendMessageKind::SetRequestedPwm, data.pwm);
  twaiSend(TwaiSendMessageKind::MeseMax, data.meseMax);
  twaiSend(TwaiSendMessageKind::Setpoint, data.setpoint);
  twaiSend(TwaiSendMessageKind::Trigger, (uint8_t) trigger);
}

void Data::sendToBle(const Bluetooth &ble) {
  if (!espBle.isConnected()) {
    return;
  }

  // Se enviarmos o payload pelo Bluetooth tod0 instante, o frontend ficará sobrecarregado
  // Enviamos o payload com o status mais recente a cada N milissegundos.
  unsigned long now = millis();
  if (now - data.lastBluetoothSendTime < 30) {
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

void Data::debugPrintAll() {
  ESP_LOGI(TAG, "");
  DEBUG(this->weightL);
  DEBUG(this->weightR);
  DEBUG(this->collectedWeight);
  DEBUG(this->pwm);
  DEBUG(this->pwmFeedback);
  DEBUG(this->mese);
  DEBUG(this->meseMax);
  DEBUG(this->setpoint);
  DEBUG(this->trigger);
  DEBUG(this->isDecreasingPwm);
  ESP_LOGI(TAG, "");
}