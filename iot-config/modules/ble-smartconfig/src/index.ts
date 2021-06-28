import { StateMachine, ITransition, tFrom } from "./fsm";
import { BLEUART } from "./ble-uart";
export { BLEUART } from "./ble-uart";

export class SSIDItem {
  ssid: string;
  rssi: number;
  authmode: number;

  constructor(line: string) {
    const item = line.split(/,/, 3);
    this.authmode = parseInt(item[0], 10);
    this.rssi = parseInt(item[1], 10);
    this.ssid = item[2];
  }

  isOpen(): boolean {
    return this.authmode === 0;
  }
}

export class IoTConfig {
  mqtt_host: string = "";
  mqtt_port: string = "443";
  mqtt_topic: string = "";
  root_ca: string = "";
  cert: string = "";
  priv: string = "";
  client_id: string = "";
}

export const State = {
  Disconnected: "Disconnected",
  Connecting: "Connecting",
  Connected: "Connected",
  LoadingListSSID: "LoadingListSSID",
} as const;
export type States = typeof State[keyof typeof State];

export const BLESmartConfigError = {
  Disconnected: "Disconnected",
  AlreadyConnected: "AlreadyConnected",
} as const;
export type BLESmartConfigErrors =
  typeof BLESmartConfigError[keyof typeof BLESmartConfigError];

export class BLESmartConfig {
  uart: BLEUART;
  protected state: StateMachine<States>;
  protected onChange?: (
    prevState: States,
    currentState: States
  ) => void | Promise<void>;

  transitions = [
    tFrom(State.Disconnected, State.Connecting),
    tFrom(State.Connecting, State.Connected),
    tFrom(State.Connecting, State.Disconnected),
    tFrom(State.Connected, State.LoadingListSSID),
    tFrom(State.LoadingListSSID, State.Connected),
  ];

  constructor(
    uart: BLEUART,
    onChange: (prevState: States, currentState: States) => void | Promise<void>
  ) {
    this.uart = uart;
    this.state = new StateMachine<States>(
      State.Disconnected,
      this.transitions,
      onChange
    );
  }

  async connect(): Promise<void> {
    this.state.guard([State.Connecting, State.Disconnected]);
    if (await this.uart.start()) {
      this.state.moveTo(State.Connecting);
    } else {
      this.state.moveTo(State.Disconnected);
    }
  }

  async list_ssid(): Promise<SSIDItem[]> {
    this.state.guard([State.LoadingListSSID, State.Disconnected]);
    this.state.moveTo(State.LoadingListSSID);

    let result: SSIDItem[] = [];
    this.uart.clear();
    await this.uart.sendln("LIST_SSID");
    while (true) {
      const line = await this.uart.readline();
      if (line === "") break;
      const item = new SSIDItem(line);
      if (item.ssid !== "") {
        result.push(item);
      }
    }
    this.state.moveTo(State.Connected);

    return result;
  }

  async test_wifi_connection(): Promise<boolean> {
    this.uart.clear();
    await this.uart.sendln("CHECK_WIFI");
    const result = await this.uart.readline();
    console.log("test: " + result);
    await this.uart.waitBlank();
    console.log("test done");
    return !!result.match(/^OK/);
  }

  async set_wifi(ssid: string, passphrase: string): Promise<boolean> {
    this.uart.clear();
    await this.uart.sendln(
      "SET_WIFI " + JSON.stringify(ssid) + " " + JSON.stringify(passphrase)
    );
    const result = await this.uart.readline();
    await this.uart.waitBlank();
    return !!result.match(/^OK/);
  }

  async set_awsiot(config: IoTConfig): Promise<boolean> {
    this.uart.clear();

    await this.send_text("mqtt_host", config.mqtt_host);
    await this.send_text("mqtt_port", config.mqtt_port);
    await this.send_text("mqtt_topic", config.mqtt_topic);
    await this.send_text_multiline("iot_root_ca", config.root_ca);
    await this.send_text_multiline("iot_cert", config.cert);
    await this.send_text_multiline("iot_priv", config.priv);
    await this.send_text("iot_client_id", config.client_id);

    const result = await this.uart.readline();
    await this.uart.waitBlank();
    return !!result.match(/^OK/);
  }

  async send_text(key: string, value: string): Promise<boolean> {
    await this.uart.sendln(`SET_STR ${key} ${JSON.stringify(value)}`);
    const result = await this.uart.readline();
    await this.uart.waitBlank();
    return !!result.match(/^OK/);
  }

  async send_text_multiline(key: string, value: string): Promise<boolean> {
    await this.uart.sendln(`SET_MULTI ${key}`);

    const lines = value.replaceAll(/\r/g, "").replace(/\n+$/, "").split(/\n+/);
    for (const line of lines) {
      console.log("L:" + line);
      await this.uart.sendln(line);
    }

    await this.uart.sendln("");
    const result = await this.uart.readline();
    await this.uart.waitBlank();
    return !!result.match(/^OK/);
  }
}
