#include "network-config-ble-internal.h"
static const char *_TAG = "MQTT_UTIL";

static ncb_mqtt_message_receiver_callback _message_callback = NULL;
static ncb_mqtt_status_callback _status_callback = NULL;
static esp_mqtt_client_handle_t _client;
static char _topic[256];

typedef enum QoS { QOS0 = 0, QOS1 = 1 } QoS;

static const char *_get_nvs_value(nvs_handle_t nvs_handle, const char *name) {
  size_t required_size = 0;
  char *value = NULL;
  _NCB_CATCH_ESP_ERR(nvs_get_str(nvs_handle, name, NULL, &required_size), "nvs_get_str");
  value = malloc(required_size);
  _NCB_CATCH_ESP_ERR(nvs_get_str(nvs_handle, name, value, &required_size), "nvs_get_str");
  return value;

esp_failed:
  free(value);
  return NULL;
}

static void _log_error_if_nonzero(const char *message, int error_code) {
  if (error_code != 0) {
    ESP_LOGE(_TAG, "Last error %s: 0x%x", message, error_code);
  }
}

static void _mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  ESP_LOGD(_TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;

  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(_TAG, "MQTT_EVENT_CONNECTED");
    if (esp_mqtt_client_subscribe(client, _topic, 1) < 0) {
    }
    break;

  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(_TAG, "MQTT_EVENT_DISCONNECTED");
    if (_status_callback)
      _status_callback(MQTT_DISCONNECTED);
    break;

  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI(_TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
    if (_status_callback)
      _status_callback(MQTT_CONNECTED);
    break;

  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(_TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    break;

  case MQTT_EVENT_PUBLISHED:
    // ESP_LOGI(_TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    break;

  case MQTT_EVENT_DATA:
    if (_message_callback)
      _message_callback(event->topic, (char *)event->data);
    break;

  case MQTT_EVENT_ERROR:
    ESP_LOGI(_TAG, "MQTT_EVENT_ERROR");
    if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
      _log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
      _log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
      _log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
      ESP_LOGI(_TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
    }
    break;

  default:
    ESP_LOGI(_TAG, "Other event id:%d", event->event_id);
    break;
  }
}

esp_err_t ncb_mqtt_connect_with_nvs(ncb_mqtt_message_receiver_callback message_callback,
                                    ncb_mqtt_status_callback status_callback) {
  _message_callback = message_callback;
  _status_callback = status_callback;
  if (_status_callback)
    status_callback(MQTT_STARTING);

  nvs_handle_t nvs_handle;
  ESP_ERROR_CHECK(nvs_open(_NCB_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle));

  const esp_mqtt_client_config_t mqtt_cfg = {
      .uri = (const char *)_get_nvs_value(nvs_handle, "mqtt_uri"),
      .client_cert_pem = (const char *)_get_nvs_value(nvs_handle, "mqtt_cert"),
      .client_key_pem = (const char *)_get_nvs_value(nvs_handle, "mqtt_priv"),
      .cert_pem = (const char *)_get_nvs_value(nvs_handle, "mqtt_root_ca"),
  };

  strlcpy(_topic, (const char *)_get_nvs_value(nvs_handle, "mqtt_topic"), sizeof(_topic));
  nvs_close(nvs_handle);
  _client = esp_mqtt_client_init(&mqtt_cfg);

  /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
  esp_mqtt_client_register_event(_client, ESP_EVENT_ANY_ID, _mqtt_event_handler, NULL);
  esp_mqtt_client_start(_client);

  return ESP_OK;
}

esp_err_t ncb_mqtt_publish(const char *message) {
  esp_mqtt_client_publish(_client, _topic, message, 0, QOS1, 0);
  return ESP_OK;
}

esp_err_t ncb_mqtt_disconnect() {
  if (esp_mqtt_client_stop(_client) == ESP_OK) {
    _client = NULL;
    return ESP_OK;
  } else {
    _client = NULL;
    return ESP_FAIL;
  }
}