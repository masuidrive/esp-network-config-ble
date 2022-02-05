#include "network-config-ble-internal.h"
static const char *_TAG = "NCB_SET";

void _ncb_command_SET_STR(int argc, const char *args[], int datac, const char *data[]) {
  nvs_handle_t nvs_handle;
  if (argc >= 2) {
    _NCB_CATCH_ESP_ERR(nvs_open(_NCB_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle), "nvs_open");
    _NCB_CATCH_ESP_ERR(nvs_set_str(nvs_handle, args[0], args[1]), "nvs_set_str");
    nvs_close(nvs_handle);
    _NCB_SEND_OK();
  } else {
    _NCB_SEND_ERROR("ignore command format");
  }
  return;

esp_failed:
  if (nvs_handle)
    nvs_close(nvs_handle);

  _NCB_SEND_ESP_ERR();
}

void _ncb_command_SET_MULTI(int argc, const char *args[], int datac, const char *data[]) {
  if (argc != 1) {
    _NCB_SEND_ERROR("ignore command format");
    return;
  }
  const char *name = args[0];

  char *value = malloc(_NCB_CONFIG_VALUE_SIZE);
  value[0] = '\0';
  size_t value_size = 0;

  int line_length;
  for (int i = 0; i < datac; i++) {
    line_length = strlen(data[i]);
    if (value_size + line_length + 1 + 1 < _NCB_CONFIG_VALUE_SIZE) {
      strlcat(value, data[i], _NCB_CONFIG_VALUE_SIZE);
      strlcat(value, "\n", _NCB_CONFIG_VALUE_SIZE);
    } else {
      ESP_LOGE(_TAG, "over size");
      _NCB_SEND_ERROR("over size");
      free(value);
      return;
    }
  }

  nvs_handle_t nvs_handle;
  _NCB_CATCH_ESP_ERR(nvs_open(_NCB_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle), "nvs_open");
  _NCB_CATCH_ESP_ERR(nvs_set_str(nvs_handle, name, value), "nvs_set_str");
  nvs_close(nvs_handle);
  free(value);

  _NCB_SEND_OK();
  return;

esp_failed:
  free(value);
  _NCB_SEND_ESP_ERR();
}
