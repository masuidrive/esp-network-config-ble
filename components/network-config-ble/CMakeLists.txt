# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
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
    bt
    mqtt
    nimble-nordic-uart
)
