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

export class BLESmartConfig {
  uart: BLEUART;

  constructor(uart: BLEUART) {
    this.uart = uart;
  }

  async list_ssid(): Promise<SSIDItem[]> {
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
    console.log(`SEND ${key}`);
    await this.uart.sendln(`SET_STR ${key} ${JSON.stringify(value)}`);
    const result = await this.uart.readline();
    console.log(`RESULT ${key}: ${result}`);
    await this.uart.waitBlank();
    console.log(`FIN ${key}`);
    return !!result.match(/^OK/);
  }

  async send_text_multiline(key: string, value: string): Promise<boolean> {
    console.log(`SENDm ${key}`);
    await this.uart.sendln(`SET_MULTI ${key}`);
    console.log("VAL1:", value);
    console.log(
      "VAL2:",
      value.replaceAll(/\r/g, "").replace(/\n+$/, "").split(/\n+/)
    );

    const lines = value.replaceAll(/\r/g, "").replace(/\n+$/, "").split(/\n+/);
    for (const line of lines) {
      console.log("L:" + line);
      await this.uart.sendln(line);
    }

    await this.uart.sendln("");
    const result = await this.uart.readline();
    console.log(`RESULTm ${key}: ${result}`);
    await this.uart.waitBlank();
    console.log(`FINm ${key}`);
    return !!result.match(/^OK/);
  }
}
