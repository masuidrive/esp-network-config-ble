#include "network-config-ble-internal.h"
static const char *TAG = "NCB_LIST_SSID";

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define MAX_SSID_COUNT 64
#define LINE_LENGTH 65

void _ncb_command_LIST_SSID(int argc, const char *args[], int datac, const char *data[]) {
  const wifi_scan_config_t scanConf = {
      .ssid = NULL, .bssid = NULL, .channel = 0, .show_hidden = true, .scan_type = WIFI_SCAN_TYPE_ACTIVE};
  _NCB_CATCH_ESP_ERR(esp_wifi_scan_start(&scanConf, true), "esp_wifi_scan_start");
  char buf[LINE_LENGTH];

  uint16_t ap_count = 0;
  esp_wifi_scan_get_ap_num(&ap_count);
  if (ap_count == 0) {
    nordic_uart_sendln("");
    return;
  }

  uint16_t ap_record_count = MIN((uint16_t)MAX_SSID_COUNT, ap_count);
  wifi_ap_record_t ap_record_list[MAX_SSID_COUNT];
  _NCB_CATCH_ESP_ERR(esp_wifi_scan_get_ap_records(&ap_record_count, ap_record_list), "esp_wifi_scan_get_ap_records");
  ESP_LOGI(TAG, "ap_record_count = %d", ap_record_count);

  for (int i = 0; i < ap_record_count; i++) {
    const wifi_ap_record_t *ap = &ap_record_list[i];
    ESP_LOGI(TAG, "Found SSID: %s, authmode: %d, rssi: %d", (const char *)ap->ssid, ap->authmode, ap->rssi);
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_wifi.html#_CPPv416wifi_auth_mode_t
    snprintf(buf, LINE_LENGTH, "%d,%d,%s", ap->authmode, ap->rssi, (const char *)ap->ssid);
    nordic_uart_sendln(buf);
  }

  _NCB_SEND_OK();
  return;

esp_failed:
  _NCB_SEND_ESP_ERR();
}
