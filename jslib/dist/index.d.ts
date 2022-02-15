import { StateMachine } from "./fsm";
import { BLEUART } from "./ble-uart";
export { BLEUART };
export declare class SSIDItem {
    ssid: string;
    rssi: number;
    authmode: number;
    constructor(line: string);
    isOpen(): boolean;
}
export declare class IoTConfig {
    mqtt_uri: string;
    mqtt_device_topic: string;
    mqtt_response_topic: string;
    root_ca: string;
    cert: string;
    priv: string;
    client_id: string;
}
export declare const State: {
    readonly Disconnected: "Disconnected";
    readonly Connecting: "Connecting";
    readonly Connected: "Connected";
    readonly Processing: "Processing";
};
export declare type States = typeof State[keyof typeof State];
export declare const BLESmartConfigError: {
    readonly Disconnected: "Disconnected";
    readonly AlreadyConnected: "AlreadyConnected";
};
export declare type BLESmartConfigErrors = typeof BLESmartConfigError[keyof typeof BLESmartConfigError];
export declare class BLESmartConfig {
    private uart;
    state: StateMachine<States>;
    onChange?: (prevState: States, currentState: States) => void | Promise<void>;
    transitions: (import("./fsm").ITransition<"Disconnected" | "Connected"> | import("./fsm").ITransition<"Disconnected" | "Connecting"> | import("./fsm").ITransition<"Connecting" | "Connected"> | import("./fsm").ITransition<"Connected" | "Processing"> | import("./fsm").ITransition<"Disconnected" | "Processing">)[];
    constructor(uart: BLEUART, onChange?: (prevState: States, currentState: States) => void | Promise<void>);
    connect(): Promise<boolean>;
    disconnect(): Promise<void>;
    list_ssid(): Promise<SSIDItem[]>;
    set_wifi(ssid: string, passphrase: string): Promise<boolean>;
    test_wifi_connection(): Promise<boolean>;
    set_mqtt(config: IoTConfig): Promise<boolean>;
    restart(): Promise<boolean>;
    check_mqtt(): Promise<boolean>;
    device_id(): Promise<string>;
    private check_ok;
    private run_command;
    private send_text;
    private send_text_multiline;
}
//# sourceMappingURL=index.d.ts.map