#pragma once

#include "esp_err.h"

esp_err_t connect_wifi(const char *ssid, const char *password);
esp_err_t connect_wifi_with_nvs();