#include "esp-smartconfig-ble-internal.h"
static const char *TAG = "SmartConfig";

#define MAX_COMMAND_ARGC 8
#define MAX_DATA_LINES 64

void command_LIST_SSID(int argc, const char *args[], int datac, const char *data[]);
void command_SET_WIFI(int argc, const char *args[], int datac, const char *data[]);
void command_SET_STR(int argc, const char *args[], int datac, const char *data[]);
void command_SET_MULTI(int argc, const char *args[], int datac, const char *data[]);
void command_GET_STR(int argc, const char *args[], int datac, const char *data[]);
void command_CHECK_MQTT(int argc, const char *args[], int datac, const char *data[]);
void command_CHECK_WIFI(int argc, const char *args[], int datac, const char *data[]);
void command_ERASE(int argc, const char *args[], int datac, const char *data[]);

const struct BLECommand default_commands[] = {
    {.name = "GET_STR", .multiline = false, .func = command_GET_STR},
    {.name = "SET_STR", .multiline = false, .func = command_SET_STR},
    {.name = "SET_MULTI", .multiline = true, .func = command_SET_MULTI},
    {.name = "ERASE", .multiline = false, .func = command_ERASE},
    {.name = "LIST_SSID", .multiline = false, .func = command_LIST_SSID},
    {.name = "SET_WIFI", .multiline = false, .func = command_SET_WIFI},
    {.name = "CHECK_WIFI", .multiline = false, .func = command_CHECK_WIFI},
    {.name = "CHECK_MQTT", .multiline = false, .func = command_CHECK_MQTT},
};

static struct BLECommand **original_commands = NULL;
static void (*smart_config_callback)(enum smart_config_callback_type) = NULL;

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
          for (struct BLECommand **commands = original_commands; *commands; ++commands) {
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

static void nordic_uart_callback(enum nordic_uart_callback_type callback_type) {
  if (callback_type == NORDIC_UART_DISCONNECTED) {
    smart_config_callback(SMART_CONFIG_WAIT_BLE);
  } else if (callback_type == NORDIC_UART_CONNECTED) {
    smart_config_callback(SMART_CONFIG_READY_TO_CONFIG);
  }
}

esp_err_t smart_config_ble_start(struct BLECommand *commands[], void (*callback)(enum smart_config_callback_type)) {
  original_commands = commands;
  smart_config_callback = callback;
  CATCH_ESP_FAIL(esp_wifi_start());

  if (smart_config_callback)
    smart_config_callback(SMART_CONFIG_WAIT_BLE);

  nordic_uart_start(nordic_uart_callback);
  xTaskCreate(uartIncomingTask, "uartIncomingTask", 8192 * 2, NULL, 1, NULL);
  return ESP_OK;

esp_failed:
  return ESP_FAIL;
}
