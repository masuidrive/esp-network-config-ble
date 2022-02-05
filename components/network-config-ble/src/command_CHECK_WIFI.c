#include "network-config-ble-internal.h"
static const char *TAG = "NCB_CHECK_WIFI";

void _ncb_command_CHECK_WIFI(int argc, const char *args[], int datac, const char *data[]) {
  ncb_wifi_disconnect();
  _CATCH_ESP_FAIL(ncb_wifi_connect_with_nvs(WIFI_TRY_CONNECT_RETRY, NULL), NULL);
  SEND_OK();
  ncb_wifi_disconnect();
  return;

esp_failed:
  SEND_ESP_ERROR();
}
