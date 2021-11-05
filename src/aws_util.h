#pragma once

#include "aws_iot_mqtt_client_interface.h"
#include "esp_err.h"

typedef void (*awsiot_mqtt_receiver_callback)(const char *topic, const char *payload);
enum awsiot_callback_status {
  AWSIOT_STARTING,
  AWSIOT_CONNECTED,
  AWSIOT_DISCONNECTED,
};
typedef void (*awsiot_status_callback)(enum awsiot_callback_status status);

esp_err_t awsiot_connect_with_nvs(awsiot_mqtt_receiver_callback message_callback,
                                  awsiot_status_callback status_callback);
esp_err_t awsiot_disconnect();
void awsiot_loop_task(void *pvParameters);