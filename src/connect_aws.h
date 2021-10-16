#pragma once

#include "aws_iot_mqtt_client_interface.h"
#include "esp_err.h"

typedef void (*awsiot_mqtt_receiver_callback)(const char *topic, const char *payload);

esp_err_t awsiot_connect_with_nvs(awsiot_mqtt_receiver_callback callback);
esp_err_t awsiot_disconnect();
void awsiot_loop_task(void *pvParameters);