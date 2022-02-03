import { StateMachine, ITransition } from "./fsm";
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
    mqtt_uri: string;
    mqtt_topic: string;
    root_ca: string;
    cert: string;
    priv: string;
    client_id: string;
}
export declare const State: {
    readonly Disconnected: "Disconnected";
    readonly Connecting: "Connecting";
    readonly Connected: "Connected";
    readonly LoadingListSSID: "LoadingListSSID";
};
export declare type States = typeof State[keyof typeof State];
export declare const BLESmartConfigError: {
    readonly Disconnected: "Disconnected";
    readonly AlreadyConnected: "AlreadyConnected";
};
export declare type BLESmartConfigErrors = typeof BLESmartConfigError[keyof typeof BLESmartConfigError];
export declare class BLESmartConfig {
    uart: BLEUART;
    protected state: StateMachine<States>;
    protected onChange?: (prevState: States, currentState: States) => void | Promise<void>;
    transitions: (ITransition<"Disconnected" | "Connecting"> | ITransition<"Connecting" | "Connected"> | ITransition<"Connected" | "LoadingListSSID">)[];
    constructor(uart: BLEUART, onChange: (prevState: States, currentState: States) => void | Promise<void>);
    connect(): Promise<void>;
    list_ssid(): Promise<SSIDItem[]>;
    test_wifi_connection(): Promise<boolean>;
    set_wifi(ssid: string, passphrase: string): Promise<boolean>;
    set_awsiot(config: IoTConfig): Promise<boolean>;
    check_awsiot(): Promise<boolean>;
    send_text(key: string, value: string): Promise<boolean>;
    send_text_multiline(key: string, value: string): Promise<boolean>;
}
//# sourceMappingURL=index.d.ts.map