#include "network-config-ble-internal.h"
static const char *_TAG = "NCB_DEVICEID";

void _ncb_command_RESTART(int argc, const char *args[], int datac, const char *data[]) { //
  esp_restart();
}
