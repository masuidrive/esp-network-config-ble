#include "network-config-ble-internal.h"
static const char *TAG = "NCB_CHECK_MQTT";

void _ncb_command_CHECK_MQTT(int argc, const char *args[], int datac, const char *data[]) {
  ncb_mqtt_disconnect();
  ncb_wifi_disconnect();
  ESP_LOGE(TAG, "CHECK_MQTT> 1");
  CATCH_ESP_FAIL(ncb_wifi_connect_with_nvs(WIFI_TRY_CONNECT_RETRY, NULL), "ncb_wifi_connect_with_nvs");
  ESP_LOGE(TAG, "CHECK_MQTT> 2");
  CATCH_ESP_FAIL(ncb_mqtt_connect_with_nvs(NULL, NULL), "ncb_mqtt_connect_with_nvs");
  ESP_LOGE(TAG, "CHECK_MQTT> 3");
  ncb_mqtt_disconnect();
  ESP_LOGE(TAG, "CHECK_MQTT> 4");
  ncb_wifi_disconnect();
  ESP_LOGE(TAG, "CHECK_MQTT> 5");
  SEND_OK();
  return;

esp_failed:
  ESP_LOGE(TAG, "CHECK_MQTT> esp_failed");
  ncb_mqtt_disconnect();
  ncb_wifi_disconnect();
  SEND_ESP_ERROR();
}
