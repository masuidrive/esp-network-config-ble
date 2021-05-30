cmake_minimum_required(VERSION 3.5)

set (EXTRA_COMPONENT_DIRS .. ../components/esp-nimble-nordic-uart ../components/esp-aws-iot)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(example)
