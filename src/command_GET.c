#include "esp-smartconfig-ble-internal.h"
static const char *TAG = "GET";

#define VALUE_SIZE 4000

void command_GET_STR(int argc, const char *args[], int datac, const char *data[]) {
  if (argc != 1) {
    nordic_uart_sendln("ERROR: ignore command format");
    nordic_uart_sendln("");
    return;
  }

  nvs_handle_t nvs_handle;
  ESP_ERROR_CHECK(nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle));

  size_t required_size;
  ESP_ERROR_CHECK(nvs_get_str(nvs_handle, args[0], NULL, &required_size));
  char *value = malloc(required_size);
  ESP_ERROR_CHECK(nvs_get_str(nvs_handle, args[0], value, &required_size));
  nvs_close(nvs_handle);
  nordic_uart_sendln(value);

  free(value);
  nordic_uart_sendln("OK");
  nordic_uart_sendln("");
}
