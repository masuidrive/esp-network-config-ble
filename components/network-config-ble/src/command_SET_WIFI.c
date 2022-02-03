#include "network-config-ble-internal.h"
static const char *TAG = "NCB_SETWIFI";

void _ncb_command_SET_WIFI(int argc, const char *args[], int datac, const char *data[]) {
  if (argc < 1) {
    nordic_uart_sendln("ERROR \"SSID is blank\"");
    return;
  }

  const char *ssid = args[0];
  const char *password = argc < 2 ? "" : args[1];

  if (ncb_wifi_connect(ssid, password, 5, NULL) == ESP_OK) {
    // save wifi config
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle));
    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "ssid", ssid));
    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "password", password));
    nvs_close(nvs_handle);

    nordic_uart_sendln("OK");
    ESP_LOGI(TAG, "Successed to connect to SSID:%s, password:[%s]", ssid, password);
  } else {
    nordic_uart_sendln("ERROR: Failed to connect");
    ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:[%s]", ssid, password);
  }
  esp_wifi_stop();

  nordic_uart_sendln("");
}
