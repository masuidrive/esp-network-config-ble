# Network / MQTT configu4e for ESP-IDF V4.2 and later

License: Apache by Yuichiro MASUI <masui@masuidrive.jp>

# Variable / function naming rules

- Start with ncb\_: public variable / function
  - It's API
- Start with \_ncb\_: private variable / function in this library
  - It only use in the library and this library's unittest
  - ex) \_ncb_hello_world(), \_ncb_params
- Start with \_: file scope variable / function in this library
  - It only use in the file
  - ex) \_counter, \_func1(), \_CATCH_ESP_FAIL()
