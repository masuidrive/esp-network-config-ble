#include "esp_log.h"
#include "network-config-ble.h"
#include "nimble-nordic-uart.h"

#include <esp_event.h>
#include <stdint.h>
#include <stdio.h>

void command_DEBUG(int argc, const char *args[], int datac, const char *data[]);

static const struct ncb_command ncb_commandDEBUG = { //
    .name = "DEBUG",
    .multiline = true,
    .func = command_DEBUG};

struct ncb_command *extend_commands[] = {
    &ncb_commandDEBUG,
    NULL,
};

void app_main() { //
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ncb_config_start("Nordic UART", extend_commands, NULL);
}

void command_DEBUG(int argc, const char *args[], int datac, const char *data[]) {
  char buf[CONFIG_NORDIC_UART_MAX_LINE_LENGTH + 1];
  sprintf(buf, "argc=%d, datac=%d", argc, datac);
  puts(buf);
  nordic_uart_sendln(buf);

  for (int i = 0; i < argc; ++i) {
    nordic_uart_sendln(args[i]);
  }
  nordic_uart_sendln("----");

  for (int i = 0; i < datac; ++i) {
    nordic_uart_sendln(data[i]);
  }
  nordic_uart_sendln("");
}
