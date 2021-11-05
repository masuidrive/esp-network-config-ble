#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "wifi_util.h"

static const char *TAG = "WIFI";

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAILED_BIT BIT1

static EventGroupHandle_t s_wifi_event_group = NULL;
static esp_event_handler_instance_t instance_any_id;
static esp_event_handler_instance_t instance_got_ip;
static wifi_status_callback status_callback = NULL;
static int max_retry = -1;
static int s_retry_num = 0;

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT) {
    if (event_id == WIFI_EVENT_STA_START) {
      esp_wifi_connect();
    } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
      if (s_retry_num < max_retry || max_retry < 0) {
        if (status_callback)
          status_callback(WIFI_RECONNECTING);
        esp_wifi_connect();
        s_retry_num++;
        ESP_LOGI(TAG, "retry to connect to the AP: %d / %d", s_retry_num, max_retry);
      } else {
        if (status_callback)
          status_callback(WIFI_DISCONNECTED);
        if (s_wifi_event_group)
          xEventGroupSetBits(s_wifi_event_group, WIFI_FAILED_BIT);
      }
      ESP_LOGI(TAG, "connect to the AP fail");
    }
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    s_retry_num = 0;
    if (status_callback)
      status_callback(WIFI_CONNECTED);
    if (s_wifi_event_group)
      xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}

static void wifi_setup(const char *ssid, const char *password) {
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

  strlcpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
  strlcpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
}

esp_err_t wifi_connect(const char *ssid, const char *password, int _max_retry, wifi_status_callback _status_callback) {
  max_retry = _max_retry;
  status_callback = _status_callback;
  s_retry_num = 0;

  esp_wifi_stop();

  if (status_callback)
    status_callback(WIFI_CONNECTING);

  // set event handler
  s_wifi_event_group = xEventGroupCreate();

  // init wifi config
  wifi_setup(ssid, password);

  // wait wifi_event_handler task
  EventBits_t bits =
      xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAILED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

  vEventGroupDelete(s_wifi_event_group);
  s_wifi_event_group = NULL;

  if (bits & WIFI_CONNECTED_BIT) {
    ESP_LOGI(TAG, "connected to ap SSID: %s", ssid);
    return ESP_OK;
  } else if (bits & WIFI_FAILED_BIT) {
    ESP_LOGI(TAG, "Failed to connect to SSID: %s", ssid);
    esp_wifi_stop();
    return ESP_FAIL;
  } else {
    ESP_LOGE(TAG, "UNEXPECTED EVENT");
    esp_wifi_stop();
    return ESP_FAIL;
  }
}

esp_err_t wifi_connect_with_nvs(int max_retry, wifi_status_callback status_callback) {
  nvs_handle_t nvs_handle;
  ESP_ERROR_CHECK(nvs_open("wifi", NVS_READONLY, &nvs_handle));

  size_t ssid_size = MAX_SSID_LEN, passphrase_size = MAX_PASSPHRASE_LEN;
  char ssid[MAX_SSID_LEN], password[MAX_PASSPHRASE_LEN];
  ESP_ERROR_CHECK(nvs_get_str(nvs_handle, "ssid", ssid, &ssid_size));
  ESP_ERROR_CHECK(nvs_get_str(nvs_handle, "password", password, &passphrase_size));
  nvs_close(nvs_handle);

  return wifi_connect(ssid, password, max_retry, status_callback);
}

esp_err_t wifi_init() {
  ESP_ERROR_CHECK(esp_netif_init());
  esp_netif_create_default_wifi_sta();

  ESP_ERROR_CHECK(
      esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));
  ESP_ERROR_CHECK(
      esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_got_ip));

  return ESP_OK;
}