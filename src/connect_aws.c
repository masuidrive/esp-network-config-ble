#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_mqtt_client_interface.h"
#include "aws_iot_version.h"

#define RETRY_COUNT 5

static const char *TAG = "CONNECT AWS";

void iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                    IoT_Publish_Message_Params *params, void *pData) {
  ESP_LOGI(TAG, "Subscribe callback");
  ESP_LOGI(TAG, "%.*s\t%.*s", topicNameLen, topicName, (int)params->payloadLen, (char *)params->payload);
}

void disconnectCallbackHandler(AWS_IoT_Client *pClient, void *data) {
  ESP_LOGW(TAG, "MQTT Disconnect");
  IoT_Error_t rc = FAILURE;

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
  ESP_ERROR_CHECK(nvs_get_str(nvs_handle, name, NULL, &required_size));
  char *value = malloc(required_size);
  ESP_ERROR_CHECK(nvs_get_str(nvs_handle, name, value, &required_size));
  return value;
}

esp_err_t connect_aws_with_nvs() {
  size_t required_size;

  int32_t i = 0;

  IoT_Error_t rc = FAILURE;

  AWS_IoT_Client client;
  IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
  IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

  ESP_LOGI(TAG, "AWS IoT SDK Version %d.%d.%d-%s", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

  mqttInitParams.enableAutoReconnect = false; // We enable this later below

  nvs_handle_t nvs_handle;
  ESP_ERROR_CHECK(nvs_open("bleconfig", NVS_READWRITE, &nvs_handle));
  mqttInitParams.pHostURL = get_nvs_value(nvs_handle, "mqtt_host");
  mqttInitParams.port = atoi(get_nvs_value(nvs_handle, "mqtt_port"));
  mqttInitParams.pRootCALocation = get_nvs_value(nvs_handle, "iot_root_ca");
  mqttInitParams.pDeviceCertLocation = get_nvs_value(nvs_handle, "iot_cert");
  mqttInitParams.pDevicePrivateKeyLocation = get_nvs_value(nvs_handle, "iot_priv");

  connectParams.pClientID = get_nvs_value(nvs_handle, "iot_client_id");
  connectParams.clientIDLen = (uint16_t)strlen(connectParams.pClientID);

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
  /* Client ID is set in the menuconfig of the example */
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
    ESP_LOGE(TAG, "Unable to ceconnect - %d", rc);
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

  // const char *TOPIC = "test_topic/esp32";
  // const char *TOPIC = "$aws/things/talkie-development-1/shadow";
  const char *TOPIC = "demo";
  const int TOPIC_LEN = strlen(TOPIC);

  ESP_LOGI(TAG, "Subscribing...");
  rc = aws_iot_mqtt_subscribe(&client, TOPIC, TOPIC_LEN, QOS0, iot_subscribe_callback_handler, NULL);
  if (SUCCESS != rc) {
    ESP_LOGE(TAG, "Error subscribing : %d ", rc);
    return ESP_FAIL;
  }

  rc = aws_iot_mqtt_yield(&client, 100);
  if (SUCCESS != rc) {
    ESP_LOGE(TAG, "Error subscribing : %d ", rc);
    return ESP_FAIL;
  }

  ESP_LOGI(TAG, "loop...");
  while ((NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc)) {

    // Max time the yield function will wait for read messages
    rc = aws_iot_mqtt_yield(&client, 100);
    if (NETWORK_ATTEMPTING_RECONNECT == rc) {
      // If the client is attempting to reconnect we will skip the rest of the loop.
      continue;
    }
    /*
    ESP_LOGI(TAG, "Stack remaining for task '%s' is %d bytes", pcTaskGetTaskName(NULL),
    uxTaskGetStackHighWaterMark(NULL)); vTaskDelay(1000 / portTICK_RATE_MS); sprintf(cPayload, "%s : %d ", "hello from
    ESP32 (QOS0)", i++); paramsQOS0.payloadLen = strlen(cPayload); rc = aws_iot_mqtt_publish(&client, TOPIC, TOPIC_LEN,
    &paramsQOS0);

    sprintf(cPayload, "%s : %d ", "hello from ESP32 (QOS1)", i++);
    paramsQOS1.payloadLen = strlen(cPayload);
    rc = aws_iot_mqtt_publish(&client, TOPIC, TOPIC_LEN, &paramsQOS1);
    if (rc == MQTT_REQUEST_TIMEOUT_ERROR)
    {
        ESP_LOGW(TAG, "QOS1 publish ack not received.");
        rc = SUCCESS;
    }
*/
  }

  ESP_LOGE(TAG, "An error occurred in the main loop.");
  abort();
}