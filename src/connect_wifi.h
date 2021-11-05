#pragma once

#include "esp_err.h"

enum wifi_callback_status {
  WIFI_CONNECTED,
  WIFI_DISCONNECTED,
};
typedef void (*wifi_status_callback)(enum wifi_callback_status status);

esp_err_t connect_wifi(const char *ssid, const char *password);
esp_err_t connect_wifi_with_nvs(wifi_status_callback status_callback);