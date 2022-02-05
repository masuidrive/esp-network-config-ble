#pragma once
#include "network-config-ble.h"
#include "nimble-nordic-uart.h"

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
extern const char* _ncb_esp_fail_err_msg;
extern esp_err_t _ncb_esp_fail_err_code;
#define _CATCH_ESP_FAIL(cmd, msg) if((msg) != NULL) _ncb_esp_fail_err_msg = (msg); if((cmd) != ESP_OK) { _ncb_esp_fail_err_code = cmd; goto esp_failed; }

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
#define SEND_ESP_ERROR() SEND_RESULT_FORMAT("ERROR %s %s", (TAG), _ncb_esp_fail_err_msg); ESP_LOGI(TAG, "Send ERROR: %s (%x) in %s", _ncb_esp_fail_err_msg, _ncb_esp_fail_err_code, __FUNCTION__);

// wifi_util.c
enum ncb_wifi_status {
  NCB_WIFI_NONE,
  NCB_WIFI_CONNECTING,
  NCB_WIFI_RECONNECTING,
  NCB_WIFI_CONNECTED,
  NCB_WIFI_DISCONNECTED,
};
typedef void (*ncb_wifi_status_callback)(enum ncb_wifi_status status);

esp_err_t ncb_wifi_init();
esp_err_t ncb_wifi_disconnect();
esp_err_t ncb_wifi_connect(const char *ssid, const char *password, int max_retry, ncb_wifi_status_callback status_callback);
esp_err_t ncb_wifi_connect_with_nvs(int max_retry, ncb_wifi_status_callback status_callback);
bool ncb_wifi_is_connected();

// tokentizer.c
// get first string from space/enter splitted string.
// text: source string
// *token: tokenized string
// result: the other string
char *_ncb_get_token(char *text, char **token);

// mqtt_util.c
typedef void (*ncb_mqtt_message_receiver_callback)(const char *topic, const char *payload);
enum ncb_mqtt_status {
  MQTT_STARTING,
  MQTT_CONNECTED,
  MQTT_DISCONNECTED,
};
typedef void (*ncb_mqtt_status_callback)(enum ncb_mqtt_status status);

esp_err_t ncb_mqtt_connect_with_nvs(ncb_mqtt_message_receiver_callback message_callback, ncb_mqtt_status_callback status_callback);
esp_err_t ncb_mqtt_disconnect();


// commands
void _ncb_command_LIST_SSID(int argc, const char *args[], int datac, const char *data[]);
void _ncb_command_SET_WIFI(int argc, const char *args[], int datac, const char *data[]);
void _ncb_command_SET_STR(int argc, const char *args[], int datac, const char *data[]);
void _ncb_command_SET_MULTI(int argc, const char *args[], int datac, const char *data[]);
void _ncb_command_GET_STR(int argc, const char *args[], int datac, const char *data[]);
void _ncb_command_CHECK_MQTT(int argc, const char *args[], int datac, const char *data[]);
void _ncb_command_CHECK_WIFI(int argc, const char *args[], int datac, const char *data[]);
void _ncb_command_DEVICE_ID(int argc, const char *args[], int datac, const char *data[]);
void _ncb_command_ERASE(int argc, const char *args[], int datac, const char *data[]);