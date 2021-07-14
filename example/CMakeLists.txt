cmake_minimum_required(VERSION 3.5)

set (EXTRA_COMPONENT_DIRS .. ../components/esp-nimble-nordic-uart ../components/esp-aws-iot ../components/led_strip ../components/color ../components/lib8tion ../components/esp_idf_lib_helpers)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(example)
