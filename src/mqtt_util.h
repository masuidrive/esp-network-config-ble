#pragma once

#include "esp_err.h"

typedef void (*mqtt_message_receiver_callback)(const char *topic, const char *payload);
enum mqtt_status {
  MQTT_STARTING,
  MQTT_CONNECTED,
  MQTT_DISCONNECTED,
};
typedef void (*mqtt_status_callback)(enum mqtt_status status);

esp_err_t mqtt_connect_with_nvs(mqtt_message_receiver_callback message_callback, mqtt_status_callback status_callback);
esp_err_t mqtt_disconnect();
