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
    test_wifi_connection(): Promise<boolean>;
    set_wifi(ssid: string, passphrase: string): Promise<boolean>;
}
//# sourceMappingURL=index.d.ts.map