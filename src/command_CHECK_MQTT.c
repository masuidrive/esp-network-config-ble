#include "esp-smartconfig-ble-internal.h"
static const char *TAG = "CHECK_MQTT";

void command_CHECK_MQTT(int argc, const char *args[], int datac, const char *data[]) {
  CATCH_ESP_FAIL(wifi_connect_with_nvs(WIFI_TRY_CONNECT_RETRY, NULL));
  CATCH_ESP_FAIL(mqtt_connect_with_nvs(NULL, NULL));
  nordic_uart_sendln("OK");
  nordic_uart_sendln("");
  mqtt_disconnect();
  ESP_LOGI(TAG, "Succeed");
  return;

esp_failed:
  nordic_uart_sendln("ERROR");
  nordic_uart_sendln("");
  mqtt_disconnect();
  ESP_LOGI(TAG, "Failed to connect");
}
