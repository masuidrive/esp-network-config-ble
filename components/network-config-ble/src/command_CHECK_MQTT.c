#include "network-config-ble-internal.h"
static const char *TAG = "NCB_CHECK_MQTT";

void _ncb_command_CHECK_MQTT(int argc, const char *args[], int datac, const char *data[]) {
  ncb_mqtt_disconnect();
  ncb_wifi_disconnect();
  _NCB_CATCH_ESP_ERR(ncb_wifi_connect_with_nvs(_NCB_WIFI_CONNECT_RETRY, NULL), "ncb_wifi_connect_with_nvs");
  _NCB_CATCH_ESP_ERR(ncb_mqtt_connect_with_nvs(NULL, NULL), "ncb_mqtt_connect_with_nvs");
  ncb_mqtt_disconnect();
  ncb_wifi_disconnect();

  _NCB_SEND_OK();
  return;

esp_failed:
  ncb_mqtt_disconnect();
  ncb_wifi_disconnect();
  _NCB_SEND_ESP_ERR();
}
