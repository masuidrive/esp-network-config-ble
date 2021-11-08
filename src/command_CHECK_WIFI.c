#include "esp-smartconfig-ble-internal.h"
static const char *TAG = "CHECK_WIFI";

void command_CHECK_WIFI(int argc, const char *args[], int datac, const char *data[]) {
  CATCH_ESP_FAIL(wifi_connect_with_nvs(WIFI_TRY_CONNECT_RETRY, NULL) == ESP_OK, NULL);
  SEND_OK();
  return;

esp_failed:
  SEND_ESP_ERROR();
}
