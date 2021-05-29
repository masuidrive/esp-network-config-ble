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

static const char *TAG = "DEBUG";

void command_DEBUG(int argc, const char *args[], int datac, const char *data[]) {
  for (int i = 0; i < argc; ++i) {
    nordic_uart_sendln(args[i]);
  }
  nordic_uart_sendln("----");

  for (int i = 0; i < datac; ++i) {
    nordic_uart_sendln(data[i]);
  }
  nordic_uart_sendln("");
}
