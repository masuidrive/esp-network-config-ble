#pragma once
#include <stdbool.h>

struct BLECommand {
  const char *name;
  bool multiline;
  void (*func)(int argc, const char *args[], int datac, const char *data[]);
};

void smart_config_ble_start(struct BLECommand **commands);