#include "esp-smartconfig-ble-internal.h"
static const char *TAG = "ERASE";

#define VALUE_SIZE 4000

void command_ERASE(int argc, const char *args[], int datac, const char *data[]) {
  nvs_handle_t nvs_handle = NULL;
  CATCH_ESP_FAIL(nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle));
  CATCH_ESP_FAIL(nvs_erase_all(nvs_handle));
  nvs_close(nvs_handle);

  nordic_uart_sendln("OK");
  nordic_uart_sendln("");
  ESP_LOGI(TAG, "Succeed");
  return;

esp_failed:
  nordic_uart_sendln("ERROR");
  nordic_uart_sendln("");
  if (nvs_handle)
    nvs_close(nvs_handle);

  ESP_LOGI(TAG, "Failed");
}
