#include <stdint.h>
#include <stdio.h>

#include "esp-nimble-nordic-uart.h"
#include "esp-smartconfig-ble.h"
#include "esp_log.h"

void app_main(void) {
  puts("Start!");
  smart_config_ble_start();
}
