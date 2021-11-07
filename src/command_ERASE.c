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

static const char *TAG = "ERASE";

#define VALUE_SIZE 4000

void command_ERASE(int argc, const char *args[], int datac, const char *data[]) {
  if (argc == 0) {
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open("bleconfig", NVS_READWRITE, &nvs_handle));
    ESP_ERROR_CHECK(nvs_erase_all(nvs_handle));
    nvs_close(nvs_handle);
    nordic_uart_sendln("OK");
    nordic_uart_sendln("");
  } else {
    nordic_uart_sendln("Error: ignore command format");
  }
}
