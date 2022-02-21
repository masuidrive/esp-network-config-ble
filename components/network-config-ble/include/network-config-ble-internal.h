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
#include <sys/time.h>

#include <cJSON.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_ota_ops.h>
#include <esp_tls.h>
#include <esp_wifi.h>
#include <mqtt_client.h>
#include <nvs_flash.h>

#define _NCB_NVS_NAMESPACE "ncb"
#define _NCB_WIFI_CONNECT_RETRY 5 // Number of retries during connection test
#define _NCB_CONFIG_VALUE_SIZE 4000

extern char *_ncb_ble_device_name;
extern char *_ncb_firmware_version;
extern char *_ncb_device_id;
extern char *_ncb_device_type;

// clang-format off
extern const char* _ncb_esp_err_msg;
extern esp_err_t _ncb_esp_err_code;
#define _NCB_CATCH_ESP_ERR(cmd, msg) if((msg) != NULL) _ncb_esp_err_msg = (msg); if((cmd) != ESP_OK) { _ncb_esp_err_code = cmd; goto esp_failed; }

#define _NCB_RESULT_LENGTH 256
#define _NCB_SEND_FORMAT(format, ...) { \
  char* __error_message__ = malloc(_NCB_RESULT_LENGTH); \
  snprintf(__error_message__, _NCB_RESULT_LENGTH, format, ##__VA_ARGS__);    \
  nordic_uart_sendln(__error_message__); \
  free(__error_message__); \
}

#define _NCB_SEND_RESULT(result) { nordic_uart_sendln((result)); nordic_uart_sendln(""); }
#define _NCB_SEND_RESULT_FORMAT(format, ...) { \
  _NCB_SEND_FORMAT(format, ##__VA_ARGS__); \
  nordic_uart_sendln(""); \
}
#define _NCB_SEND_ERROR(result) { nordic_uart_send("ERROR "); nordic_uart_sendln((result)); }

#define _NCB_SEND_OK() _NCB_SEND_RESULT("OK"); ESP_LOGI(_TAG, "Send OK in %s", __FUNCTION__);
#define _NCB_SEND_ESP_ERR() _NCB_SEND_RESULT_FORMAT("ERROR %s %s", (_TAG), _ncb_esp_err_msg); ESP_LOGI(_TAG, "Send ERROR: %s (%x) in %s", _ncb_esp_err_msg, _ncb_esp_err_code, __FUNCTION__);

// tokentizer.c
// get first string from space/enter splitted string.
// text: source string
// *token: tokenized string
// result: the other string
char *_ncb_get_token(char *text, char **token);

// commands
void _ncb_command_LIST_SSID(int argc, const char *args[], int datac, const char *data[]);
void _ncb_command_SET_WIFI(int argc, const char *args[], int datac, const char *data[]);
void _ncb_command_SET_STR(int argc, const char *args[], int datac, const char *data[]);
void _ncb_command_SET_MULTI(int argc, const char *args[], int datac, const char *data[]);
void _ncb_command_GET_STR(int argc, const char *args[], int datac, const char *data[]);
void _ncb_command_ERASE_ALL(int argc, const char *args[], int datac, const char *data[]);
void _ncb_command_CHECK_MQTT(int argc, const char *args[], int datac, const char *data[]);
void _ncb_command_CHECK_WIFI(int argc, const char *args[], int datac, const char *data[]);
void _ncb_command_DEVICE_ID(int argc, const char *args[], int datac, const char *data[]);
void _ncb_command_RESTART(int argc, const char *args[], int datac, const char *data[]);
void _ncb_command_OTA(int argc, const char *args[], int datac, const char *data[]);
