#pragma once
#include "esp_err.h"
#include <stdbool.h>

struct BLECommand {
  const char *name;
  bool multiline;
  void (*func)(int argc, const char *args[], int datac, const char *data[]);
};

enum smart_config_callback_type {
  SMART_CONFIG_WAIT_BLE,
  SMART_CONFIG_READY_TO_CONFIG,
};

esp_err_t smart_config_ble_start(const struct BLECommand *commands[],
                                 void (*callback)(enum smart_config_callback_type callback_type));