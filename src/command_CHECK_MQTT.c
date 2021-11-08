#include "esp-smartconfig-ble-internal.h"
static const char *TAG = "CHECK_MQTT";

void command_CHECK_MQTT(int argc, const char *args[], int datac, const char *data[]) {
  CATCH_ESP_FAIL(wifi_connect_with_nvs(WIFI_TRY_CONNECT_RETRY, NULL), "wifi_connect_with_nvs");
  CATCH_ESP_FAIL(mqtt_connect_with_nvs(NULL, NULL), "mqtt_connect_with_nvs");
  mqtt_disconnect();
  SEND_OK();
  return;

esp_failed:
  mqtt_disconnect();
  SEND_ESP_ERROR();
}
