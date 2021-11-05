#include "connect_wifi.h"
#include "esp-nimble-nordic-uart.h"
#include "esp-smartconfig-ble.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"

static const char *TAG = "CHECK WIFI";

void command_CHECK_WIFI(int argc, const char *args[], int datac, const char *data[]) {
  if (connect_wifi_with_nvs(NULL) == ESP_OK) {
    nordic_uart_sendln("OK");
    ESP_LOGI(TAG, "Successed to connect");
  } else {
    nordic_uart_sendln("ERROR: Failed to connect");
    ESP_LOGI(TAG, "Failed to connect");
  }
  nordic_uart_sendln("");
}
