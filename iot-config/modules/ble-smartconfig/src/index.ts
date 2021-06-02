import { BLEUART } from "./ble-uart";
export { BLEUART } from "./ble-uart";

export class BLESmartConfig {
  uart: BLEUART;

  constructor(uart: BLEUART) {
    this.uart = uart;
  }

  async ssids(): Promise<string[]> {
    let result: string[] = [];
    await this.uart.sendln("LIST_SSID");
    while (true) {
      const line = await this.uart.readline();
      if (line === "") break;
      result.push(line);
    }

    return result;
  }
}
