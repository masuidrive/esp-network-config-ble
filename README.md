# Network / MQTT configure for ESP-IDF V5

License: Apache 2.0 by Yuichiro MASUI <masui@masuidrive.jp>

This repo included ESP-IDF component and Javascript library for browser.


# ESP-IDF Component API Documentation

## Functions

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

## Enumerations

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
