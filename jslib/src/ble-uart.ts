import { TypedEvent } from "./event-emitter";

export class BLEUART {
  BLE_MTU = 128;

  bluetoothDevice?: BluetoothDevice;
  namePrefix: string;
  serviceUUID: string;
  rxUUID: string;
  txUUID: string;
  rxChar?: BluetoothRemoteGATTCharacteristic;
  txChar?: BluetoothRemoteGATTCharacteristic;

  onDisconnect = new TypedEvent<BLEUART>();
  onReceive = new TypedEvent<string>();
  onError = new TypedEvent<unknown>();

  constructor(
    namePrefix: string,
    serviceUUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e",
    rxUUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e",
    txUUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"
  ) {
    this.namePrefix = namePrefix;
    this.serviceUUID = serviceUUID;
    this.rxUUID = rxUUID;
    this.txUUID = txUUID;
  }

  async start(): Promise<boolean> {
    try {
      this.bluetoothDevice = await navigator.bluetooth.requestDevice({
        filters: [
          { services: [this.serviceUUID] },
          { namePrefix: this.namePrefix },
        ],
      });
      if (this.bluetoothDevice === undefined) return false;

      this.bluetoothDevice!.addEventListener("gattserverdisconnected", () => {
        this.onDisconnect.emit(this);
      });

      const server = await this.bluetoothDevice!.gatt!.connect();
      const service = await server.getPrimaryService(this.serviceUUID);
      this.rxChar = await service.getCharacteristic(this.rxUUID);
      this.txChar = await service.getCharacteristic(this.txUUID);
      await this.txChar.startNotifications();
      this.txChar!.addEventListener(
        "characteristicvaluechanged",
        this.handleNotifications.bind(this)
      );
      return true;
    } catch (e) {
      console.error(e);
      return false;
    }
  }

  disconnect() {
    this.bluetoothDevice?.gatt?.disconnect();
    this.bluetoothDevice = undefined;
  }

  isConnected() {
    return this.bluetoothDevice !== undefined;
  }

  async send(text: string) {
    const arrayBuffe = new TextEncoder().encode(text);
    for (let i = 0; i < arrayBuffe.length; i += this.BLE_MTU) {
      await this.rxChar!.writeValue(arrayBuffe.slice(i, i + this.BLE_MTU));
    }
  }

  async sendln(text: string) {
    await this.send(text + "\r\n");
  }

  readlineBuffer: string[] = [];
  readlineResolve?: (value: string | PromiseLike<string>) => void = undefined;
  readline(): Promise<string> {
    return new Promise((resolve, reject) => {
      if (this.readlineBuffer.length === 0) {
        this.readlineResolve = resolve;
      } else {
        this.readlineResolve = undefined;
        resolve(this.readlineBuffer.shift()!);
      }
    });
  }

  waitBlankResolve?: () => void = undefined;
  waitBlank(): Promise<void> {
    return new Promise((resolve, reject) => {
      this.waitBlankResolve = resolve;
    });
  }

  clear() {
    this.readlineBuffer = [];
    this.rx_buffer = "";
  }

  rx_buffer: string = "";
  private async handleNotifications(event: Event) {
    if (this.txChar) {
      try {
        const value = (event!.target! as any).value;
        const text = new TextDecoder().decode(value);
        this.rx_buffer += text;

        let splited = this.rx_buffer.split(/\r*\n/g);
        for (let i = 0; i < splited.length - 1; ++i) {
          const line = splited.shift();
          if (line !== undefined) {
            if (this.readlineResolve) {
              this.readlineResolve(line);
              this.readlineResolve = undefined;
            } else {
              this.readlineBuffer?.push(line);
            }
            if (this.waitBlankResolve && line === "") {
              this.clear();
              this.waitBlankResolve();
            }

            this.onReceive.emit(line);
          }
        }
        this.rx_buffer = splited.shift() ?? "";
      } catch (error) {
        console.error(error);
        this.onError.emit(error);
      }
    }
  }
}
