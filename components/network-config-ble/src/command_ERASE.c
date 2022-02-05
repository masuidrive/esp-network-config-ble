#include "network-config-ble-internal.h"
static const char *TAG = "NCB_ERASE";

#define VALUE_SIZE 4000

void _ncb_command_ERASE(int argc, const char *args[], int datac, const char *data[]) {
  nvs_handle_t nvs_handle;
  _CATCH_ESP_FAIL(nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle), "nvs_open");
  _CATCH_ESP_FAIL(nvs_erase_all(nvs_handle), "nvs_erase_all");
  nvs_close(nvs_handle);

  SEND_OK();
  return;

esp_failed:
  if (nvs_handle)
    nvs_close(nvs_handle);
  SEND_ESP_ERROR();
}
