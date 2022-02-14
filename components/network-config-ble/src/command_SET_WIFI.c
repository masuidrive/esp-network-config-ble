#include "network-config-ble-internal.h"
static const char *_TAG = "NCB_SETWIFI";

void _ncb_command_SET_WIFI(int argc, const char *args[], int datac, const char *data[]) {
  if (argc < 1) {
    _NCB_SEND_ERROR("\"SSID is blank\"");
    return;
  }

  const char *ssid = args[0];
  const char *password = argc < 2 ? "" : args[1];

  _NCB_CATCH_ESP_ERR(ncb_wifi_connect(ssid, password, _NCB_WIFI_CONNECT_RETRY, NULL), "ncb_wifi_connect");

  // save wifi config
  nvs_handle_t nvs_handle;
  _NCB_CATCH_ESP_ERR(nvs_open(_NCB_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle), "nvs_open");
  _NCB_CATCH_ESP_ERR(nvs_set_str(nvs_handle, "ssid", ssid), "nvs_set_str");
  _NCB_CATCH_ESP_ERR(nvs_set_str(nvs_handle, "password", password), "nvs_set_str");
  nvs_close(nvs_handle);

  ESP_LOGI(_TAG, "Successed to connect to SSID:%s, password:[%s]", ssid, password);
  _NCB_SEND_OK();
  return;

esp_failed:
  _NCB_SEND_ESP_ERR();
}
