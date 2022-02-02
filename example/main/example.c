#include <stdint.h>
#include <stdio.h>

#include "esp-smartconfig-ble.h"
#include "esp_log.h"
#include "nimble-nordic-uart.h"

void command_DEBUG(int argc, const char *args[], int datac, const char *data[]);

static const struct BLECommand BLECommandDEBUG = {.name = "DEBUG", .multiline = true, .func = command_DEBUG};

struct BLECommand *extend_commands[] = {
    &BLECommandDEBUG,
    NULL,
};

void app_main() { smart_config_ble_start(extend_commands, NULL); }

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

/*
// Define the type of LED strip, I have the WS2812b, which has the same timings as the WS2812
#define LED_TYPE LED_STRIP_WS2812

// We connect the data IN of the ledstrip to GPIO 15, on the LyraT board this pin is marked as MTDO by the JTAG headers
#define LED_GPIO 32

// We use RMT to send the data, and the ESP has multiple channels, we select the first one, since we dont use any other
// devices on our RMT channels.
#define LED_CHANNEL RMT_CHANNEL_0

// My LED strip has 256 individual WS2812B RGB LED's.
#define LED_STRIP_LEN 8

static led_strip_t strip = {
    .type = LED_TYPE,
    .length = LED_STRIP_LEN,
    .gpio = LED_GPIO,
    .channel = LED_CHANNEL,
    .buf = NULL,     // We dont use a buffer, this would use extra RAM and we dont need this feature.
    .brightness = 30 // The max brightness is 255, but we set it lower so we can comfortly look at the animation without
                     // burning our eyes out. This also decreases our current usage.
};

int speed = 1000;

void app_main() {
  led_strip_install();

  led_strip_init(&strip);

  int i = 0;
  for (;;) {
    rgb_t colorRGB0 = {.r = 129, .g = 0, .b = 0};
    led_strip_fill(&strip, 0, LED_STRIP_LEN, colorRGB0);

    rgb_t colorRGB = {.r = 0, .g = 0, .b = 32};
    led_strip_set_pixel(&strip, i, colorRGB); // Set that pixel to the color
    led_strip_flush(&strip);
    i = (i + 1) % LED_STRIP_LEN;
    vTaskDelay(((speed / LED_STRIP_LEN) / portTICK_PERIOD_MS));
  }
}
*/