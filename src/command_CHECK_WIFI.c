#include "esp-smartconfig-ble-internal.h"
static const char *TAG = "CHECK_WIFI";

void command_CHECK_WIFI(int argc, const char *args[], int datac, const char *data[]) {
  CATCH_ESP_FAIL(wifi_connect_with_nvs(WIFI_TRY_CONNECT_RETRY, NULL) == ESP_OK);
  nordic_uart_sendln("OK");
  nordic_uart_sendln("");
  ESP_LOGI(TAG, "Successed to connect");
  return;

esp_failed:
  nordic_uart_sendln("ERROR: Failed to connect");
  nordic_uart_sendln("");
  ESP_LOGI(TAG, "Failed to connect");
}
