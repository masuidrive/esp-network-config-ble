cmake_minimum_required(VERSION 3.5)

idf_component_register(
  REQUIRED_IDF_TARGETS
    "esp32"
    "esp32s3"
    "esp32c3"
  INCLUDE_DIRS
    "include"
  SRC_DIRS
    "src"
  REQUIRES
    "nimble-nordic-uart"
    "json"
    "bt"
    "mqtt"
    "app_update"
    "esp_https_ota"
)
