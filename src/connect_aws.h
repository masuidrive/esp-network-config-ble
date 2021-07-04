#pragma once

#include "aws_iot_mqtt_client_interface.h"
#include "esp_err.h"

esp_err_t awsiot_connect_with_nvs();
esp_err_t awsiot_disconnect();
void awsiot_loop_task(void *pvParameters);