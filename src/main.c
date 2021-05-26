#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esp-nimble-nordic-uart.h"
#include "esp_log.h"
#include "freertos/ringbuf.h"
#include "freertos/task.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define is_skip_char(chr) (chr == '\r')
#define is_escape_char(chr) (chr == '\\')
#define is_quote_char(chr) (chr == '"')
#define is_end_of_string(chr) (chr == '\0')
#define is_separator(chr) (chr == ' ' || chr == '\n')
#define is_separator_or_null(chr) (is_separator(chr) || is_end_of_string(chr))

char *get_token(char *text, char **token) {
  // skip head spaces
  while (is_separator(*text))
    ++text;

  // empty string
  if (is_end_of_string(*text)) {
    printf("end\n");
    *token = NULL;
    return NULL;
  }

  char *cur;
  if (is_quote_char(*text)) { // quoted
    cur = ++text;             // skip quote char
    *token = text;
    bool escape_char = false;
    while (!((is_quote_char(*text) && !escape_char) || is_end_of_string(*text))) {
      if (is_quote_char(*text) && escape_char) {
        *(cur - 1) = *text++;
        escape_char = false;
      } else {
        escape_char = is_escape_char(*text);
        if (!is_skip_char(*text)) {
          *cur++ = *text;
        }
        ++text;
      }
    }
  } else {
    cur = text;
    *token = text;
    while (!is_separator_or_null(*text)) {
      if (!is_skip_char(*text)) {
        *cur++ = *text;
      }
      ++text;
    }
  }
  int end_of_text = is_end_of_string(*text);
  *cur++ = '\0';

  return end_of_text ? NULL : text + 1;
}

int main() {
  char str[] = "  Hello \" World \" ";
  char *token;
  char *result = get_token(str, &token, tok_string);
  printf("t=[%s], next p=[%s]\n", token, result);
  result = get_token(result, &token, tok_string);
  printf("t=[%s], next p=[%p]\n", token, result);

  return 0;
}

static void notifyTask(void *parameter) {
  static char mbuf[CONFIG_NORDIC_UART_MAX_LINE_LENGTH + 1];

  for (;;) {
    size_t item_size;
    const char *item = (char *)xRingbufferReceive(nordic_uart_rx_buf_handle, &item_size, portMAX_DELAY);

    if (item) {
      char command[COMMAND_LENGTH] = "";
      int i;
      for (i = 0; i < item_size; ++i) {
        if (item[i] >= 'a' && item[i] <= 'z')
          mbuf[i] = item[i] - 0x20;
        else
          mbuf[i] = item[i];
      }
      mbuf[item_size] = '\0';

      nordic_uart_sendln(mbuf);
      vRingbufferReturnItem(nordic_uart_rx_buf_handle, (void *)item);
    }
  }

  vTaskDelete(NULL);
}

void app_main(void) {
  nordic_uart_start();
  xTaskCreate(notifyTask, "notifyTask", 4196, NULL, 1, NULL);
}
