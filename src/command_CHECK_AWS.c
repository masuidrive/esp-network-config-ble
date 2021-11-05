#include "aws_iot_mqtt_client_interface.h"
#include "aws_util.h"
#include "esp-nimble-nordic-uart.h"
#include "esp-smartconfig-ble.h"
#include "wifi_util.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"

static const char *TAG = "CHECK AWSIOT";

void command_CHECK_AWSIOT(int argc, const char *args[], int datac, const char *data[]) {
  if (wifi_connect_with_nvs(3, NULL) != ESP_OK) {
    nordic_uart_sendln("ERROR: Failed to connect");
    nordic_uart_sendln("");
    ESP_LOGI(TAG, "Failed to connect");
    return;
  }
  ESP_LOGI(TAG, "Successed to connect");
  if (awsiot_connect_with_nvs(NULL, NULL) == ESP_OK) {
    nordic_uart_sendln("OK");
    nordic_uart_sendln("");
  } else {
    nordic_uart_sendln("FAILED");
    nordic_uart_sendln("");
  }
  awsiot_disconnect();
}
