# Network / MQTT configure for ESP-IDF

[![Component Registry](https://components.espressif.com/components/masuidrive/network-config-ble/badge.svg)](https://components.espressif.com/components/masuidrive/network-config-ble)

License: Apache 2.0 by Yuichiro MASUI <masui@masuidrive.jp>

## Overview

This library offers a convenient way to configure ESP32's settings via BLE (Bluetooth Low Energy). It simplifies the process of connecting your ESP32 device to a WiFi network.

## Features

- **Easy ESP32 Configuration via BLE**: The primary function of this library is to allow straightforward setup for ESP32 devices using Bluetooth Low Energy.
- **NVS Read/Write, WiFi, and MQTT Settings**: The library provides standard functionalities for reading and writing to NVS (Non-Volatile Storage), configuring WiFi and MQTT. However, it is also designed for easy extension of commands if needed. For guidance on how to extend commands, you can refer to the files located at [src/config.c](https://github.com/masuidrive/esp-network-config-ble/blob/main/components/network-config-ble/src/config.c) and [src/command\_\*.c](https://github.com/masuidrive/esp-network-config-ble/blob/main/components/network-config-ble/src).
