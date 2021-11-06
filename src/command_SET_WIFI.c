#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esp-nimble-nordic-uart.h"
#include "esp-smartconfig-ble.h"
#include "wifi_util.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"

static const char *TAG = "SETWIFI";

void command_SET_WIFI(int argc, const char *args[], int datac, const char *data[]) {
  if (argc < 1) {
    nordic_uart_sendln("ERROR \"SSID is blank\"");
    return;
  }

  const char *ssid = args[0];
  const char *password = argc < 2 ? "" : args[1];

  if (wifi_connect(ssid, password, 10, NULL) == ESP_OK) {
    // save wifi config
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open("wifi", NVS_READWRITE, &nvs_handle));
    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "ssid", ssid));
    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "password", password));
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
