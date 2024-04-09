#include <string.h>
#include <Arduino.h>
#include "Twai.h"

static const char *TAG = "Twai";

static const twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(WIRESS_GPIO_TX, WIRESS_GPIO_RX,
                                                                          TWAI_MODE_NORMAL);
static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

twai_message_t lastReceivedMessage;

void twaiStart()
{
  memset(&lastReceivedMessage, 0, sizeof(twai_message_t));

  // Install TWAI driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK)
  {
    ESP_LOGI(TAG, "Driver installed!");
  }
  else
  {
    ESP_LOGE(TAG, "Failed to install driver");
    return;
  }

  // Start TWAI driver
  if (twai_start() == ESP_OK)
  {
    ESP_LOGI(TAG, "Driver started!");
  }
  else
  {
    ESP_LOGE(TAG, "Failed to start driver");
    return;
  }
}

void twaiSend(TwaiSendMessageKind kind, uint16_t extraData)
{
  twai_message_t message;
  message.identifier = kind;
  message.flags = TWAI_MSG_FLAG_NONE;
  message.data_length_code = 4;

  uint8_t octet1 = extraData >> 8;
  uint8_t octet2 = extraData & 0xFF;

  message.data[0] = octet1;
  message.data[1] = octet2;

  // Fila de transmissão
  if (twai_transmit(&message, pdMS_TO_TICKS(0)) == ESP_OK)
  {
    //  ESP_LOGD(TAG, "Message (Kind=%0X, Data=%0X) queued for transmission", kind, extraData);
  }
  else
  {
    //  ESP_LOGD(TAG, "Failed to queue message (Kind=%0X, Data=%0X) for transmission", kind, extraData);
  }
}

esp_err_t twaiReceive(TwaiReceivedMessage *received)
{
  memset(&lastReceivedMessage, 0, sizeof(twai_message_t));
  memset(received, 0, sizeof(TwaiReceivedMessage));

  esp_err_t statusCode = twai_receive(&lastReceivedMessage, pdMS_TO_TICKS(0));
  if (statusCode == ESP_OK)
  {
    // Received OK!
    uint8_t octet1 = lastReceivedMessage.data[0];
    uint8_t octet2 = lastReceivedMessage.data[1];
    uint16_t data = (octet1 << 8) | octet2;

    received->Kind = (TwaiReceivedMessageKind)lastReceivedMessage.identifier;
    received->ExtraData = data;

    ESP_LOGD(TAG, "Received message Kind=%0X Data=%000X", received->Kind, received->ExtraData);
  }
  else
  {
    // ESP_LOGD(TAG, "Message RX queue empty. Status code: 0x%0X", statusCode);
  }

  return statusCode;
}
