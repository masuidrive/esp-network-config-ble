#pragma once
#include "esp-smartconfig-ble.h"
#include "mqtt_util.h"
#include "nimble-nordic-uart.h"
#include "tokenizer.h"
#include "wifi_util.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <freertos/FreeRTOS.h>

#include <freertos/event_groups.h>
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
extern const char* esp_fail_err_msg;
extern esp_err_t esp_fail_err_code;
#define CATCH_ESP_FAIL(cmd, msg) if((msg) != NULL) esp_fail_err_msg = (msg); if((cmd) != ESP_OK) { esp_fail_err_code = cmd; goto esp_failed; }

#define ERROR_MESSAGE_LENGTH 256
#define SEND_RESULT(result) { nordic_uart_sendln((result)); nordic_uart_sendln(""); }
#define SEND_RESULT_FORMAT(format, ...) { \
  char* __error_message__ = malloc(ERROR_MESSAGE_LENGTH); \
  snprintf(__error_message__, ERROR_MESSAGE_LENGTH, format, ##__VA_ARGS__);    \
  nordic_uart_sendln(__error_message__); \
  free(__error_message__); \
  nordic_uart_sendln(""); \
}
#define SEND_OK() SEND_RESULT("OK"); ESP_LOGI(TAG, "Send OK in %s", __FUNCTION__);
#define SEND_ESP_ERROR() SEND_RESULT_FORMAT("ERROR %s %s", (TAG), esp_fail_err_msg); ESP_LOGI(TAG, "Send ERROR: %s (%x) in %s", esp_fail_err_msg, esp_fail_err_code, __FUNCTION__);
