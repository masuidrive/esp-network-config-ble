#include "network-config-ble-internal.h"
static const char *_TAG = "NCB_GET";

void _ncb_command_GET_STR(int argc, const char *args[], int datac, const char *data[]) {
  if (argc != 1) {
    _NCB_SEND_ERROR("ignore command format");
    return;
  }
  const char *name = args[0];

  if (strcmp("device_id", name) == 0) {
    _NCB_SEND_RESULT(_ncb_device_id);
  } else if (strcmp("firmware_version", name) == 0) {
    _NCB_SEND_RESULT(_ncb_firmware_version);
  } else {
    _NCB_SEND_ERROR("not found");
    return;
  }

  _NCB_SEND_OK();
  return;
}
