import { BLEUART } from "./ble-uart";
import { SSIDItem, BLESmartConfig, IoTConfig } from "./configure";
import { TypedEvent } from "./event-emitter";

export type PageStartupState =
  | "Ready"
  | "Connecting"
  | "Disconnected"
  | "NotSupported"
  | "ConnectError";

export interface PageStartup {
  state: PageStartupState;
}

export const enum PageSelectSSIDState {
  Loading,
  Loaded,
  Error,
}

export interface PageSelectSSID {
  state: PageSelectSSIDState;
  listSSID: SSIDItem[];
}

export type PageUpdatingState =
  | "ConnectingWifi"
  | "ConnectWiFiError"
  | "SetVariables"
  | "Fetching"
  | "FetchError"
  | "ConnectingMQTT"
  | "ConnectMQTTError"
  | "Other"
  | "OtherError"
  | "Finished";

export interface PageUpdating {
  state: PageUpdatingState;
}

export interface PageFinished {}

export class BLESmartConfigWizard {
  private uart: BLEUART;
  private smartConfig?: BLESmartConfig;
  private fetchConfig: (deviceId: string) => Promise<IoTConfig | undefined>;

  pageStartup = new TypedEvent<PageStartup>();
  pageSelectSSID = new TypedEvent<PageSelectSSID>();
  pageUpdating = new TypedEvent<PageUpdating>();
  pageFinished = new TypedEvent<PageFinished>();

  constructor(
    prefix: string,
    fetchConfig: (deviceId: string) => Promise<IoTConfig | undefined>
  ) {
    this.uart = new BLEUART(prefix);
    this.fetchConfig = fetchConfig;
  }

  // start this wizard flow
  start(): boolean {
    this.uart.disconnect();
    if (this.uart.supported) {
      this.pageStartup.emit({
        state: "Ready",
      });
    }
    return this.uart.supported;
  }

  // push "open device button"
  async openDevice(): Promise<void> {
    // Choice device
    try {
      this.pageStartup.emit({
        state: "Connecting",
      });
      if (!(await this.uart.start())) throw new Error();
    } catch (e) {
      this.uart.disconnect();
      this.pageStartup.emit({
        state: "ConnectError",
      });
      return;
    }

    // Connect to Device
    try {
      this.smartConfig = new BLESmartConfig(this.uart);
      if (!(await this.smartConfig!.connect())) throw new Error();
    } catch (e) {
      this.smartConfig = undefined;
      this.pageStartup.emit({
        state: "ConnectError",
      });
      return;
    }

    await this.loadSSID();
  }

  async loadSSID(): Promise<void> {
    // Load SSID list
    try {
      this.pageSelectSSID.emit({
        state: PageSelectSSIDState.Loading,
        listSSID: [],
      });
      const listSSID = await this.smartConfig!.list_ssid();
      this.pageSelectSSID.emit({
        state: PageSelectSSIDState.Loaded,
        listSSID: listSSID,
      });
    } catch (e) {
      this.pageSelectSSID.emit({
        state: PageSelectSSIDState.Error,
        listSSID: [],
      });
      return;
    }
  }

  async connectWiFi(ssid: string, passphrase: string): Promise<void> {
    // connect WiFi
    try {
      this.pageUpdating.emit({
        state: "ConnectingWifi",
      });
      const result_set_wifi = await this.smartConfig!.set_wifi(
        ssid,
        passphrase
      );
      if (!result_set_wifi) throw new Error();
      const result_check_wifi = await this.smartConfig!.check_wifi();
      if (!result_check_wifi) throw new Error();
    } catch (e) {
      this.pageUpdating.emit({
        state: "ConnectWiFiError",
      });
      return;
    }

    // Fetch MQTT and config data from callback
    let config: IoTConfig | undefined = undefined;
    try {
      this.pageUpdating.emit({
        state: "Fetching",
      });
      const device_id = await this.smartConfig!.device_id();
      config = await this.fetchConfig(device_id);
      if (config === undefined) throw new Error();
    } catch (e) {
      this.pageUpdating.emit({
        state: "FetchError",
      });
      return;
    }

    // Set MQTT and check it
    try {
      this.pageUpdating.emit({
        state: "ConnectingMQTT",
      });
      const result_set_mqtt = await this.smartConfig!.set_mqtt(config!);
      if (!result_set_mqtt) throw new Error();
      const result_check_mqtt = await this.smartConfig!.check_mqtt();
      if (!result_check_mqtt) throw new Error();
    } catch (e) {
      this.pageUpdating.emit({
        state: "ConnectMQTTError",
      });
      return;
    }
    this.pageFinished.emit({});
    return;
  }
}
