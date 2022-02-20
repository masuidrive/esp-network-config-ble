#include "network-config-ble-internal.h"

static const char *_TAG = "NCB_OTA";

static int _ota_size = 0;
static const esp_partition_t *_ota_part;
static esp_ota_handle_t _ota_handle;
static void _ota_receiver(struct ble_gatt_access_ctxt *ctxt) {
  ESP_ERROR_CHECK(esp_ota_write(_ota_handle, ctxt->om->om_data, ctxt->om->om_len));
  _ota_size -= ctxt->om->om_len;
  if (_ota_size < 0) {
    ESP_ERROR_CHECK(esp_ota_end(_ota_handle));
    ESP_ERROR_CHECK(esp_ota_set_boot_partition(_ota_part));
    ESP_LOGI(_TAG, "esp_ota_set_boot_partition succeeded");
    esp_restart();
  }
}

void _ncb_command_OTA(int argc, const char *args[], int datac, const char *data[]) {
  if (argc >= 1) {
    _ota_size = atoi(args[0]);
    _ota_part = esp_ota_get_next_update_partition(NULL);
    _NCB_CATCH_ESP_ERR(esp_ota_begin(_ota_part, _ota_size, &_ota_handle), "esp_ota_begin");
    nordic_uart_yield(_ota_receiver);
    _NCB_SEND_OK();
  } else {
    _NCB_SEND_ERROR("ignore command format");
  }
  return;

esp_failed:
  _NCB_SEND_ESP_ERR();
}
