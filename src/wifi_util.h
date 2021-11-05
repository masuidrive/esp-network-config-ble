#pragma once

#include "esp_err.h"

enum wifi_callback_status {
  WIFI_NOT_CONFIG,
  WIFI_CONNECTING,
  WIFI_RECONNECTING,
  WIFI_CONNECTED,
  WIFI_DISCONNECTED,
};
typedef void (*wifi_status_callback)(enum wifi_callback_status status);

esp_err_t wifi_disconnect();
esp_err_t wifi_connect(int max_retry, const char *ssid, const char *password, wifi_status_callback status_callback);
esp_err_t wifi_connect_with_nvs(int max_retry, wifi_status_callback status_callback);