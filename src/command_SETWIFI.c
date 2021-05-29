#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "connect_wifi.h"
#include "esp-nimble-nordic-uart.h"
#include "esp-smartconfig-ble.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"

static const char *TAG = "SETWIFI";

void command_SETWIFI(int argc, const char *args[], int datac, const char *data[]) {
  if (argc < 1) {
    nordic_uart_sendln("ERROR \"SSID is blank\"");
    nordic_uart_sendln("");
    return;
  }

  if (connect_wifi(args[0], args[1]) == ESP_OK) {
    // save wifi config
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open("wifi", NVS_READWRITE, &nvs_handle));
    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "ssid", args[0]));
    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "password", args[1]));
    nvs_close(nvs_handle);

    nordic_uart_sendln("OK");
    ESP_LOGI(TAG, "Successed to connect to SSID:%s, password:%s", args[0], args[1]);
  } else {
    nordic_uart_sendln("ERROR: Failed to connect");
    ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", args[0], args[1]);
  }
  esp_wifi_stop();

  nordic_uart_sendln("");
}
