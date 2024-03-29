#include "network-config-ble-internal.h"
static const char *_TAG = "NCB";

#define _NCB_UART_COMMAND_MAX_ARGC 8

static const struct ncb_command default_commands[] = {
    {.name = "GET_STR", .multiline = false, .func = _ncb_command_GET_STR},
    {.name = "SET_STR", .multiline = false, .func = _ncb_command_SET_STR},
    {.name = "SET_MULTI", .multiline = true, .func = _ncb_command_SET_MULTI},
    {.name = "ERASE_ALL", .multiline = false, .func = _ncb_command_ERASE_ALL},
    {.name = "LIST_SSID", .multiline = false, .func = _ncb_command_LIST_SSID},
    {.name = "SET_WIFI", .multiline = false, .func = _ncb_command_SET_WIFI},
    {.name = "CHECK_WIFI", .multiline = false, .func = _ncb_command_CHECK_WIFI},
    {.name = "CHECK_MQTT", .multiline = false, .func = _ncb_command_CHECK_MQTT},
    {.name = "RESTART", .multiline = false, .func = _ncb_command_RESTART},
    {.name = "OTA_BT", .multiline = false, .func = _ncb_command_OTA_BT},
    {.name = "OTA_HTTPS", .multiline = true, .func = _ncb_command_OTA_HTTPS},
};

static const struct ncb_command *_extend_commands;
static size_t _extend_commands_count = 0;
static TaskHandle_t _ncb_uart_task = NULL;

const char *_ncb_esp_err_msg = NULL;
esp_err_t _ncb_esp_err_code = ESP_OK;

char *_ncb_ble_device_name = NULL;
char *_ncb_firmware_version = NULL;
char *_ncb_device_id = NULL;
void (*_ncb_config_callback)(enum ncb_callback_type, int param) = NULL;

static void _run_command(const struct ncb_command *command, char *item) {
  char *args[_NCB_UART_COMMAND_MAX_ARGC];
  int argc = 0;

  while (item && argc < _NCB_UART_COMMAND_MAX_ARGC) {
    item = _ncb_get_token(item, &args[argc]);
    if (args[argc] == NULL)
      break;
    ++argc;
  }

  if (command->multiline) {
    char *data[CONFIG_NORDIC_UART_MAX_LINE_LENGTH] = {};

    size_t dataline_size;
    int line_count = atoi(args[--argc]);
    args[argc] = NULL;

    if (line_count >= CONFIG_NORDIC_UART_MAX_LINE_LENGTH) {
      _NCB_SEND_ERROR("over lines");
      return;
    }

    bool break_multiline = false;
    for (int i = 0; i < line_count; ++i) {
      char *dataline = (char *)xRingbufferReceive(nordic_uart_rx_buf_handle, &dataline_size, portMAX_DELAY);
      size_t dataline_len = strlen(dataline);
      if (strcmp(dataline, "\003") == 0) { // press Ctrl-C or disconnected
        break_multiline = true;
        break;
      } else {
        size_t size = dataline_len + 1;
        data[i] = malloc(size);
        strlcpy(data[i], dataline, size);
      }

      vRingbufferReturnItem(nordic_uart_rx_buf_handle, (void *)dataline);
    };
    if (!break_multiline)
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

    if (line && strcmp(line, "\003") != 0) {
      ESP_LOGI(_TAG, "LINE: %s", line);

      char *command_name;
      char *item = _ncb_get_token(line, &command_name);
      bool executed = false;

      if (command_name) {
        if (_extend_commands_count > 0) {
          for (int i = 0; i < _extend_commands_count; ++i) {
            if (strcasecmp(command_name, _extend_commands[i].name) == 0) {
              _run_command(&_extend_commands[i], item);
              executed = true;
              break;
            }
          }
        }

        for (int i = 0; i < sizeof(default_commands) / sizeof(struct ncb_command); ++i) {
          if (strcasecmp(command_name, default_commands[i].name) == 0) {
            _run_command(&default_commands[i], item);
            executed = true;
            break;
          }
        }
        if (!executed) {
          _NCB_SEND_ERROR("unknown command");
        }
      }

      vRingbufferReturnItem(nordic_uart_rx_buf_handle, (void *)line);
    }
  }
  vTaskDelete(NULL);
}

static void _nordic_uart_callback(enum nordic_uart_callback_type callback_type) {
  if (_ncb_config_callback) {
    if (callback_type == NORDIC_UART_DISCONNECTED) {
      _ncb_config_callback(NCB_WAIT_CONNECT, 0);
    } else if (callback_type == NORDIC_UART_CONNECTED) {
      _ncb_config_callback(NCB_PROCESSING, 0);
    }
  }
}

static char *alloc_strcpy(const char *str) {
  if (str == NULL)
    return NULL;
  size_t size = strlen(str) + 1;
  char *dest = malloc(size);
  strlcpy(dest, str, size);
  return dest;
}

esp_err_t ncb_config_start(const char *ble_device_name, const char *device_id, const char *firmware_version,
                           const struct ncb_command commands[], size_t commands_count,
                           void (*callback)(enum ncb_callback_type callback_type, int param)) {
  if (_ncb_uart_task)
    return ESP_FAIL;

  _ncb_ble_device_name = alloc_strcpy(ble_device_name);
  _ncb_device_id = alloc_strcpy(device_id);
  _ncb_firmware_version = alloc_strcpy(firmware_version);
  _extend_commands = commands;
  _extend_commands_count = commands_count;
  _ncb_config_callback = callback;
  _NCB_CATCH_ESP_ERR(ncb_wifi_init(), "ncb_wifi_init");
  _NCB_CATCH_ESP_ERR(esp_wifi_start(), "esp_wifi_start");

  if (_ncb_config_callback)
    _ncb_config_callback(NCB_WAIT_CONNECT, 0);

  nordic_uart_start(_ncb_ble_device_name, _nordic_uart_callback);

  xTaskCreate(_uart_incoming_task, "_uart_incoming_task", 2048 * 6, NULL, 1, &_ncb_uart_task);
  return ESP_OK;

esp_failed:
  return ESP_FAIL;
}

void ncb_config_end() {
  vTaskDelete(_ncb_uart_task);
  _ncb_uart_task = NULL;
  free(_ncb_ble_device_name);
  _ncb_ble_device_name = NULL;
  free(_ncb_firmware_version);
  _ncb_firmware_version = NULL;
  free(_ncb_device_id);
  _ncb_device_id = NULL;
  nordic_uart_stop();
}