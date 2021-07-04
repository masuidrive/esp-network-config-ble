#pragma once

#include "aws_iot_mqtt_client_interface.h"
#include "esp_err.h"

esp_err_t connect_awsiot_with_nvs(AWS_IoT_Client *client);
esp_err_t disconnect_awsiot(AWS_IoT_Client *client);
