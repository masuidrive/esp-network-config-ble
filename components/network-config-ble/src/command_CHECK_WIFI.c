#include "network-config-ble-internal.h"
static const char *_TAG = "NCB_CHECK_WIFI";

void _ncb_command_CHECK_WIFI(int argc, const char *args[], int datac, const char *data[]) {
  ncb_wifi_disconnect();
  _NCB_CATCH_ESP_ERR(ncb_wifi_connect_with_nvs(_NCB_WIFI_CONNECT_RETRY, NULL), NULL);
  ncb_wifi_disconnect();

  _NCB_SEND_OK();
  return;

esp_failed:
  _NCB_SEND_ESP_ERR();
}
