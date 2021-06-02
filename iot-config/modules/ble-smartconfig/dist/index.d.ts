import { BLEUART } from "./ble-uart";
export { BLEUART } from "./ble-uart";
export declare class BLESmartConfig {
    uart: BLEUART;
    constructor(uart: BLEUART);
    ssids(): Promise<string[]>;
}
//# sourceMappingURL=index.d.ts.map