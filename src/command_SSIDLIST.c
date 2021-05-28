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
#define SSID_STR_LENGTH 32

void command_SSIDLIST(const char *args[], const char *data) {

  const wifi_scan_config_t scanConf = {
      .ssid = NULL, .bssid = NULL, .channel = 0, .show_hidden = true, .scan_type = WIFI_SCAN_TYPE_ACTIVE};

  ESP_ERROR_CHECK(esp_wifi_scan_start(&scanConf, true));

  uint16_t ap_count = 0;
  esp_wifi_scan_get_ap_num(&ap_count);
  if (ap_count == 0) {
    nordic_uart_sendln("");
    return;
  }

  uint16_t ap_record_count = MIN((uint16_t)MAX_SSID_COUNT, ap_count);
  wifi_ap_record_t ap_record_list[MAX_SSID_COUNT];
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_record_count, ap_record_list));

  for (int i = 0; i < ap_record_count; i++) {
    if (strlen((const char *)ap_record_list[i].ssid) == 0)
      continue;
    bool duplicated = false;
    for (int j = 0; j < i; ++j) {
      if (strcmp((const char *)ap_record_list[j].ssid, (const char *)ap_record_list[i].ssid) == 0) {
        duplicated = true;
        break;
      }
    }
    if (!duplicated) {
      nordic_uart_sendln((const char *)ap_record_list[i].ssid);
    }
  }
  nordic_uart_sendln("");
}
