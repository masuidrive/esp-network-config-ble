/// <reference types="web-bluetooth" />
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
    start(): Promise<boolean>;
    disconnect(): void;
    isConnected(): boolean;
    onDisconnected(_: BluetoothDevice, ev: Event): void;
    send(text: string): Promise<void>;
    sendln(text: string): Promise<void>;
    readlineBuffer: string[];
    readlineResolve?: (value: string | PromiseLike<string>) => void;
    readline(): Promise<string>;
    waitBlankResolve?: () => void;
    waitBlank(): Promise<void>;
    clear(): void;
    rx_buffer: string;
    private handleNotifications;
}
//# sourceMappingURL=ble-uart.d.ts.map