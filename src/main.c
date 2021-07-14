#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esp-nimble-nordic-uart.h"
#include "esp-smartconfig-ble.h"
#include "esp32/himem.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/ringbuf.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "tokenizer.h"

static const char *TAG = "SmartConfig";

#define MAX_COMMAND_ARGC 8
#define MAX_DATA_LINES 64

void command_LIST_SSID(int argc, const char *args[], int datac, const char *data[]);
void command_SET_WIFI(int argc, const char *args[], int datac, const char *data[]);
void command_SET_STR(int argc, const char *args[], int datac, const char *data[]);
void command_SET_MULTI(int argc, const char *args[], int datac, const char *data[]);
void command_GET_STR(int argc, const char *args[], int datac, const char *data[]);
void command_CHECK_AWSIOT(int argc, const char *args[], int datac, const char *data[]);
void command_CHECK_WIFI(int argc, const char *args[], int datac, const char *data[]);

const struct BLECommand default_commands[] = {
    {.name = "LIST_SSID", .multiline = false, .func = command_LIST_SSID},
    {.name = "SET_WIFI", .multiline = false, .func = command_SET_WIFI},
    {.name = "GET_STR", .multiline = false, .func = command_GET_STR},
    {.name = "SET_STR", .multiline = false, .func = command_SET_STR},
    {.name = "SET_MULTI", .multiline = true, .func = command_SET_MULTI},
    {.name = "CHECK_WIFI", .multiline = false, .func = command_CHECK_WIFI},
    {.name = "CHECK_AWSIOT", .multiline = false, .func = command_CHECK_AWSIOT},
};

struct BLECommand **original_commands = NULL;

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

static void run_command(const struct BLECommand *command, char *item) {
  char *args[MAX_COMMAND_ARGC];
  int argc = 0;

  while (item && argc < MAX_COMMAND_ARGC) {
    item = get_token(item, &args[argc]);
    if (args[argc] == NULL)
      break;
    ++argc;
  }

  if (command->multiline) {
    int datac = 0;
    char *data[CONFIG_NORDIC_UART_MAX_LINE_LENGTH];

    while (true) {
      size_t dataline_size;
      char *dataline = (char *)xRingbufferReceive(nordic_uart_rx_buf_handle, &dataline_size, portMAX_DELAY);
      size_t dataline_len = strlen(dataline);
      ESP_LOGI(TAG, "ML: %d, %d", dataline_len, dataline_size);
      if (dataline_size <= 1)
        break;
      if (datac >= CONFIG_NORDIC_UART_MAX_LINE_LENGTH - 1) {
        ESP_LOGE(TAG, "over lines");
      } else {
        data[datac] = malloc(dataline_size);
        strcpy(data[datac++], dataline);
      }
      vRingbufferReturnItem(nordic_uart_rx_buf_handle, (void *)dataline);
    };

    command->func(argc, (const char **)args, datac, (const char **)data);
    for (int j = 0; j < datac; ++j)
      free(data[j]);
  } else {
    command->func(argc, (const char **)args, 0, NULL);
  }
}

static void uartIncomingTask(void *parameter) {
  for (;;) {
    size_t item_size;
    char *line = (char *)xRingbufferReceive(nordic_uart_rx_buf_handle, &item_size, portMAX_DELAY);

    if (line) {
      ESP_LOGI(TAG, "LINE: %s", line);

      char *command_name;
      char *item = get_token(line, &command_name);

      if (command_name) {
        if (original_commands) {
          struct BLECommand **commands = original_commands;
          while (*commands) {
            if (strcasecmp(command_name, (*commands)->name) == 0) {
              run_command(*commands, item);
              break;
            }
          }
        }

        for (int i = 0; i < sizeof(default_commands) / sizeof(struct BLECommand); ++i) {
          if (strcasecmp(command_name, default_commands[i].name) == 0) {
            run_command(&default_commands[i], item);
            break;
          }
        }
      }

      vRingbufferReturnItem(nordic_uart_rx_buf_handle, (void *)line);
    }
  }
  vTaskDelete(NULL);
}

void smart_config_ble_start(struct BLECommand *commands[]) {
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  original_commands = commands;

  initialisze_wifi();
  nordic_uart_start();
  xTaskCreate(uartIncomingTask, "uartIncomingTask", 8192 * 2, NULL, 1, NULL);
}
