import { StateMachine, tFrom } from "./fsm";
import { BLEUART } from "./ble-uart";

export { BLEUART };
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
  mqtt_uri: string = "";
  mqtt_dev_topic: string = "";
  mqtt_res_topic: string = "";
  root_ca: string = "";
  cert: string = "";
  priv: string = "";
  client_id: string = "";
}

export const State = {
  Disconnected: "Disconnected",
  Connecting: "Connecting",
  Connected: "Connected",
  Processing: "Processing",
} as const;
export type States = typeof State[keyof typeof State];

export const BLESmartConfigError = {
  Disconnected: "Disconnected",
  AlreadyConnected: "AlreadyConnected",
} as const;
export type BLESmartConfigErrors =
  typeof BLESmartConfigError[keyof typeof BLESmartConfigError];

export class BLESmartConfig {
  private uart: BLEUART;
  public state: StateMachine<States>;
  public onChange?: (
    prevState: States,
    currentState: States
  ) => void | Promise<void>;

  transitions = [
    tFrom(State.Connected, State.Disconnected),
    tFrom(State.Connecting, State.Disconnected),
    tFrom(State.Disconnected, State.Connecting),
    tFrom(State.Disconnected, State.Connected),
    tFrom(State.Connecting, State.Connected),
    tFrom(State.Connected, State.Processing),
    tFrom(State.Processing, State.Connected),
    tFrom(State.Processing, State.Disconnected),
  ];

  constructor(
    uart: BLEUART,
    onChange?: (prevState: States, currentState: States) => void | Promise<void>
  ) {
    this.uart = uart;
    this.state = new StateMachine<States>(
      State.Disconnected,
      this.transitions,
      onChange
    );
    this.uart.addListener("disconnect", () =>
      this.state.moveTo(State.Disconnected)
    );
  }

  async connect(): Promise<boolean> {
    if (this.uart.isConnected()) {
      await this.state.moveTo(State.Connected);
      return true;
    }

    await this.state.moveTo(State.Connecting);
    try {
      if (!(await this.uart.start())) {
        await this.disconnect();
        return false;
      }
      await this.state.moveTo(State.Connected);
    } catch (e) {
      console.error(e);
      await this.disconnect();
      return false;
    }
    return true;
  }

  async disconnect(): Promise<void> {
    try {
      this.uart.disconnect();
    } catch (e) {
      console.error(e);
    } finally {
      this.state.moveTo(State.Disconnected);
    }
  }

  async list_ssid(): Promise<SSIDItem[]> {
    let result: SSIDItem[] = [];
    await this.state.moveTo(State.Processing);

    try {
      this.uart.clear();
      await this.uart.sendln("LIST_SSID");
      let last_line = "";
      while (true) {
        const line = await this.uart.readline();
        if (line === "") break;
        last_line = line;
        const item = new SSIDItem(line);
        if (item.ssid !== "") {
          result.push(item);
        }
      }
      result.pop();
      if (!last_line.match(/^OK/)) throw new Error("device returned error");
      await this.state.moveTo(State.Connected);
      return result;
    } catch (e) {
      await this.disconnect();
      return [];
    }
  }

  async set_wifi(ssid: string, passphrase: string): Promise<boolean> {
    return await this.run_command(
      `SET_WIFI ${JSON.stringify(ssid)} ${JSON.stringify(passphrase)}`
    );
  }

  async test_wifi_connection(): Promise<boolean> {
    return await this.run_command("CHECK_WIFI");
  }

  async set_mqtt(config: IoTConfig): Promise<boolean> {
    await this.state.moveTo(State.Processing);
    try {
      if (!(await this.send_text("mqtt_uri", config.mqtt_uri)))
        throw new Error(`Error at mqtt_uri`);
      if (!(await this.send_text("mqtt_dev_topic", config.mqtt_dev_topic)))
        throw new Error(`Error at mqtt_dev_topic`);
      if (!(await this.send_text_multiline("mqtt_root_ca", config.root_ca)))
        throw new Error(`Error at root_ca`);
      if (!(await this.send_text_multiline("mqtt_cert", config.cert)))
        throw new Error(`Error at mqtt_cert`);
      if (!(await this.send_text_multiline("mqtt_priv", config.priv)))
        throw new Error(`Error at mqtt_priv`);

      await this.state.moveTo(State.Connected);
    } catch (e) {
      await this.disconnect();
      return false;
    }

    return true;
  }

  async restart(): Promise<boolean> {
    return await this.run_command("RESTART");
  }

  async check_mqtt(): Promise<boolean> {
    return await this.run_command("CHECK_MQTT");
  }

  async device_id(): Promise<string> {
    await this.state.moveTo(State.Processing);

    try {
      this.uart.clear();
      await this.uart.sendln("DEVICE_ID");
      const result = await this.uart.readline();
      await this.uart.waitBlank();
      await this.state.moveTo(State.Connected);

      return result;
    } catch (e) {
      console.error(e);
      await this.disconnect();
      return "";
    }
  }

  private async check_ok() {
    const result = await this.uart.readline();
    await this.uart.waitBlank();
    return !!result.match(/^OK/);
  }

  private async run_command(command: string): Promise<boolean> {
    await this.state.moveTo(State.Processing);

    try {
      this.uart.clear();
      await this.uart.sendln(command);
      const result = await this.check_ok();
      await this.state.moveTo(State.Connected);

      return result;
    } catch (e) {
      console.error(e);
      await this.disconnect();
      return false;
    }
  }

  private async send_text(key: string, value: string): Promise<boolean> {
    await this.uart.sendln(`SET_STR ${key} ${JSON.stringify(value)}`);
    return await this.check_ok();
  }

  private async send_text_multiline(
    key: string,
    value: string
  ): Promise<boolean> {
    const lines = value.replaceAll(/\r/g, "").replace(/\n+$/, "").split(/\n+/);
    await this.uart.sendln(`SET_MULTI ${key} ${lines.length}`);
    for (const line of lines) {
      await this.uart.sendln(line);
    }
    return await this.check_ok();
  }
}
