#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esp-nimble-nordic-uart.h"
#include "esp-smartconfig-ble.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

static const char *TAG = "SETWIFI";

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAILED_BIT BIT1
#define RETRY_COUNT 3
#define SSID_LENGTH 32
#define PASSWORD_LENGTH 64

static EventGroupHandle_t s_wifi_event_group;
static esp_event_handler_instance_t instance_any_id;
static esp_event_handler_instance_t instance_got_ip;

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  static int s_retry_num = 0;
  if (event_base == WIFI_EVENT) {
    if (event_id == WIFI_EVENT_STA_START) {
      esp_wifi_connect();
    } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
      if (s_retry_num < RETRY_COUNT) {
        esp_wifi_connect();
        s_retry_num++;
        ESP_LOGI(TAG, "retry to connect to the AP");
      } else {
        xEventGroupSetBits(s_wifi_event_group, WIFI_FAILED_BIT);
      }
      ESP_LOGI(TAG, "connect to the AP fail");
    }
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    s_retry_num = 0;
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}

void command_CHECK_AWS(int argc, const char *args[], int datac, const char *data[]) {
  if (argc < 1) {
    nordic_uart_sendln("ERROR \"SSID is blank\"");
    nordic_uart_sendln("");
    return;
  }

  esp_event_loop_create_default();
  esp_wifi_stop();

  s_wifi_event_group = xEventGroupCreate();
  ESP_ERROR_CHECK(
      esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));
  ESP_ERROR_CHECK(
      esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_got_ip));

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

  strlcpy((char *)wifi_config.sta.ssid, args[0], sizeof(wifi_config.sta.ssid));
  strlcpy((char *)wifi_config.sta.password, args[1], sizeof(wifi_config.sta.password));
  puts((char *)wifi_config.sta.password);
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  EventBits_t bits =
      xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAILED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

  ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));

  vEventGroupDelete(s_wifi_event_group);
  s_wifi_event_group = NULL;

  if (bits & WIFI_CONNECTED_BIT) {
    ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", wifi_config.sta.ssid, wifi_config.sta.password);
    nordic_uart_sendln("OK");
  } else if (bits & WIFI_FAILED_BIT) {
    ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", wifi_config.sta.ssid, wifi_config.sta.password);
    nordic_uart_sendln("ERROR: Failed to connect");
  } else {
    ESP_LOGE(TAG, "UNEXPECTED EVENT");
    nordic_uart_sendln("ERROR: Unexpected event");
  }

  nordic_uart_sendln("");
  ESP_LOGI(TAG, "Fin.");
}
