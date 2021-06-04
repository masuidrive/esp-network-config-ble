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

export class BLESmartConfig {
  uart: BLEUART;

  constructor(uart: BLEUART) {
    this.uart = uart;
  }

  async list_ssid(): Promise<SSIDItem[]> {
    let result: SSIDItem[] = [];
    await this.uart.sendln("LIST_SSID");
    while (true) {
      const line = await this.uart.readline();
      console.log(`[${line}]`);
      if (line === "") break;
      result.push(new SSIDItem(line));
    }
    console.log(result);
    return result;
  }
}
