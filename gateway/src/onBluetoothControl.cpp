#include <esp_log.h>
#include <onBluetoothControl.h>
#include "Bluetooth/Bluetooth.h"
#include "Data.h"

static const char *TAG = "onBluetoothControlCode";

void onBluetoothControl(uint16_t fullPayload) {
  ESP_LOGI("main", "Control! Dump=%02X\n", fullPayload);

  BluetoothControlCode code = (BluetoothControlCode) (fullPayload & 0x00FF);
  uint8_t extraData = (fullPayload & 0xFF00) >> 8;

  ESP_LOGI("main", "Control! Code=%X ExtraData=%d\n", code, extraData);

  switch (code) {
    case BluetoothControlCode::SetTrigger:
      data.trigger = extraData == 0 ? FlagTrigger::MalhaAberta : FlagTrigger::MalhaFechadaOperacao;
      break;
    case BluetoothControlCode::CollectWeight:
      data.collectedWeight = data.weightL + data.weightR;
      break;
    case BluetoothControlCode::IncreasePwmStep:
      // If it causes overflow, set it to zero instead
      data.pwm += 5;
      if (data.pwm > 255) {
        data.pwm = 255;
      }

      break;
    case BluetoothControlCode::DecreasePwmStep:
      // If it causes underflow, set it to zero instead
      if (data.pwm < 5) {
        data.pwm = 0;
      } else {
        data.pwm -= 5;
      }

      break;
    case BluetoothControlCode::DecreaseMeseMaxStep:
      // If it causes underflow, set it to zero instead
      if (data.meseMax < 5) {
        data.meseMax = 0;
      } else {
        data.meseMax -= 5;
      }

      break;
    case BluetoothControlCode::IncreaseMeseMaxStep:
      data.meseMax += 5;
      if (data.meseMax > 250) {
        data.meseMax = 255;
      }

      break;
    case BluetoothControlCode::SaveMese:
      data.mese = data.pwm;
      // 120% do mese
      data.meseMax = data.pwm * 1.2f;
      // 50% do mese
      data.setpoint = data.mese * 0.5f;
      break;
    case BluetoothControlCode::ResetPwmGradual:
      data.isDecreasingPwm = true;
      break;
    case BluetoothControlCode::SetSetpoint:
      data.setpoint = extraData;
      break;
    default:
      ESP_LOGE(TAG, "Unhandled control code %x extraData %d\n", code, extraData);
  }
}
