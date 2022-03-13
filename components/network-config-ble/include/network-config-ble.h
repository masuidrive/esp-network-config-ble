#pragma once
#include "esp_err.h"
#include "esp_sntp.h"
#include <stdbool.h>

// config.c
struct ncb_command {
  const char *name;
  bool multiline;
  void (*func)(int argc, const char *args[], int datac, const char *data[]);
};

enum ncb_callback_type {
  NCB_WAIT_CONNECT,
  NCB_PROCESSING,
  NCB_OTA_PROCESSING,
};

esp_err_t ncb_config_start(const char *device_id, const char *ble_device_name, const char *firmware_version,
                           const char *device_type, const struct ncb_command extend_commands[],
                           size_t extend_commands_count,
                           void (*callback)(enum ncb_callback_type callback_type, int param1));

void ncb_config_stop();

// wifi_util.c
enum ncb_wifi_status {
  NCB_WIFI_NONE,
  NCB_WIFI_INITIALIAED,
  NCB_WIFI_CONNECTING,
  NCB_WIFI_RECONNECTING,
  NCB_WIFI_CONNECTED,
  NCB_WIFI_STOPPING,
  NCB_WIFI_DISCONNECTED,
};
typedef void (*ncb_wifi_status_callback)(enum ncb_wifi_status status);

esp_err_t ncb_wifi_init();
bool ncb_wifi_is_configured();
esp_err_t ncb_wifi_disconnect();
esp_err_t ncb_wifi_connect(const char *ssid, const char *password, int max_retry,
                           ncb_wifi_status_callback status_callback);
esp_err_t ncb_wifi_connect_with_nvs(int max_retry, ncb_wifi_status_callback status_callback);
bool ncb_wifi_is_connected();
esp_err_t ncb_mqtt_publish_to_response_topic(const char *message);

// mqtt_util.c
enum ncb_mqtt_status {
  MQTT_STARTING,
  MQTT_CONNECTED,
  MQTT_DISCONNECTED,
};
typedef void (*ncb_mqtt_status_callback)(enum ncb_mqtt_status status);
typedef void (*ncb_mqtt_message_receiver_callback)(const char *topic, const char *payload);
esp_err_t ncb_mqtt_connect_with_nvs(ncb_mqtt_message_receiver_callback message_callback,
                                    ncb_mqtt_status_callback status_callback);
esp_err_t ncb_mqtt_disconnect();
