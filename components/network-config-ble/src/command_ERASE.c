#include "network-config-ble-internal.h"
static const char *TAG = "NCB_ERASE";

void _ncb_command_ERASE_ALL(int argc, const char *args[], int datac, const char *data[]) {
  nvs_handle_t nvs_handle;
  _NCB_CATCH_ESP_ERR(nvs_open(_NCB_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle), "nvs_open");
  _NCB_CATCH_ESP_ERR(nvs_erase_all(nvs_handle), "nvs_erase_all");
  nvs_close(nvs_handle);

  _NCB_SEND_OK();
  return;

esp_failed:
  if (nvs_handle)
    nvs_close(nvs_handle);
  _NCB_SEND_ESP_ERR();
}
