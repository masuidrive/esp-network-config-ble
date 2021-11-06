#include "esp-nimble-nordic-uart.h"
#include "esp-smartconfig-ble.h"
#include "mqtt_util.h"
#include "wifi_util.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"

static const char *TAG = "CHECK_MQTT";

void command_CHECK_MQTT(int argc, const char *args[], int datac, const char *data[]) {
  if (wifi_connect_with_nvs(6, NULL) != ESP_OK) {
    nordic_uart_sendln("ERROR: Failed to connect");
    nordic_uart_sendln("");
    ESP_LOGI(TAG, "Failed to connect");
    return;
  }
  ESP_LOGI(TAG, "Successed to connect");
  if (mqtt_connect_with_nvs(NULL, NULL) == ESP_OK) {
    nordic_uart_sendln("OK");
    nordic_uart_sendln("");
  } else {
    nordic_uart_sendln("FAILED");
    nordic_uart_sendln("");
  }
  mqtt_disconnect();
}
