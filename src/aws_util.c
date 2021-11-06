#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "aws_util.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "esp_tls.h"
#include "mqtt_client.h"

#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_mqtt_client_interface.h"
#include "aws_iot_version.h"

#define RETRY_COUNT 5

static const char *TAG = "CONNECT AWS";
static awsiot_mqtt_receiver_callback mqtt_receiver = NULL;
static awsiot_status_callback status_callback = NULL;

void iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                    IoT_Publish_Message_Params *params, void *pData) {
  ESP_LOGI(TAG, "%.*s\t%.*s", topicNameLen, topicName, (int)params->payloadLen, (char *)params->payload);
  if (mqtt_receiver)
    mqtt_receiver(topicName, (char *)params->payload);
}

void disconnectCallbackHandler(AWS_IoT_Client *pClient, void *data) {
  ESP_LOGW(TAG, "MQTT Disconnect");
  IoT_Error_t rc = FAILURE;
  if (status_callback)
    status_callback(AWSIOT_DISCONNECTED);

  if (NULL == pClient) {
    return;
  }

  if (aws_iot_is_autoreconnect_enabled(pClient)) {
    ESP_LOGI(TAG, "Auto Reconnect is enabled, Reconnecting attempt will start now");
  } else {
    ESP_LOGW(TAG, "Auto Reconnect not enabled. Starting manual reconnect...");
    rc = aws_iot_mqtt_attempt_reconnect(pClient);
    if (NETWORK_RECONNECTED == rc) {
      ESP_LOGW(TAG, "Manual Reconnect Successful");
    } else {
      ESP_LOGW(TAG, "Manual Reconnect Failed - %d", rc);
    }
  }
}

const char *aws_root_ca_pem_start;
const char *certificate_pem_crt_start;
const char *private_pem_key_start;
const char *iot_host_name;
uint16_t iot_host_port;

const char *get_nvs_value(nvs_handle_t nvs_handle, const char *name) {
  size_t required_size = 0;
  esp_err_t ret = nvs_get_str(nvs_handle, name, NULL, &required_size);
  if (ret != ESP_OK) {
    return NULL;
  }
  char *value = malloc(required_size);
  ESP_ERROR_CHECK(nvs_get_str(nvs_handle, name, value, &required_size));
  return value;
}

static void log_error_if_nonzero(const char *message, int error_code) {
  if (error_code != 0) {
    ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
  }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;
  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

    msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

    msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
    ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
    break;

  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
    msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_DATA:
    ESP_LOGI(TAG, "MQTT_EVENT_DATA");
    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
    if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
      log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
      log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
      log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
      ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
    }
    break;
  default:
    ESP_LOGI(TAG, "Other event id:%d", event->event_id);
    break;
  }
}

static AWS_IoT_Client client;
static char topic[256];

esp_err_t awsiot_connect_with_nvs(awsiot_mqtt_receiver_callback message_callback,
                                  awsiot_status_callback _status_callback) {
  mqtt_receiver = message_callback;
  status_callback = _status_callback;
  nvs_handle_t nvs_handle;
  ESP_ERROR_CHECK(nvs_open("bleconfig", NVS_READWRITE, &nvs_handle));
  const esp_mqtt_client_config_t mqtt_cfg = {
      .uri = "mqtts://ai9nvtn1ux7sl-ats.iot.ap-northeast-1.amazonaws.com:8883",
      .client_cert_pem = (const char *)get_nvs_value(nvs_handle, "iot_cert"),
      .client_key_pem = (const char *)get_nvs_value(nvs_handle, "iot_priv"),
      .cert_pem = (const char *)get_nvs_value(nvs_handle, "iot_root_ca"),
  };
  // esp_log_level_set(TAG, ESP_LOG_DEBUG);

  ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
  /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
  esp_mqtt_client_start(client);

  return ESP_OK;
}

esp_err_t _awsiot_connect_with_nvs(awsiot_mqtt_receiver_callback message_callback,
                                   awsiot_status_callback _status_callback) {
  mqtt_receiver = message_callback;
  status_callback = _status_callback;
  size_t required_size;

  int32_t i = 0;

  IoT_Error_t rc = FAILURE;

  IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
  IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

  ESP_LOGI(TAG, "AWS IoT SDK Version %d.%d.%d-%s", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

  mqttInitParams.enableAutoReconnect = false; // set true later

  nvs_handle_t nvs_handle;
  ESP_ERROR_CHECK(nvs_open("bleconfig", NVS_READWRITE, &nvs_handle));
  // TODO: これらのメモリをどこで解放しよう？
  mqttInitParams.pHostURL = get_nvs_value(nvs_handle, "mqtt_host");
  mqttInitParams.port = atoi(get_nvs_value(nvs_handle, "mqtt_port"));
  mqttInitParams.pRootCALocation = get_nvs_value(nvs_handle, "iot_root_ca");
  mqttInitParams.pDeviceCertLocation = get_nvs_value(nvs_handle, "iot_cert");
  mqttInitParams.pDevicePrivateKeyLocation = get_nvs_value(nvs_handle, "iot_priv");

  connectParams.pClientID = get_nvs_value(nvs_handle, "iot_client_id");
  connectParams.clientIDLen = (uint16_t)strlen(connectParams.pClientID);

  char *_topic = get_nvs_value(nvs_handle, "mqtt_topic");
  strncpy(topic, _topic, sizeof(topic));

  nvs_close(nvs_handle);

  mqttInitParams.mqttCommandTimeout_ms = 20000;
  mqttInitParams.tlsHandshakeTimeout_ms = 5000;
  mqttInitParams.isSSLHostnameVerify = true;
  mqttInitParams.disconnectHandler = disconnectCallbackHandler;
  mqttInitParams.disconnectHandlerData = NULL;

  rc = aws_iot_mqtt_init(&client, &mqttInitParams);
  if (SUCCESS != rc) {
    ESP_LOGE(TAG, "aws_iot_mqtt_init returned error : %d ", rc);
    abort();
  }

  connectParams.keepAliveIntervalInSec = 10;
  connectParams.isCleanSession = true;
  connectParams.MQTTVersion = MQTT_3_1_1;
  connectParams.isWillMsgPresent = false;

  ESP_LOGI(TAG, "Connecting to AWS...");

  int retry_count = 0;
  do {
    rc = aws_iot_mqtt_connect(&client, &connectParams);
    if (SUCCESS != rc) {
      ESP_LOGE(TAG, "Error(%d) connecting to %s:%d", rc, mqttInitParams.pHostURL, mqttInitParams.port);
      vTaskDelay(1000 / portTICK_RATE_MS);
    }
  } while (SUCCESS != rc && retry_count++ < RETRY_COUNT);
  rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
  if (SUCCESS != rc) {
    ESP_LOGE(TAG, "Unable to connect - %d", rc);
    return ESP_FAIL;
  }

  /*
   * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
   *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
   *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
   */
  rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
  if (SUCCESS != rc) {
    ESP_LOGE(TAG, "Unable to set Auto Reconnect to true - %d", rc);
    return ESP_FAIL;
  }

  const int topic_len = strlen(topic);

  ESP_LOGI(TAG, "Subscribing...");
  rc = aws_iot_mqtt_subscribe(&client, topic, topic_len, QOS0, iot_subscribe_callback_handler, NULL);
  if (SUCCESS != rc) {
    ESP_LOGE(TAG, "Error subscribing : %d ", rc);
    return ESP_FAIL;
  }

  rc = aws_iot_mqtt_yield(&client, 100);
  if (SUCCESS != rc) {
    ESP_LOGE(TAG, "Error subscribing : %d ", rc);
    return ESP_FAIL;
  }

  ESP_LOGI(TAG, ">> waiting test");
  BaseType_t xReturned;
  TaskHandle_t xHandle = NULL;

  /* Create the task, storing the handle. */
  xReturned = xTaskCreate(awsiot_loop_task, /* Function that implements the task. */
                          "NAME",           /* Text name for the task. */
                          8192,             /* Stack size in words, not bytes. */
                          (void *)1,        /* Parameter passed into the task. */
                          tskIDLE_PRIORITY, /* Priority at which the task is created. */
                          &xHandle);        /* Used to pass out the created task's handle. */

  if (xReturned == pdPASS) {
    /* The task was created.  Use the task's handle to delete the task. */
    // vTaskDelete(xHandle);
    // ESP_LOGI(TAG, "failed waiting");
  }
  // vTaskDelay((1000 / portTICK_RATE_MS) * 10);
  // vTaskDelete(xHandle);
  ESP_LOGI(TAG, "<< waiting");

  return ESP_OK;
}

esp_err_t awsiot_publish(const char *message) {
  IoT_Publish_Message_Params paramsQOS1;
  paramsQOS1.payloadLen = strlen(message);
  paramsQOS1.qos = QOS1;
  paramsQOS1.payload = (void *)message;
  paramsQOS1.isRetained = 0;
  int rc = aws_iot_mqtt_publish(&client, topic, strlen(topic), &paramsQOS1);
  if (rc == MQTT_REQUEST_TIMEOUT_ERROR) {
    ESP_LOGW(TAG, "QOS1 publish ack not received.");
    return ESP_FAIL;
  }
  return ESP_OK;
}

void awsiot_loop_task(void *pvParameters) {
  ESP_LOGI(TAG, ">> awsiot_loop_task");
  if (status_callback)
    status_callback(AWSIOT_CONNECTED);

  int rc = SUCCESS;
  while ((NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc)) {
    // Max time the yield function will wait for read messages
    rc = aws_iot_mqtt_yield(&client, 100);
    if (NETWORK_ATTEMPTING_RECONNECT == rc) {
      // If the client is attempting to reconnect we will skip the rest of the loop.
      continue;
    } else {
      if (status_callback)
        status_callback(AWSIOT_CONNECTED);
    }
  }
  if (status_callback)
    status_callback(AWSIOT_DISCONNECTED);

  ESP_LOGI(TAG, "awsiot_loop_task> END");
}

esp_err_t awsiot_disconnect() {
  if (aws_iot_mqtt_disconnect(&client) == SUCCESS) {
    return ESP_OK;
  } else {
    return ESP_FAIL;
  }
}