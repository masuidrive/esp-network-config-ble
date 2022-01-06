# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

idf_component_register(
  REQUIRED_IDF_TARGETS
    "esp32"
    "esp32s3"
    "esp32c3"
  INCLUDE_DIRS
    "src"
  SRCS
    "src/main.c"
    "src/tokenizer.c"
    "src/wifi_util.c"
    "src/mqtt_util.c"
    "src/command_LIST_SSID.c"
    "src/command_SET_WIFI.c"
    "src/command_GET.c"
    "src/command_SET.c"
    "src/command_CHECK_MQTT.c"
    "src/command_CHECK_WIFI.c"
    "src/command_ERASE.c"
    "src/command_DEVICE_ID.c"
  REQUIRES
    bt
    esp-nimble-nordic-uart
    mqtt
  PRIV_REQUIRES
    ${ESP_NIMBLE_PRIV_REQUIRES}
)
project(esp-smartconfig-ble)