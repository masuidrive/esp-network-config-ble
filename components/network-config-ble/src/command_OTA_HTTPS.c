#include "network-config-ble-internal.h"

static const char *_TAG = "NCB_OTA_HTTPS";

void _ncb_command_OTA_HTTPS(int argc, const char *args[], int datac, const char *data[]) {
  if (argc >= 1) {
    char *cert = malloc(_NCB_CONFIG_VALUE_SIZE);
    cert[0] = '\0';
    size_t cert_size = 0;

    int line_length;
    for (int i = 0; i < datac; i++) {
      line_length = strlen(data[i]);
      if (cert_size + line_length + 1 + 1 < _NCB_CONFIG_VALUE_SIZE) {
        strlcat(cert, data[i], _NCB_CONFIG_VALUE_SIZE);
        strlcat(cert, "\n", _NCB_CONFIG_VALUE_SIZE);
      } else {
        free(cert);
        ESP_LOGE(_TAG, "cert over size");
        _NCB_SEND_ERROR("cert over size");
        return;
      }
    }

    _NCB_CATCH_ESP_ERR(ncb_wifi_connect_with_nvs(10, NULL), "Can't connect WiFi");

    esp_http_client_config_t config = {
        .url = args[0],
        .cert_pem = cert,
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };

    esp_https_ota_handle_t https_ota_handle = NULL;
    _NCB_CATCH_ESP_ERR(esp_https_ota_begin(&ota_config, &https_ota_handle), "esp_https_ota_begin");

    esp_err_t err;
    do {
      err = esp_https_ota_perform(https_ota_handle);

      int image_size = esp_https_ota_get_image_size(https_ota_handle);
      if (image_size > 0) {
        int progress = (esp_https_ota_get_image_len_read(https_ota_handle) * 100) / image_size;
        if (_ncb_config_callback)
          _ncb_config_callback(NCB_OTA_PROCESSING, progress);
      }
    } while (err == ESP_ERR_HTTPS_OTA_IN_PROGRESS);

    if (err != ESP_OK) {
      esp_https_ota_abort(https_ota_handle);
      _ncb_config_callback(NCB_OTA_PROCESSING, -1);
      _ncb_esp_err_msg = "esp_https_ota_abort";
      goto esp_failed;
    }

    _NCB_CATCH_ESP_ERR(esp_https_ota_finish(https_ota_handle), "esp_https_ota_finish");

    esp_restart();
  } else {
    _NCB_SEND_ERROR("ignore command format");
  }
  return;

esp_failed:
  _NCB_SEND_ESP_ERR();
}
