#include "network-config-ble-internal.h"
static const char *TAG = "NCB_DEVICEID";

void _ncb_command_DEVICE_ID(int argc, const char *args[], int datac, const char *data[]) {
  uint8_t mac_addr[6] = {0};

  _CATCH_ESP_FAIL(esp_read_mac(mac_addr, ESP_MAC_WIFI_STA), "esp_read_mac");

  SEND_RESULT_FORMAT("%02x%02x%02x", mac_addr[3], mac_addr[4], mac_addr[5]);

  SEND_OK();
  return;

esp_failed:
  SEND_ESP_ERROR();
}
