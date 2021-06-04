import { BLEUART } from "./ble-uart";
export { BLEUART } from "./ble-uart";
export declare class SSIDItem {
    ssid: string;
    rssi: number;
    authmode: number;
    constructor(line: string);
    isOpen(): boolean;
}
export declare class BLESmartConfig {
    uart: BLEUART;
    constructor(uart: BLEUART);
    list_ssid(): Promise<SSIDItem[]>;
}
//# sourceMappingURL=index.d.ts.map