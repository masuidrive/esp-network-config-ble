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

static const char *TAG = "CONNECT AWS";

#define DEMO_EXIT_BIT (BIT0)

static EventGroupHandle_t EXIT_FLAG;

#define aws_root_ca_pem_start                                                                                          \
  "-----BEGIN "                                                                                                        \
  "CERTIFICATE-----\nMIIDQTCCAimgAwIBAgITBmyfz5m/"                                                                     \
  "jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\nADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\nb24gUm9vdC" \
  "BDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\nMAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem" \
  "9uIFJv\nb3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\nca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOg" \
  "Q3pOsqTQNroBvo3bSMgHFzZM\n9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\nIFAGbHrQgLKm+a/"        \
  "sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\nVOujw5H5SNz/"                                                    \
  "0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/"         \
  "IIDEgEw+OyQm\njgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/"                                                        \
  "zAOBgNVHQ8BAf8EBAMC\nAYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\nA4IBAQCY8jdaQZChGsV2USggNiM" \
  "OruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\nU5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/"                        \
  "msv0tadQ1wUs\nN+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\no/"                                 \
  "ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n5MsI+yMRQ+"                                         \
  "hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\nrqXRfboQnoZsG4q5WTP468SQvvG5\n-----END CERTIFICATE-----"
#define certificate_pem_crt_start                                                                                      \
  "-----BEGIN "                                                                                                        \
  "CERTIFICATE-----"                                                                                                   \
  "\nMIIDWTCCAkGgAwIBAgIUZCotGTpsXrD1WHzLELhPocXEDVYwDQYJKoZIhvcNAQEL\nBQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcy" \
  "BPPUFtYXpvbi5jb20g\nSW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTIxMDUxNTEzMTUy\nMloXDTQ5MTIzMTIzNTk1OVowHjEc" \
  "MBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\nZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAN6TdEvvDRCyX8kikJjw\nHTBvF5RfQX" \
  "Lo6HM8W95oh3EeltqkdV8tqKta2kIsJvjI6meCCKhSehTXX7YLha6L\nFoUb7fV2vjLSFb0meURsrBAxbGfiwzSPi9T6mr3PjA3lnwgYpa5Xo/"     \
  "zoVfMp9tct\nOaLvmxI19lijdRywiNWQhhHXsLH3WpdvZjvDJENwdZjSxFGpbBxGnyYk5LxajXrX\nmj6/Zm3DBIn2ZziqBwe16c9X80+CQ/"       \
  "R6K1ckci9XgRG4RZ56pQVClKkSDmIlUQOK\nY176f2bksvGpoChNw7Fvr6/imUaOCdrwXLYpEY5NI2RcymZcO/"                             \
  "DwtTLNOw14m2X+\ncX0CAwEAAaNgMF4wHwYDVR0jBBgwFoAU4wHK276I0DI4lwOU/"                                                  \
  "xK0ulhunnEwHQYD\nVR0OBBYEFAD2m09eZEuAgX0Kbdc2XlQ7m9SQMAwGA1UdEwEB/wQCMAAwDgYDVR0P\nAQH/"                            \
  "BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQA/9shAGC48IRtkv3+o6bvH8QPr\n/"                                                    \
  "c0KDA3UdD51I4YnTy5ulMSXQ1ukFuRdPRKROiuLiVe6DFMCmU1L7NTTozMP9vND\n5WX15KLctLKqcYLbXLC/"                              \
  "UeOd4IoJa0sOeYdiLvaVfFf4mC3A47JfHv2wqK+V1Rop\nYcjjzYGHhJy7m+dme3m273IdjVmLOfQvK+"                                   \
  "EJlO2C9Kiu6XCaFGBnVkgmf6ePhIML\nq7UpQLvAZxCFj4t/"                                                                   \
  "B+uotr+MbnyXzmMxo5wJR9bw1d6JHR5BsBILAUTTfSI3BgwT\nFRD7pbi5BBQC9+tfMWwWLz+SbEaSNsAK6OJR+mRgDJxUxX58DYeb1O42QaLF\n--" \
  "---END CERTIFICATE-----\n"
#define private_pem_key_start                                                                                          \
  "-----BEGIN RSA PRIVATE "                                                                                            \
  "KEY-----\nMIIEpAIBAAKCAQEA3pN0S+8NELJfySKQmPAdMG8XlF9Bcujoczxb3miHcR6W2qR1\nXy2oq1raQiwm+"                          \
  "MjqZ4IIqFJ6FNdftguFrosWhRvt9Xa+MtIVvSZ5RGysEDFsZ+LD\nNI+L1Pqavc+MDeWfCBilrlej/"                                     \
  "OhV8yn21y05ou+"                                                                                                     \
  "bEjX2WKN1HLCI1ZCGEdewsfda\nl29mO8MkQ3B1mNLEUalsHEafJiTkvFqNeteaPr9mbcMEifZnOKoHB7Xpz1fzT4JD\n9HorVyRyL1eBEbhFnnqlB" \
  "UKUqRIOYiVRA4pjXvp/"                                                                                                \
  "ZuSy8amgKE3DsW+vr+KZRo4J\n2vBctikRjk0jZFzKZlw78PC1Ms07DXibZf5xfQIDAQABAoIBAQCA591+"                                 \
  "7OAgP26C\nSuZ77PS6zFxXriN1rbaSpu5+DQQEW7b6wizIBuWc+"                                                                \
  "G1XzcJ0a8h2UjU5ZysVTvc7\ns8ikoiFUziwu8aqOPGYiulmZa2brqfQi5o4vrsNJgiqEq+NtWQOV6NBr/I4ZN2Yw\nAqnXTXYIC8U/"            \
  "8sjN5T06dHOLXLyJL9EtWKizFp3A9cDYosz0wlD/"                                                                           \
  "gSHNkw+m3obm\neWsug+BFsEutopRA1mbLsZwAFR6w7XfbvPn0snioK0NszcuLYKostFzyd7r9x+Po\ns3zIIHbi+dvFd5ObDbV4bp2Gr8haiA/"    \
  "3GIwhXPXyUy1+Hiu1heoUHjRICfiW7v81\nE+1chInxAoGBAP67sgj2aWH7PGLAr1nK+"                                               \
  "pdTvngEkjeuhIqdGjEzIxTd3zNf1fbp\n9uPxN7xLfg3RWyq6clPdbW08NMFvGBMJr/y1j40rIah/"                                      \
  "1VCBulCEavRHU7ySL4sN\nXXSwnXBw7YmPXtJ9JwCiOd6027bEI8o6y8Y12YvdCXC4NZuDzhObnjlzAoGBAN+"                              \
  "u\n0ay8ygKwrgZKQ3VgQyJSsnp7VLPpYWYl1QBdS13YHg1aLeWliaw37ld1LQyYAdmm\nzv0p0vH+"                                      \
  "h0JM0zddZCnZJU1HEEs5N1Wnf0ekSnizVwVPi662j1fOUQYHymCMDHXU\nYRsfWHn2jp38Ev9oF4jNeFowkQM9Do6z3dPJSK1PAoGBAPySdIuIb6MU" \
  "LaKo9PSu\nWtuy0DdGKQiGxKp/"                                                                                         \
  "7Rl9X7pv58CTtJ5XC1KluSK9I6Zlv+RU68kV00stGA33rkIq\nJ9IE3+oUKxJcddCWYHs3YKkHFBL0S6mvPcvW1rRHdG8htd4083ggYgrc/"        \
  "sFwJyB8\nPdIxUJvqNVMLgzPIc4tjkHXbAoGAcn0ARPG8U/Qqnn2UWZeTxRWe1KIbkJp7h9UF\nWNJnqPjdKOhnZYt9E/DsxUbsb+29xLFkcJ/"     \
  "foI0unfjEgyoCAolqVktxQ49dizHY\nK8sjcvQYLlewnxRB+idXwUZrh08ncYfqy/"                                                  \
  "8XQhW4mZRUBPY6VYMVftQs5QNzWAco\nGyaDVHECgYAk6RLQ9XISeogRAcn5c9NNcWsSCllBDgBSQ1D6Y3+"                                \
  "SeeqthqSaIDNk\n2UEdOqIpXqJqC3O0pNDCwgN6rSAux2xGj+KDDvMRYppiypuj2VlaziI1a+eyeZjd\nCqKb+"                             \
  "UHgndM1434Cqqw1d0x9slSbAauPU8Av802GfowClEBFw2ktyA==\n-----END RSA PRIVATE KEY-----\n"
#define CONFIG_AWS_EXAMPLE_CLIENT_ID "ad859d8d71af52a56045f824ae13d017bf0e6618beaa5fd3584124689531bcae"

/**
 * @brief Default MQTT HOST URL is pulled from the aws_iot_config.h
 */
#define HostAddress "ai9nvtn1ux7sl-ats.iot.ap-northeast-1.amazonaws.com"

/**
 * @brief Default MQTT port is pulled from the aws_iot_config.h
 */
#define MQTT_PORT 443

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

esp_err_t connect_aws_with_nvs() {
  char cPayload[100];

  int32_t i = 0;

  IoT_Error_t rc = FAILURE;

  AWS_IoT_Client client;
  IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
  IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

  ESP_LOGI(TAG, "AWS IoT SDK Version %d.%d.%d-%s", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

  mqttInitParams.enableAutoReconnect = false; // We enable this later below
  mqttInitParams.pHostURL = HostAddress;
  mqttInitParams.port = MQTT_PORT;
  mqttInitParams.pRootCALocation = (const char *)aws_root_ca_pem_start;
  mqttInitParams.pDeviceCertLocation = (const char *)certificate_pem_crt_start;
  mqttInitParams.pDevicePrivateKeyLocation = (const char *)private_pem_key_start;

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
  connectParams.pClientID = CONFIG_AWS_EXAMPLE_CLIENT_ID;
  connectParams.clientIDLen = (uint16_t)strlen(CONFIG_AWS_EXAMPLE_CLIENT_ID);
  connectParams.isWillMsgPresent = false;

  ESP_LOGI(TAG, "Connecting to AWS...");
  do {
    rc = aws_iot_mqtt_connect(&client, &connectParams);
    if (SUCCESS != rc) {
      ESP_LOGE(TAG, "Error(%d) connecting to %s:%d", rc, mqttInitParams.pHostURL, mqttInitParams.port);
      vTaskDelay(1000 / portTICK_RATE_MS);
    }
  } while (SUCCESS != rc);

  /*
   * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
   *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
   *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
   */
  rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
  if (SUCCESS != rc) {
    ESP_LOGE(TAG, "Unable to set Auto Reconnect to true - %d", rc);
    abort();
  }

  // const char *TOPIC = "test_topic/esp32";
  const char *TOPIC = "$aws/things/talkie-development-1/shadow";
  const int TOPIC_LEN = strlen(TOPIC);

  ESP_LOGI(TAG, "Subscribing...");
  rc = aws_iot_mqtt_subscribe(&client, TOPIC, TOPIC_LEN, QOS0, iot_subscribe_callback_handler, NULL);
  if (SUCCESS != rc) {
    ESP_LOGE(TAG, "Error subscribing : %d ", rc);
    abort();
  }

  sprintf(cPayload, "%s : %d ", "hello from SDK", i);

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