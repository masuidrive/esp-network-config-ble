#pragma once

#include "esp_err.h"
#include <stdbool.h>

enum wifi_callback_status {
  WIFI_NOT_CONFIG,
  WIFI_CONNECTING,
  WIFI_RECONNECTING,
  WIFI_CONNECTED,
  WIFI_DISCONNECTED,
};
typedef void (*wifi_status_callback)(enum wifi_callback_status status);

esp_err_t wifi_init();
esp_err_t wifi_disconnect();
esp_err_t wifi_connect(const char *ssid, const char *password, int max_retry, wifi_status_callback status_callback);
esp_err_t wifi_connect_with_nvs(int max_retry, wifi_status_callback status_callback);
bool wifi_is_connected();