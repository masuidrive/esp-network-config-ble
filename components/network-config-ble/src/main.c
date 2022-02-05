#include "network-config-ble-internal.h"
static const char *TAG = "SmartConfig";

#define MAX_COMMAND_ARGC 8
#define MAX_DATA_LINES 64

static const struct BLECommand default_commands[] = {
    {.name = "GET_STR", .multiline = false, .func = _ncb_command_GET_STR},
    {.name = "SET_STR", .multiline = false, .func = _ncb_command_SET_STR},
    {.name = "SET_MULTI", .multiline = true, .func = _ncb_command_SET_MULTI},
    {.name = "ERASE", .multiline = false, .func = _ncb_command_ERASE},
    {.name = "LIST_SSID", .multiline = false, .func = _ncb_command_LIST_SSID},
    {.name = "SET_WIFI", .multiline = false, .func = _ncb_command_SET_WIFI},
    {.name = "CHECK_WIFI", .multiline = false, .func = _ncb_command_CHECK_WIFI},
    {.name = "DEVICE_ID", .multiline = false, .func = _ncb_command_DEVICE_ID},
    {.name = "CHECK_MQTT", .multiline = false, .func = _ncb_command_CHECK_MQTT},
};

static struct BLECommand **_original_commands = NULL;
static void (*_smart_config_callback)(enum smart_config_callback_type) = NULL;

static TaskHandle_t _ncb_uart_task = NULL;

const char *_ncb_esp_fail_err_msg = NULL;
esp_err_t _ncb_esp_fail_err_code;

static void _run_command(const struct BLECommand *command, char *item) {
  char *args[MAX_COMMAND_ARGC];
  int argc = 0;

  while (item && argc < MAX_COMMAND_ARGC) {
    item = _ncb_get_token(item, &args[argc]);
    if (args[argc] == NULL)
      break;
    ++argc;
  }

  if (command->multiline) {
    char *data[CONFIG_NORDIC_UART_MAX_LINE_LENGTH];

    size_t dataline_size;
    int line_count = atoi(args[--argc]);
    args[argc] = NULL;

    if (line_count >= CONFIG_NORDIC_UART_MAX_LINE_LENGTH) {
      ESP_LOGE(TAG, "over lines");
      nordic_uart_sendln("ERROR");
      nordic_uart_sendln("");
      return;
    }

    for (int i = 0; i < line_count; ++i) {
      char *dataline = (char *)xRingbufferReceive(nordic_uart_rx_buf_handle, &dataline_size, portMAX_DELAY);
      size_t dataline_len = strlen(dataline);

      data[i] = malloc(dataline_len + 1);
      strcpy(data[i], dataline);

      vRingbufferReturnItem(nordic_uart_rx_buf_handle, (void *)dataline);
    };

    command->func(argc, (const char **)args, line_count, (const char **)data);
    for (int j = 0; j < line_count; ++j)
      free(data[j]);
  } else {
    command->func(argc, (const char **)args, 0, NULL);
  }
}

static void _uart_incoming_task(void *parameter) {
  while (1) {
    size_t item_size;
    char *line = (char *)xRingbufferReceive(nordic_uart_rx_buf_handle, &item_size, portMAX_DELAY);

    if (line) {
      ESP_LOGI(TAG, "LINE: %s", line);

      char *command_name;
      char *item = _ncb_get_token(line, &command_name);

      if (command_name) {
        if (_original_commands) {
          for (struct BLECommand **commands = _original_commands; *commands; ++commands) {
            if (strcasecmp(command_name, (*commands)->name) == 0) {
              _run_command(*commands, item);
              break;
            }
          }
        }

        for (int i = 0; i < sizeof(default_commands) / sizeof(struct BLECommand); ++i) {
          if (strcasecmp(command_name, default_commands[i].name) == 0) {
            _run_command(&default_commands[i], item);
            break;
          }
        }
      }

      vRingbufferReturnItem(nordic_uart_rx_buf_handle, (void *)line);
    }
  }
  vTaskDelete(NULL);
}

static void _nordic_uart_callback(enum nordic_uart_callback_type callback_type) {
  if (_smart_config_callback) {
    if (callback_type == NORDIC_UART_DISCONNECTED) {
      _smart_config_callback(SMART_CONFIG_WAIT_BLE);
    } else if (callback_type == NORDIC_UART_CONNECTED) {
      _smart_config_callback(SMART_CONFIG_READY_TO_CONFIG);
    }
  }
}

esp_err_t ncb_config_start(const char *filename, const struct BLECommand *commands[],
                           void (*callback)(enum smart_config_callback_type)) {
  if (_ncb_uart_task)
    return ESP_FAIL;

  _original_commands = commands;
  _smart_config_callback = callback;
  _CATCH_ESP_FAIL(ncb_wifi_init(), "ncb_wifi_init");
  _CATCH_ESP_FAIL(esp_wifi_start(), "esp_wifi_start");

  if (_smart_config_callback)
    _smart_config_callback(SMART_CONFIG_WAIT_BLE);

  nordic_uart_start(filename, _nordic_uart_callback);

  _ncb_uart_task = xTaskCreate(_uart_incoming_task, "_uart_incoming_task", 2048 * 6, NULL, 1, &_ncb_uart_task);
  return ESP_OK;

esp_failed:
  return ESP_FAIL;
}

esp_err_t ncb_config_end() {
  vTaskDelete(_ncb_uart_task);
  _ncb_uart_task = NULL;
  nordic_uart_stop();
}