import { BLEUART } from "./ble-uart";
export { BLEUART } from "./ble-uart";
export declare class SSIDItem {
    ssid: string;
    rssi: number;
    authmode: number;
    constructor(line: string);
    isOpen(): boolean;
}
export declare class IoTConfig {
    mqtt_host: string;
    mqtt_port: string;
    mqtt_topic: string;
    root_ca: string;
    cert: string;
    priv: string;
    client_id: string;
}
export declare class BLESmartConfig {
    uart: BLEUART;
    constructor(uart: BLEUART);
    list_ssid(): Promise<SSIDItem[]>;
    test_wifi_connection(): Promise<boolean>;
    set_wifi(ssid: string, passphrase: string): Promise<boolean>;
    set_awsiot(config: IoTConfig): Promise<boolean>;
    send_text(key: string, value: string): Promise<boolean>;
    send_text_multiline(key: string, value: string): Promise<boolean>;
}
//# sourceMappingURL=index.d.ts.map