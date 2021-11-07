#pragma once
#include "esp-nimble-nordic-uart.h"
#include "esp-smartconfig-ble.h"
#include "mqtt_util.h"
#include "tokenizer.h"
#include "wifi_util.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <freertos/event_groups.h>
#include <freertos/freertos.h>
#include <freertos/ringbuf.h>
#include <freertos/task.h>

#include <esp_log.h>
#include <esp_netif.h>
#include <esp_tls.h>
#include <esp_wifi.h>
#include <mqtt_client.h>
#include <nvs_flash.h>

#define NVS_NAMESPACE "bleconfig"
#define WIFI_TRY_CONNECT_RETRY 5 // Number of retries during connection test

// clang-format off
#define CATCH_ESP_FAIL(cmd) if ((cmd) != ESP_OK) { goto esp_failed; }
