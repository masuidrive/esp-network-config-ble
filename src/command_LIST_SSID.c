#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esp-nimble-nordic-uart.h"
#include "esp-smartconfig-ble.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define MAX_SSID_COUNT 64
#define LINE_LENGTH 65

static const char *TAG = "LIST_SSID";

void command_LIST_SSID(int argc, const char *args[], int datac, const char *data[]) {
  const wifi_scan_config_t scanConf = {
      .ssid = NULL, .bssid = NULL, .channel = 0, .show_hidden = true, .scan_type = WIFI_SCAN_TYPE_ACTIVE};
  ESP_ERROR_CHECK(esp_wifi_scan_start(&scanConf, true));
  char buf[LINE_LENGTH];

  uint16_t ap_count = 0;
  esp_wifi_scan_get_ap_num(&ap_count);
  if (ap_count == 0) {
    nordic_uart_sendln("");
    return;
  }

  uint16_t ap_record_count = MIN((uint16_t)MAX_SSID_COUNT, ap_count);
  wifi_ap_record_t ap_record_list[MAX_SSID_COUNT];
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_record_count, ap_record_list));
  ESP_LOGI(TAG, "ap_record_count = %d", ap_record_count);

  for (int i = 0; i < ap_record_count; i++) {
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_wifi.html#_CPPv416wifi_auth_mode_t
    snprintf(buf, LINE_LENGTH, "%d,%d,%s", ap_record_list[i].authmode, ap_record_list[i].rssi,
             (const char *)ap_record_list[i].ssid);
    nordic_uart_sendln(buf);
  }
  nordic_uart_sendln("");
}
