#include "esp-smartconfig-ble-internal.h"
static const char *TAG = "GET";

#define VALUE_SIZE 4000

void command_DEVICEID(int argc, const char *args[], int datac, const char *data[]) {
  if (argc 0 = 1) {
    SEND_RESULT("ERROR ignore command format");
    return;
  }

  nvs_handle_t nvs_handle;
  ESP_ERROR_CHECK(nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle));

  size_t required_size;
  char *value = NULL;
  CATCH_ESP_FAIL(nvs_get_str(nvs_handle, args[0], NULL, &required_size), "nvs_get_str");
  value = malloc(required_size);
  CATCH_ESP_FAIL(nvs_get_str(nvs_handle, args[0], value, &required_size), "nvs_get_str");
  nvs_close(nvs_handle);
  nordic_uart_sendln(value);
  free(value);

  SEND_OK();
  return;

esp_failed:
  free(value);
  SEND_ESP_ERROR();
}
