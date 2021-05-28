#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esp-nimble-nordic-uart.h"
#include "esp-smartconfig-ble.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/ringbuf.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "tokenizer.h"

void command_SSIDLIST(const char *args[], const char *data);

struct BLECommand {
  const char *name;
  bool multiline;
  void (*func)(const char *args[], const char *data);
};

const struct BLECommand commands[] = {
    {.name = "SSIDLIST", .multiline = false, .func = command_SSIDLIST},
};

static void initialisze_wifi(void) {
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());
}

static void uartIncomingTask(void *parameter) {
  for (;;) {
    size_t item_size;
    char *line = (char *)xRingbufferReceive(nordic_uart_rx_buf_handle, &item_size, portMAX_DELAY);

    if (line) {
      char *command_name;
      char *item = get_token(line, &command_name);

      if (!command_name)
        break;

      for (int i = 0; i < sizeof(commands) / sizeof(struct BLECommand); ++i) {
        if (strcasecmp(command_name, commands[i].name) == 0) {
          commands[i].func(NULL, NULL);
          break;
        }
      }

      vRingbufferReturnItem(nordic_uart_rx_buf_handle, (void *)line);
    }
  }

  vTaskDelete(NULL);
}

void smart_config_ble_start(void) {
  initialisze_wifi();
  nordic_uart_start();
  xTaskCreate(uartIncomingTask, "uartIncomingTask", 8192, NULL, 1, NULL);
}
