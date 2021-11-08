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

#include <freertos/freertos.h>

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
#define CATCH_ESP_FAIL(cmd, msg) if((msg) != NULL) esp_fail_err_msg = (msg); if((cmd) != ESP_OK) goto esp_failed;

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
#define SEND_ESP_ERROR() SEND_RESULT_FORMAT("ERROR %s %s", (TAG), esp_fail_err_msg); ESP_LOGI(TAG, "Send ERROR: %s in %s", esp_fail_err_msg, __FUNCTION__);

