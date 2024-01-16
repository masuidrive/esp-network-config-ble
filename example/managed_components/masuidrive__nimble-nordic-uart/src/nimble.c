#include "nimble-nordic-uart.h"

#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "host/ble_hs.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "nvs_flash.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include <freertos/FreeRTOS.h>

static const char *_TAG = "NORDIC UART";

// #define CONFIG_NORDIC_UART_MAX_LINE_LENGTH 256
// #define CONFIG_NORDIC_UART_RX_BUFFER_SIZE 4096
#define BLE_SEND_MTU 128

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define B0(x) ((x) & 0xFF)
#define B1(x) (((x) >> 8) & 0xFF)
#define B2(x) (((x) >> 16) & 0xFF)
#define B3(x) (((x) >> 24) & 0xFF)
#define B4(x) (((x) >> 32) & 0xFF)
#define B5(x) (((x) >> 40) & 0xFF)

// clang-format off
#define UUID128_CONST(a32, b16, c16, d16, e48) \
  BLE_UUID128_INIT( \
    B0(e48), B1(e48), B2(e48), B3(e48), B4(e48), B5(e48), \
    B0(d16), B1(d16), B0(c16), B1(c16), B0(b16), \
    B1(b16), B0(a32), B1(a32), B2(a32), B3(a32), \
  )
// clang-format off

static const ble_uuid128_t SERVICE_UUID = UUID128_CONST(0x6E400001, 0xB5A3, 0xF393, 0xE0A9, 0xE50E24DCCA9E);
static const ble_uuid128_t CHAR_UUID_RX = UUID128_CONST(0x6E400002, 0xB5A3, 0xF393, 0xE0A9, 0xE50E24DCCA9E);
static const ble_uuid128_t CHAR_UUID_TX = UUID128_CONST(0x6E400003, 0xB5A3, 0xF393, 0xE0A9, 0xE50E24DCCA9E);

static uint8_t ble_addr_type;

static uint16_t ble_conn_hdl;
static uint16_t notify_char_attr_hdl;

static void (*_nordic_uart_callback)(enum nordic_uart_callback_type callback_type) = NULL;
static uart_receive_callback_t _uart_receive_callback = NULL;

esp_err_t nordic_uart_yield(uart_receive_callback_t uart_receive_callback) {
  _uart_receive_callback = uart_receive_callback;
  return ESP_OK;
}

static int _uart_receive(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
  if (_uart_receive_callback) {
    _uart_receive_callback(ctxt);
  } else {
    for (int i = 0; i < ctxt->om->om_len; ++i) {
      const char c = ctxt->om->om_data[i];
      _nordic_uart_linebuf_append(c);
    }
  }
  return 0;
}

// notify GATT callback is no operation.
static int _uart_noop(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
  return 0;
}

static const struct ble_gatt_svc_def gat_svcs[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = &SERVICE_UUID.u,
     .characteristics =
         (struct ble_gatt_chr_def[]){
             {.uuid = (ble_uuid_t *)&CHAR_UUID_RX,
              .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
              .access_cb = _uart_receive},
             {.uuid = (ble_uuid_t *)&CHAR_UUID_TX,
              .flags = BLE_GATT_CHR_F_NOTIFY,
              .val_handle = &notify_char_attr_hdl,
              .access_cb = _uart_noop},
             {0},
         }},
    {0}};

static int ble_gap_event_cb(struct ble_gap_event *event, void *arg);

static void ble_app_advertise(void) {
  struct ble_hs_adv_fields fields, fields_ext;
  memset(&fields, 0, sizeof(fields));

  // fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_DISC_LTD;
  fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

  fields.tx_pwr_lvl_is_present = 1;
  fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

  char short_name[6]; // 5 plus zero byte
  const char *name = ble_svc_gap_device_name();
  strncpy(short_name, name, sizeof(short_name));
  short_name[sizeof(short_name) - 1] = '\0';
  fields.name = (uint8_t *)short_name;
  fields.name_len = strlen(short_name);
  if (strlen(name) <= sizeof(short_name) - 1) {
    fields.name_is_complete = 1;
  } else {
    fields.name_is_complete = 0;
  }

  fields.uuids128_is_complete = 1;
  fields.uuids128 = &SERVICE_UUID;
  fields.num_uuids128 = 1;

  int err = ble_gap_adv_set_fields(&fields);
  if (err) {
    ESP_LOGE(_TAG, "ble_gap_adv_set_fields, err %d", err);
  }

  memset(&fields_ext, 0, sizeof(fields_ext));
  fields_ext.flags = fields.flags;
  fields_ext.name = (uint8_t *)name;
  fields_ext.name_len = strlen(name);
  fields_ext.name_is_complete = 1;
  err = ble_gap_adv_rsp_set_fields(&fields_ext);
  if (err) {
    ESP_LOGE(_TAG, "ble_gap_adv_rsp_set_fields fields_ext, name might be too long, err %d", err);
  }

  struct ble_gap_adv_params adv_params;
  memset(&adv_params, 0, sizeof(adv_params));
  adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
  adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

  err = ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event_cb, NULL);
  if (err) {
    ESP_LOGE(_TAG, "Advertising start failed: err %d", err);
  }
}

static int ble_gap_event_cb(struct ble_gap_event *event, void *arg) {
  switch (event->type) {
  case BLE_GAP_EVENT_CONNECT:
    ESP_LOGI(_TAG, "BLE_GAP_EVENT_CONNECT %s", event->connect.status == 0 ? "OK" : "Failed");
    if (event->connect.status == 0) {
      ble_conn_hdl = event->connect.conn_handle;
      if (_nordic_uart_callback)
        _nordic_uart_callback(NORDIC_UART_CONNECTED);
    } else {
      ble_app_advertise();
    }
    break;
  case BLE_GAP_EVENT_DISCONNECT:
    _nordic_uart_linebuf_append('\003'); // send Ctrl-C
    ESP_LOGI(_TAG, "BLE_GAP_EVENT_DISCONNECT");
    if (_nordic_uart_callback)
      _nordic_uart_callback(NORDIC_UART_DISCONNECTED);
    ble_app_advertise();
    break;
  case BLE_GAP_EVENT_ADV_COMPLETE:
    ESP_LOGI(_TAG, "BLE_GAP_EVENT_ADV_COMPLETE");
    ble_app_advertise();
    break;
  case BLE_GAP_EVENT_SUBSCRIBE:
    ESP_LOGI(_TAG, "BLE_GAP_EVENT_SUBSCRIBE");
    break;
  default:
    break;
  }
  return 0;
}

static void ble_app_on_sync_cb(void) {
  int ret = ble_hs_id_infer_auto(0, &ble_addr_type);
  if (ret != 0) {
    ESP_LOGE(_TAG, "Error ble_hs_id_infer_auto: %d", ret);
  }
  ble_app_advertise();
}

// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/nimble/index.html#_CPPv434esp_nimble_hci_and_controller_initv
static void ble_host_task(void *param) {
  nimble_port_run(); // This function will return only when nimble_port_stop() is executed.
  nimble_port_freertos_deinit();
  _nordic_uart_buf_deinit();
}

// Split the message in BLE_SEND_MTU and send it.
esp_err_t _nordic_uart_send(const char *message) {
  const int len = strlen(message);
  if (len == 0)
    return ESP_OK;
  // Split the message in BLE_SEND_MTU and send it.
  for (int i = 0; i < len; i += BLE_SEND_MTU) {
    int err;
    struct os_mbuf *om;
    int err_count = 0;
  do_notify:
    om = ble_hs_mbuf_from_flat(&message[i], MIN(BLE_SEND_MTU, len - i));
    err = ble_gattc_notify_custom(ble_conn_hdl, notify_char_attr_hdl, om);
    if (err == BLE_HS_ENOMEM && err_count++ < 10) {
      vTaskDelay(100 / portTICK_PERIOD_MS);
      goto do_notify;
    }
    if (err)
      return ESP_FAIL;
  }
  return ESP_OK;
}

/***
 *
 * Note:
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/nimble/index.html
 */
esp_err_t _nordic_uart_start(const char *device_name, void (*callback)(enum nordic_uart_callback_type callback_type)) {
  // already initialized will return ESP_FAIL
  if (_nordic_uart_linebuf_initialized()) {
    ESP_LOGE(_TAG, "Already initialized");
    return ESP_FAIL;
  }

  if (nvs_flash_init() != ESP_OK) {
    ESP_LOGE(_TAG, "Failed to nvs_flash_init");
    return ESP_FAIL;
  }

  _nordic_uart_callback = callback;
  _nordic_uart_buf_init();

  // Initialize NimBLE
  esp_err_t ret = nimble_port_init();
  if (ret != ESP_OK) {
    ESP_LOGE(_TAG, "nimble_port_init() failed with error: %d", ret);
  }
  
  // Initialize the NimBLE Host configuration
  // Bluetooth device name for advertisement
  ble_svc_gap_device_name_set(device_name);
  ble_svc_gap_init();
  ble_svc_gatt_init();

  ble_gatts_count_cfg(gat_svcs);
  ble_gatts_add_svcs(gat_svcs);

  ble_hs_cfg.sync_cb = ble_app_on_sync_cb;

  // Create NimBLE thread
  nimble_port_freertos_init(ble_host_task);

  return ESP_OK;
}

esp_err_t _nordic_uart_stop(void) {
  esp_err_t rc = ble_gap_adv_stop();
  if (rc) {
    // if already stoped BLE, some error is raised. but no problem.
    ESP_LOGD(_TAG, "Error in stopping advertisement with err code = %d", rc);
    return ESP_FAIL;

  }

  int ret = nimble_port_stop();
  if (ret == ESP_OK) {
    ret = nimble_port_deinit();
    if (ret != ESP_OK) {
      ESP_LOGE(_TAG, "nimble_port_deinit() failed with error: %d", ret);
      return ESP_FAIL;
    }
  }
  _nordic_uart_buf_deinit();

  _nordic_uart_callback = NULL;
  
  return ESP_OK;
}
