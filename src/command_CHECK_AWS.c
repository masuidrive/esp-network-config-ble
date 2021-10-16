#include "aws_iot_mqtt_client_interface.h"
#include "connect_aws.h"
#include "connect_wifi.h"
#include "esp-nimble-nordic-uart.h"
#include "esp-smartconfig-ble.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"

static const char *TAG = "CHECK AWSIOT";

void command_CHECK_AWSIOT(int argc, const char *args[], int datac, const char *data[]) {
  if (connect_wifi_with_nvs() != ESP_OK) {
    nordic_uart_sendln("ERROR: Failed to connect");
    nordic_uart_sendln("");
    ESP_LOGI(TAG, "Failed to connect");
    return;
  }
  ESP_LOGI(TAG, "Successed to connect");
  if (awsiot_connect_with_nvs(NULL) == ESP_OK) {
    nordic_uart_sendln("OK");
    nordic_uart_sendln("");
  } else {
    nordic_uart_sendln("FAILED");
    nordic_uart_sendln("");
  }
  awsiot_disconnect();
}
