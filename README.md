# Network / MQTT configure for ESP-IDF V5

License: Apache 2.0 by Yuichiro MASUI <masui@masuidrive.jp>

## Overview

This library offers a convenient way to configure ESP32's settings via BLE (Bluetooth Low Energy). It simplifies the process of connecting your ESP32 device to a WiFi network.

## Features

- **Easy ESP32 Configuration via BLE**: The primary function of this library is to allow straightforward setup for ESP32 devices using Bluetooth Low Energy.
  
- **NVS Read/Write, WiFi, and MQTT Settings**: The library provides standard functionalities for reading and writing to NVS (Non-Volatile Storage), configuring WiFi and MQTT. However, it is also designed for easy extension of commands if needed. For guidance on how to extend commands, you can refer to the files located at [src/config.c](https://github.com/masuidrive/esp-network-config-ble/blob/main/components/network-config-ble/src/config.c) and [src/command_*.c](https://github.com/masuidrive/esp-network-config-ble/blob/main/components/network-config-ble/src).

- **Line-Oriented Protocol Similar to SMTP**: The underlying protocol on Nordic UART is line-oriented and resembles SMTP, which makes it straightforward to test and experiment with.

- **Manual Testing**: If you wish to manually test the functionalities, you can connect to the device using any [Nordic UART compatible app](https://www.google.com/search?q=nordic-uart+terminal+app). For example, entering `LIST_SSID\r\n` will retrieve a list of accessible SSIDs from the device.

- **Storage in NVS**: All configuration settings are stored in plaintext within the device's Non-Volatile Storage (NVS).

# ESP-IDF Component API Documentation

## Functions
#### `esp_err_t ncb_config_start(const char *device_id, const char *ble_device_name, const char *firmware_version, const char *device_type, const struct ncb_command extend_commands[], size_t extend_commands_count, void (*callback)(enum ncb_callback_type callback_type, int param1))`
Starts the network configuration process over BLE.

- **Parameters**
  - `device_id`: The unique identifier of the device.
  - `ble_device_name`: The BLE device name for the configuration.
  - `firmware_version`: Firmware version of the device.
  - `device_type`: Type of the device.
  - `extend_commands`: Array of extended commands.
  - `extend_commands_count`: Number of extended commands.
  - `callback`: Callback function for various network configuration stages.
- **Returns**
  - `esp_err_t`: Returns ESP_OK on success or an error code on failure.

---

#### `void ncb_config_stop()`
Stops the BLE network configuration process.

- **Description**
  - This function halts the ongoing network configuration process and performs necessary cleanup.

---

### `esp_err_t ncb_wifi_init()`
Initializes the Wi-Fi driver. This function must be called before using any other Wi-Fi API functions.

- **Returns**
  - `esp_err_t`: ESP_OK on success, otherwise an error code.

---

### `bool ncb_wifi_is_configured()`
Checks if Wi-Fi has been configured with SSID and password.

- **Returns**
  - `bool`: True if Wi-Fi is configured, false otherwise.

---

### `esp_err_t ncb_wifi_disconnect()`
Disconnects from the Wi-Fi network and stops the Wi-Fi driver.

- **Returns**
  - `esp_err_t`: ESP_OK on successful disconnection, otherwise an error code.

---

### `esp_err_t ncb_wifi_connect(const char *ssid, const char *password, int max_retry, ncb_wifi_status_callback status_callback)`
Attempts to connect to the specified Wi-Fi network.

- **Parameters**
  - `ssid`: The SSID of the Wi-Fi network.
  - `password`: The password of the Wi-Fi network.
  - `max_retry`: Maximum number of retries for connecting to the network.
  - `status_callback`: Callback function to report Wi-Fi status.
- **Returns**
  - `esp_err_t`: ESP_OK if the connection process is started successfully, otherwise an error code.

---

### `esp_err_t ncb_wifi_connect_with_nvs(int max_retry, ncb_wifi_status_callback status_callback)`
Attempts to connect to a Wi-Fi network using credentials stored in NVS (Non-Volatile Storage).

- **Parameters**
  - `max_retry`: Maximum number of retries for connecting to the network.
  - `status_callback`: Callback function to report Wi-Fi status.
- **Returns**
  - `esp_err_t`: ESP_OK if the connection process is started successfully, otherwise an error code.

---

### `bool ncb_wifi_is_connected()`
Checks if the device is currently connected to a Wi-Fi network.

- **Returns**
  - `bool`: True if connected, false otherwise.

---

### `esp_err_t ncb_mqtt_publish_to_response_topic(const char *message)`
Publishes a message to the configured MQTT response topic.

- **Parameters**
  - `message`: The message to be published.
- **Returns**
  - `esp_err_t`: ESP_OK on successful publication, otherwise an error code.

---

### `esp_err_t ncb_mqtt_connect_with_nvs(ncb_mqtt_message_receiver_callback message_callback, ncb_mqtt_status_callback status_callback)`
Connects to MQTT using credentials stored in NVS.

- **Parameters**
  - `message_callback`: Callback function to handle received MQTT messages.
  - `status_callback`: Callback function to report MQTT status.
- **Returns**
  - `esp_err_t`: ESP_OK if the connection process is started successfully, otherwise an error code.

---

### `esp_err_t ncb_mqtt_disconnect()`
Disconnects from the MQTT broker.

- **Returns**
  - `esp_err_t`: ESP_OK on successful disconnection, otherwise an error code.
---

### Structures

#### `struct ncb_command`
Represents a command for network configuration.

- **Fields**
  - `name`: The name of the command.
  - `multiline`: A boolean indicating if the command can accept multiline input.
  - `func`: Function pointer to the command's implementation, which takes arguments count, arguments array, data count, and data array.

---

## Enumerations

#### `enum ncb_callback_type`
Defines callback types for network configuration processes.

- `NCB_WAIT_CONNECT`: Indicates waiting for a BLE connection.
- `NCB_PROCESSING`: Indicates general processing.
- `NCB_OTA_PROCESSING`: Indicates OTA (Over-The-Air) update processing.

---

### `enum ncb_wifi_status`
Defines the different statuses of the Wi-Fi connection.

- `NCB_WIFI_NONE`: No Wi-Fi status.
- `NCB_WIFI_INITIALIAED`: Wi-Fi is initialized.
- `NCB_WIFI_CONNECTING`: Wi-Fi is connecting.
- `NCB_WIFI_RECONNECTING`: Wi-Fi is reconnecting.
- `NCB_WIFI_CONNECTED`: Wi-Fi is connected.
- `NCB_WIFI_STOPPING`: Wi-Fi is stopping.
- `NCB_WIFI_DISCONNECTED`: Wi-Fi is disconnected.

---

### `enum ncb_mqtt_status`
Defines the different statuses of MQTT connection.

- `MQTT_STARTING`: MQTT is starting.
- `MQTT_CONNECTED`: MQTT is connected.
- `MQTT_DISCONNECTED`: MQTT is disconnected.

---

## Typedefs

### `ncb_mqtt_status_callback`
Function pointer type for MQTT status callback.

- **Parameters**
  - `status`: The current MQTT status.

---

### `ncb_mqtt_message_receiver_callback`
Function pointer type for MQTT message receiver callback.

- **Parameters**
  - `topic`: The topic of the received message.
  - `payload`: The payload of the received message.

## Variable / function naming rules

- Start with ncb\_: public variable / function
  - It's API
- Start with \_ncb\_: private variable / function in this library
  - It only use in the library and this library's unittest
  - ex) \_ncb_hello_world(), \_ncb_params
- Start with \_: file scope variable / function in this library
  - It only use in the file
  - ex) \_counter, \_func1(), \_NCB_CATCH_ESP_ERR()
