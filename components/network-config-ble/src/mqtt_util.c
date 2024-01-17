#include "network-config-ble-internal.h"
static const char *_TAG = "MQTT_UTIL";

static ncb_mqtt_message_receiver_callback _message_callback = NULL;
static ncb_mqtt_status_callback _status_callback = NULL;
static esp_mqtt_client_handle_t _client;
static char _topic[256] = "";
static char _response_topic[256] = "";
static bool _clear_retained_message = true;

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

static bool strlcpy_nvs_value(char *dest, nvs_handle_t nvs_handle, const char *name, size_t size) {
  const char *val = _get_nvs_value(nvs_handle, name);
  if (val) {
    strlcpy(dest, val, size - 1);
    free((void *)val);
    return true;
  } else {
    strlcpy(dest, "", size - 1);
    free((void *)val);
    return false;
  }
}

static void _log_error_if_nonzero(const char *message, int error_code) {
  if (error_code != 0) {
    ESP_LOGE(_TAG, "Last error %s: 0x%x", message, error_code);
  }
}

esp_err_t _send_clear_retained_message() {
  if (strlen(_topic) > 0) {
    esp_mqtt_client_publish(_client, _topic, "", 0, QOS1, true);
    return ESP_OK;
  } else {
    return ESP_FAIL;
  }
}

static void _mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  ESP_LOGD(_TAG, "Event dispatched from event loop base=%s, event_id=%ld", base, event_id);
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;

  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(_TAG, "MQTT_EVENT_CONNECTED");
    if (esp_mqtt_client_subscribe(client, _topic, 1) < 0) {
      // TODO: need it?
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
    ESP_LOGI(_TAG, "MQTT_EVENT_DATA, msg_id=%d, %d", event->msg_id, event->data_len);

    if (_message_callback && event->data_len) {
      char *data = malloc(event->data_len + 1);
      memcpy(data, event->data, event->data_len);
      data[event->data_len] = '\0';

      _message_callback(event->topic, data);

      if (strlen(_response_topic) > 0) {
        const int resp_size = 256;

        cJSON *root = cJSON_Parse((char *)event->data);
        if (root && root->type == cJSON_Object) {
          char *response = (char *)malloc(resp_size);
          cJSON *uuid = cJSON_GetObjectItem(root, "uuid");

          if (uuid && uuid->type == cJSON_String) {
            strlcpy(response, "{\"event\":\"receive_message\",\"uuid\":\"", resp_size - 1);
            strlcat(response, uuid->valuestring, resp_size - strlen(uuid->valuestring) - 1);
            strlcat(response, "\"}", resp_size - 1);
          } else {
            strlcpy(response, "{\"event\":\"receive_message\"}", resp_size - 1);
          }
          cJSON_Delete(root);

          ncb_mqtt_publish_to_response_topic(response);

          free(response);
        }
      }
      free((void *)data);
    }

    if (event->data_len && _clear_retained_message)
      _send_clear_retained_message();
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
      .broker = {.address =
                     {
                         .uri = (const char *)_get_nvs_value(nvs_handle, "mqtt_uri"),
                     },
                 .verification =
                     {
                         .certificate = (const char *)_get_nvs_value(nvs_handle, "mqtt_root_ca"),
                     }},
      .credentials = {.authentication = {
                          .certificate = (const char *)_get_nvs_value(nvs_handle, "mqtt_cert"),
                          .key = (const char *)_get_nvs_value(nvs_handle, "mqtt_priv"),
                      }}};

  if (mqtt_cfg.broker.address.uri == NULL || strlen(mqtt_cfg.broker.address.uri) == 0 ||
      mqtt_cfg.credentials.authentication.certificate == NULL ||
      strlen(mqtt_cfg.credentials.authentication.certificate) == 0 || mqtt_cfg.credentials.authentication.key == NULL ||
      strlen(mqtt_cfg.credentials.authentication.key) == 0 || mqtt_cfg.broker.verification.certificate == NULL ||
      strlen(mqtt_cfg.broker.verification.certificate) == 0) {
    return ESP_ERR_INVALID_ARG;
  }

  _client = esp_mqtt_client_init(&mqtt_cfg);
  // TODO: ここで解放していいの？<ダメだった
  // free((void *)mqtt_cfg.uri);
  // free((void *)mqtt_cfg.client_cert_pem);
  // free((void *)mqtt_cfg.client_key_pem);
  // free((void *)mqtt_cfg.cert_pem);

  strlcpy_nvs_value(_topic, nvs_handle, "mqtt_dev_topic", sizeof(_topic));
  strlcpy_nvs_value(_response_topic, nvs_handle, "mqtt_res_topic", sizeof(_response_topic));
  ESP_LOGI(_TAG, "mqtt_dev_topic=%s", _topic);
  ESP_LOGI(_TAG, "mqtt_res_topic=%s", _response_topic);

  nvs_close(nvs_handle);

  /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
  esp_mqtt_client_register_event(_client, ESP_EVENT_ANY_ID, _mqtt_event_handler, NULL);
  esp_mqtt_client_start(_client);

  return ESP_OK;
}

esp_err_t ncb_mqtt_publish_to_response_topic(const char *message) {
  if (strlen(_response_topic) > 0) {
    esp_mqtt_client_publish(_client, _response_topic, message, 0, QOS1, 0);
    return ESP_OK;
  } else {
    return ESP_FAIL;
  }
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