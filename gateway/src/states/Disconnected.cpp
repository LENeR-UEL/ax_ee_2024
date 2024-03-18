#include <Arduino.h>
#include <esp_log.h>
#include "../StateManager.h"

static const char *TAG = "DisconnectedState";

void onEnter_Disconnected()
{
    ESP_LOGI(TAG, "Entering...");
}

void onLoop_Disconnected()
{
    ESP_LOGI(TAG, "Main Loop...");
}

void onExit_Disconnected()
{
    ESP_LOGI(TAG, "Exit...");
}
