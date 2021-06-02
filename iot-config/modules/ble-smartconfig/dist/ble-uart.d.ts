/// <reference types="web-bluetooth" />
/// <reference types="node" />
import { EventEmitter } from "events";
export declare class BLEUART extends EventEmitter {
    bluetoothDevice?: BluetoothDevice;
    namePrefix: string;
    serviceUUID: string;
    rxUUID: string;
    txUUID: string;
    rxChar?: BluetoothRemoteGATTCharacteristic;
    txChar?: BluetoothRemoteGATTCharacteristic;
    constructor(namePrefix: string, serviceUUID?: string, rxUUID?: string, txUUID?: string);
    start(): Promise<void>;
    onDisconnected(_: BluetoothDevice, ev: Event): void;
    send(text: string): Promise<void>;
    sendln(text: string): Promise<void>;
    readlineBuffer: string[];
    readlineResolve?: (value: string | PromiseLike<string>) => void;
    readline(): Promise<string>;
    rx_buffer: string;
    handleNotifications(event: Event): Promise<void>;
}
//# sourceMappingURL=ble-uart.d.ts.map