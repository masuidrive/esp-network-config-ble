#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esp-nimble-nordic-uart.h"
#include "esp-smartconfig-ble.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"

static const char *TAG = "SET";

#define VALUE_SIZE 4000

void command_SET_STR(int argc, const char *args[], int datac, const char *data[]) {
  if (argc == 2) {
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open("bleconfig", NVS_READWRITE, &nvs_handle));
    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, args[0], args[1]));
    nvs_close(nvs_handle);
    nordic_uart_sendln("OK");
    nordic_uart_sendln("");
  } else {
    nordic_uart_sendln("Error: ignore command format");
  }
}

void command_SET_MULTI(int argc, const char *args[], int datac, const char *data[]) {
  if (argc != 1) {
    nordic_uart_sendln("Error: ignore command format");
    return;
  }

  char *value = malloc(VALUE_SIZE);
  value[0] = '\0';
  size_t value_size = 0;

  int line_length;
  for (int i = 0; i < datac; i++) {
    line_length = strlen(data[i]);
    if (value_size + line_length + 1 + 1 < VALUE_SIZE) {
      strlcat(value, data[i], VALUE_SIZE);
      strlcat(value, "\n", VALUE_SIZE);
    } else {
      ESP_LOGE(TAG, "SET_MULTI overflow");
    }
  }

  nvs_handle_t nvs_handle;
  ESP_ERROR_CHECK(nvs_open("bleconfig", NVS_READWRITE, &nvs_handle));
  ESP_ERROR_CHECK(nvs_set_str(nvs_handle, args[0], value));
  nvs_close(nvs_handle);

  free(value);
  nordic_uart_sendln("OK");
  nordic_uart_sendln("");
}
