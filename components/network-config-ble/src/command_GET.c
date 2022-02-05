#include "network-config-ble-internal.h"
static const char *TAG = "NCB_GET";

void _ncb_command_GET_STR(int argc, const char *args[], int datac, const char *data[]) {
  if (argc != 1) {
    _NCB_SEND_RESULT("ERROR: ignore command format");
    return;
  }

  nvs_handle_t nvs_handle;
  ESP_ERROR_CHECK(nvs_open(_NCB_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle));

  size_t required_size;
  char *value = NULL;
  _NCB_CATCH_ESP_ERR(nvs_get_str(nvs_handle, args[0], NULL, &required_size), "nvs_get_str");
  value = malloc(required_size);
  _NCB_CATCH_ESP_ERR(nvs_get_str(nvs_handle, args[0], value, &required_size), "nvs_get_str");
  nvs_close(nvs_handle);
  nordic_uart_sendln(value);
  free(value);

  _NCB_SEND_OK();
  return;

esp_failed:
  free(value);
  _NCB_SEND_ESP_ERR();
}
