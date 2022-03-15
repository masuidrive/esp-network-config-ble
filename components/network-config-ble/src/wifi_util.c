#include "network-config-ble-internal.h"
static const char *_TAG = "NCB WIFI";

#define _WIFI_CONNECTED_BIT BIT0
#define _WIFI_FAILED_BIT BIT1
static EventGroupHandle_t _wifi_event_group = NULL;

static esp_event_handler_instance_t _instance_any_id = NULL;
static esp_event_handler_instance_t _instance_got_ip = NULL;
static ncb_wifi_status_callback _status_callback = NULL;
static enum ncb_wifi_status _wifi_status = NCB_WIFI_NONE;
static int _max_retry = 0;
static int _retry_num = 0;

static esp_err_t _set_ntp() {
  time_t now = 0;
  struct tm timeinfo = {0};
  int retry = 0;

  time(&now);
  localtime_r(&now, &timeinfo);
  if (timeinfo.tm_year > (2020 - 1900)) {
    return ESP_OK; // already set system time
  }

  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, "pool.ntp.org");
  sntp_init();

  while (timeinfo.tm_year < (2020 - 1900)) {
    ESP_LOGI(_TAG, "Waiting for system time to be set... (%d)", retry + 1);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    time(&now);
    localtime_r(&now, &timeinfo);
    ++retry;
  }
  ESP_LOGI(_TAG, "Current time is %d/%d/%d %d:%d:%d UTC", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1,
           timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

  return ESP_OK;
}

static void _wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT) {
    if (event_id == WIFI_EVENT_STA_START) {
      _retry_num = 0;
      esp_wifi_connect();
    } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
      if ((_wifi_status == NCB_WIFI_CONNECTED || _wifi_status == NCB_WIFI_RECONNECTING) &&
          (_max_retry < 0 || _retry_num < _max_retry)) {
        _wifi_status = NCB_WIFI_RECONNECTING;
        if (_status_callback)
          _status_callback(_wifi_status);
        esp_wifi_connect();
        _retry_num++;
        ESP_LOGI(_TAG, "retry to connect to the AP: %d / %d", _retry_num, _max_retry);
      } else {
        _retry_num = 0;
        _wifi_status = NCB_WIFI_DISCONNECTED;
        if (_status_callback)
          _status_callback(_wifi_status);
        if (_wifi_event_group)
          xEventGroupSetBits(_wifi_event_group, _WIFI_FAILED_BIT);
      }
    }
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(_TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    _retry_num = 0;
    _wifi_status = NCB_WIFI_CONNECTED;
    _set_ntp();

    if (_status_callback)
      _status_callback(_wifi_status);
    if (_wifi_event_group)
      xEventGroupSetBits(_wifi_event_group, _WIFI_CONNECTED_BIT);
  }
}

static esp_err_t _wifi_setup(const char *ssid, const char *password) {
  wifi_config_t wifi_config = {
      .sta =
          {
              .threshold =
                  {
                      .authmode = WIFI_AUTH_WPA2_PSK,
                  },
              .pmf_cfg = {.capable = true, .required = false},
          },
  };
  if (strlen(password) == 0) {
    wifi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;
  }

  strlcpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
  strlcpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

  _NCB_CATCH_ESP_ERR(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config), "esp_wifi_set_config");
  _NCB_CATCH_ESP_ERR(esp_wifi_start(), "esp_wifi_start");

  return ESP_OK;

esp_failed:
  return ESP_FAIL;
}

esp_err_t ncb_wifi_connect(const char *ssid, const char *password, int max_retry,
                           ncb_wifi_status_callback status_callback) {
  _max_retry = max_retry;
  _status_callback = status_callback;
  _retry_num = 0;
  ESP_LOGI(_TAG, "connect SSID: %s, %s", ssid, password);

  _NCB_CATCH_ESP_ERR(ncb_wifi_disconnect(), "esp_wifi_stop");

  if (_status_callback)
    _status_callback(_wifi_status = NCB_WIFI_CONNECTING);

  // set event handler
  _wifi_event_group = xEventGroupCreate();

  // init wifi config
  _NCB_CATCH_ESP_ERR(_wifi_setup(ssid, password), "_wifi_setup");

  // wait wifi_event_handler task
  EventBits_t bits =
      xEventGroupWaitBits(_wifi_event_group, _WIFI_CONNECTED_BIT | _WIFI_FAILED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

  vEventGroupDelete(_wifi_event_group);
  _wifi_event_group = NULL;

  if (bits & _WIFI_CONNECTED_BIT) {
    ESP_LOGI(_TAG, "connected to ap SSID: %s", ssid);
    return ESP_OK;
  } else if (bits & _WIFI_FAILED_BIT) {
    ESP_LOGI(_TAG, "Failed to connect to SSID: %s", ssid);
    _NCB_CATCH_ESP_ERR(esp_wifi_stop(), "esp_wifi_stop");
    return ESP_FAIL;
  } else {
    ESP_LOGE(_TAG, "UNEXPECTED EVENT");
    _NCB_CATCH_ESP_ERR(esp_wifi_stop(), "esp_wifi_stop");
    return ESP_FAIL;
  }

  return ESP_OK;

esp_failed:
  return ESP_FAIL;
}

esp_err_t ncb_wifi_connect_with_nvs(int max_retry, ncb_wifi_status_callback status_callback) {
  nvs_handle_t nvs_handle;
  _NCB_CATCH_ESP_ERR(nvs_open(_NCB_NVS_NAMESPACE, NVS_READONLY, &nvs_handle), "nvs_open");

  size_t ssid_size = MAX_SSID_LEN, passphrase_size = MAX_PASSPHRASE_LEN;
  char ssid[MAX_SSID_LEN], password[MAX_PASSPHRASE_LEN];

  _NCB_CATCH_ESP_ERR(nvs_get_str(nvs_handle, "ssid", ssid, &ssid_size), "nvs_get_str: ssid");
  _NCB_CATCH_ESP_ERR(nvs_get_str(nvs_handle, "password", password, &passphrase_size), "nvs_get_str: password");

  nvs_close(nvs_handle);

  return ncb_wifi_connect(ssid, password, max_retry, status_callback);

esp_failed:
  nvs_close(nvs_handle);
  return ESP_ERR_NOT_FOUND; // NVS config not found
}

esp_err_t ncb_wifi_init() {
  if (_wifi_status != NCB_WIFI_NONE)
    return ESP_OK;
  _NCB_CATCH_ESP_ERR(nvs_flash_init(), "nvs_flash_init");
  _NCB_CATCH_ESP_ERR(esp_netif_init(), "esp_netif_init");
  esp_netif_create_default_wifi_sta();

  _NCB_CATCH_ESP_ERR(
      esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &_wifi_event_handler, NULL, &_instance_any_id),
      "register WIFI_EVENT");
  _NCB_CATCH_ESP_ERR(
      esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &_wifi_event_handler, NULL, &_instance_got_ip),
      "register IP_EVENT");

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  _NCB_CATCH_ESP_ERR(esp_wifi_init(&cfg), "esp_wifi_init");
  _NCB_CATCH_ESP_ERR(esp_wifi_set_storage(WIFI_STORAGE_FLASH), "WIFI_STORAGE_FLASH");
  _NCB_CATCH_ESP_ERR(esp_wifi_set_mode(WIFI_MODE_STA), "WIFI_MODE_STA");
  _wifi_status = NCB_WIFI_INITIALIAED;

  return ESP_OK;

esp_failed:
  return ESP_FAIL;
}

esp_err_t ncb_wifi_disconnect() {
  _wifi_status = NCB_WIFI_STOPPING;

  esp_wifi_stop();

  return ESP_OK;
}

bool ncb_wifi_is_connected() { //
  return _wifi_status == NCB_WIFI_CONNECTED;
}

bool ncb_wifi_is_configured() { //
  nvs_handle_t nvs_handle;
  puts("open?");
  _NCB_CATCH_ESP_ERR(nvs_open(_NCB_NVS_NAMESPACE, NVS_READONLY, &nvs_handle), "nvs_open");
  puts("opened");

  size_t ssid_size;
  _NCB_CATCH_ESP_ERR(nvs_get_str(nvs_handle, "ssid", NULL, &ssid_size), "nvs_get_str: ssid size");
  puts("get");
  nvs_close(nvs_handle);
  printf("ssid=%d\n", ssid_size);

  return ssid_size > 0;

esp_failed:
  nvs_close(nvs_handle);

  return false;
}