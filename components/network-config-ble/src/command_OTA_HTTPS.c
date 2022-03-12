#include "network-config-ble-internal.h"

static const char *_TAG = "NCB_OTA_HTTPS";

void _ncb_command_OTA_HTTPS(int argc, const char *args[], int datac, const char *data[]) {
  if (argc >= 1) {
    esp_http_client_config_t config = {
        .url = args[0],
        .cert_pem = data,
    };
    _NCB_CATCH_ESP_ERR(esp_https_ota(&config), "esp_https_ota");
    esp_restart();
  } else {
    _NCB_SEND_ERROR("ignore command format");
  }
  return;

esp_failed:
  _NCB_SEND_ESP_ERR();
}
