"use strict";
var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
var __generator = (this && this.__generator) || function (thisArg, body) {
    var _ = { label: 0, sent: function() { if (t[0] & 1) throw t[1]; return t[1]; }, trys: [], ops: [] }, f, y, t, g;
    return g = { next: verb(0), "throw": verb(1), "return": verb(2) }, typeof Symbol === "function" && (g[Symbol.iterator] = function() { return this; }), g;
    function verb(n) { return function (v) { return step([n, v]); }; }
    function step(op) {
        if (f) throw new TypeError("Generator is already executing.");
        while (_) try {
            if (f = 1, y && (t = op[0] & 2 ? y["return"] : op[0] ? y["throw"] || ((t = y["return"]) && t.call(y), 0) : y.next) && !(t = t.call(y, op[1])).done) return t;
            if (y = 0, t) op = [op[0] & 2, t.value];
            switch (op[0]) {
                case 0: case 1: t = op; break;
                case 4: _.label++; return { value: op[1], done: false };
                case 5: _.label++; y = op[1]; op = [0]; continue;
                case 7: op = _.ops.pop(); _.trys.pop(); continue;
                default:
                    if (!(t = _.trys, t = t.length > 0 && t[t.length - 1]) && (op[0] === 6 || op[0] === 2)) { _ = 0; continue; }
                    if (op[0] === 3 && (!t || (op[1] > t[0] && op[1] < t[3]))) { _.label = op[1]; break; }
                    if (op[0] === 6 && _.label < t[1]) { _.label = t[1]; t = op; break; }
                    if (t && _.label < t[2]) { _.label = t[2]; _.ops.push(op); break; }
                    if (t[2]) _.ops.pop();
                    _.trys.pop(); continue;
            }
            op = body.call(thisArg, _);
        } catch (e) { op = [6, e]; y = 0; } finally { f = t = 0; }
        if (op[0] & 5) throw op[1]; return { value: op[0] ? op[1] : void 0, done: true };
    }
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.BLESmartConfig = exports.BLESmartConfigError = exports.State = exports.IoTConfig = exports.SSIDItem = exports.BLEUART = void 0;
var fsm_1 = require("./fsm");
var ble_uart_1 = require("./ble-uart");
Object.defineProperty(exports, "BLEUART", { enumerable: true, get: function () { return ble_uart_1.BLEUART; } });
var SSIDItem = /** @class */ (function () {
    function SSIDItem(line) {
        var item = line.split(/,/, 3);
        this.authmode = parseInt(item[0], 10);
        this.rssi = parseInt(item[1], 10);
        this.ssid = item[2];
    }
    SSIDItem.prototype.isOpen = function () {
        return this.authmode === 0;
    };
    return SSIDItem;
}());
exports.SSIDItem = SSIDItem;
var IoTConfig = /** @class */ (function () {
    function IoTConfig() {
        this.mqtt_uri = "";
        this.mqtt_dev_topic = "";
        this.mqtt_res_topic = "";
        this.root_ca = "";
        this.cert = "";
        this.priv = "";
        this.client_id = "";
    }
    return IoTConfig;
}());
exports.IoTConfig = IoTConfig;
exports.State = {
    Disconnected: "Disconnected",
    Connecting: "Connecting",
    Connected: "Connected",
    Processing: "Processing",
};
exports.BLESmartConfigError = {
    Disconnected: "Disconnected",
    AlreadyConnected: "AlreadyConnected",
};
var BLESmartConfig = /** @class */ (function () {
    function BLESmartConfig(uart, onChange) {
        var _this = this;
        this.transitions = [
            fsm_1.tFrom(exports.State.Connected, exports.State.Disconnected),
            fsm_1.tFrom(exports.State.Connecting, exports.State.Disconnected),
            fsm_1.tFrom(exports.State.Disconnected, exports.State.Connecting),
            fsm_1.tFrom(exports.State.Disconnected, exports.State.Connected),
            fsm_1.tFrom(exports.State.Connecting, exports.State.Connected),
            fsm_1.tFrom(exports.State.Connected, exports.State.Processing),
            fsm_1.tFrom(exports.State.Processing, exports.State.Connected),
            fsm_1.tFrom(exports.State.Processing, exports.State.Disconnected),
        ];
        this.uart = uart;
        this.state = new fsm_1.StateMachine(exports.State.Disconnected, this.transitions, onChange);
        this.uart.addListener("disconnect", function () {
            return _this.state.moveTo(exports.State.Disconnected);
        });
    }
    BLESmartConfig.prototype.connect = function () {
        return __awaiter(this, void 0, void 0, function () {
            var e_1;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        if (!this.uart.isConnected()) return [3 /*break*/, 2];
                        return [4 /*yield*/, this.state.moveTo(exports.State.Connected)];
                    case 1:
                        _a.sent();
                        return [2 /*return*/, true];
                    case 2: return [4 /*yield*/, this.state.moveTo(exports.State.Connecting)];
                    case 3:
                        _a.sent();
                        _a.label = 4;
                    case 4:
                        _a.trys.push([4, 9, , 11]);
                        return [4 /*yield*/, this.uart.start()];
                    case 5:
                        if (!!(_a.sent())) return [3 /*break*/, 7];
                        return [4 /*yield*/, this.disconnect()];
                    case 6:
                        _a.sent();
                        return [2 /*return*/, false];
                    case 7: return [4 /*yield*/, this.state.moveTo(exports.State.Connected)];
                    case 8:
                        _a.sent();
                        return [3 /*break*/, 11];
                    case 9:
                        e_1 = _a.sent();
                        console.error(e_1);
                        return [4 /*yield*/, this.disconnect()];
                    case 10:
                        _a.sent();
                        return [2 /*return*/, false];
                    case 11: return [2 /*return*/, true];
                }
            });
        });
    };
    BLESmartConfig.prototype.disconnect = function () {
        return __awaiter(this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                try {
                    this.uart.disconnect();
                }
                catch (e) {
                    console.error(e);
                }
                finally {
                    this.state.moveTo(exports.State.Disconnected);
                }
                return [2 /*return*/];
            });
        });
    };
    BLESmartConfig.prototype.list_ssid = function () {
        return __awaiter(this, void 0, void 0, function () {
            var result, last_line, line, item, e_2;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        result = [];
                        return [4 /*yield*/, this.state.moveTo(exports.State.Processing)];
                    case 1:
                        _a.sent();
                        _a.label = 2;
                    case 2:
                        _a.trys.push([2, 8, , 10]);
                        this.uart.clear();
                        return [4 /*yield*/, this.uart.sendln("LIST_SSID")];
                    case 3:
                        _a.sent();
                        last_line = "";
                        _a.label = 4;
                    case 4:
                        if (!true) return [3 /*break*/, 6];
                        return [4 /*yield*/, this.uart.readline()];
                    case 5:
                        line = _a.sent();
                        if (line === "")
                            return [3 /*break*/, 6];
                        last_line = line;
                        item = new SSIDItem(line);
                        if (item.ssid !== "") {
                            result.push(item);
                        }
                        return [3 /*break*/, 4];
                    case 6:
                        result.pop();
                        if (!last_line.match(/^OK/))
                            throw new Error("device returned error");
                        return [4 /*yield*/, this.state.moveTo(exports.State.Connected)];
                    case 7:
                        _a.sent();
                        return [2 /*return*/, result];
                    case 8:
                        e_2 = _a.sent();
                        return [4 /*yield*/, this.disconnect()];
                    case 9:
                        _a.sent();
                        return [2 /*return*/, []];
                    case 10: return [2 /*return*/];
                }
            });
        });
    };
    BLESmartConfig.prototype.set_wifi = function (ssid, passphrase) {
        return __awaiter(this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0: return [4 /*yield*/, this.run_command("SET_WIFI " + JSON.stringify(ssid) + " " + JSON.stringify(passphrase))];
                    case 1: return [2 /*return*/, _a.sent()];
                }
            });
        });
    };
    BLESmartConfig.prototype.test_wifi_connection = function () {
        return __awaiter(this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0: return [4 /*yield*/, this.run_command("CHECK_WIFI")];
                    case 1: return [2 /*return*/, _a.sent()];
                }
            });
        });
    };
    BLESmartConfig.prototype.set_mqtt = function (config) {
        return __awaiter(this, void 0, void 0, function () {
            var e_3;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0: return [4 /*yield*/, this.state.moveTo(exports.State.Processing)];
                    case 1:
                        _a.sent();
                        _a.label = 2;
                    case 2:
                        _a.trys.push([2, 10, , 12]);
                        return [4 /*yield*/, this.send_text("mqtt_uri", config.mqtt_uri)];
                    case 3:
                        if (!(_a.sent()))
                            throw new Error("Error at mqtt_uri");
                        return [4 /*yield*/, this.send_text("mqtt_dev_topic", config.mqtt_dev_topic)];
                    case 4:
                        if (!(_a.sent()))
                            throw new Error("Error at mqtt_dev_topic");
                        return [4 /*yield*/, this.send_text("mqtt_res_topic", config.mqtt_res_topic)];
                    case 5:
                        if (!(_a.sent()))
                            throw new Error("Error at mqtt_res_topic");
                        return [4 /*yield*/, this.send_text_multiline("mqtt_root_ca", config.root_ca)];
                    case 6:
                        if (!(_a.sent()))
                            throw new Error("Error at root_ca");
                        return [4 /*yield*/, this.send_text_multiline("mqtt_cert", config.cert)];
                    case 7:
                        if (!(_a.sent()))
                            throw new Error("Error at mqtt_cert");
                        return [4 /*yield*/, this.send_text_multiline("mqtt_priv", config.priv)];
                    case 8:
                        if (!(_a.sent()))
                            throw new Error("Error at mqtt_priv");
                        return [4 /*yield*/, this.state.moveTo(exports.State.Connected)];
                    case 9:
                        _a.sent();
                        return [3 /*break*/, 12];
                    case 10:
                        e_3 = _a.sent();
                        return [4 /*yield*/, this.disconnect()];
                    case 11:
                        _a.sent();
                        return [2 /*return*/, false];
                    case 12: return [2 /*return*/, true];
                }
            });
        });
    };
    BLESmartConfig.prototype.restart = function () {
        return __awaiter(this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0: return [4 /*yield*/, this.run_command("RESTART")];
                    case 1: return [2 /*return*/, _a.sent()];
                }
            });
        });
    };
    BLESmartConfig.prototype.check_mqtt = function () {
        return __awaiter(this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0: return [4 /*yield*/, this.run_command("CHECK_MQTT")];
                    case 1: return [2 /*return*/, _a.sent()];
                }
            });
        });
    };
    BLESmartConfig.prototype.device_id = function () {
        return __awaiter(this, void 0, void 0, function () {
            var result, e_4;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0: return [4 /*yield*/, this.state.moveTo(exports.State.Processing)];
                    case 1:
                        _a.sent();
                        _a.label = 2;
                    case 2:
                        _a.trys.push([2, 7, , 9]);
                        this.uart.clear();
                        return [4 /*yield*/, this.uart.sendln("GET_STR device_id")];
                    case 3:
                        _a.sent();
                        return [4 /*yield*/, this.uart.readline()];
                    case 4:
                        result = _a.sent();
                        return [4 /*yield*/, this.uart.waitBlank()];
                    case 5:
                        _a.sent();
                        return [4 /*yield*/, this.state.moveTo(exports.State.Connected)];
                    case 6:
                        _a.sent();
                        return [2 /*return*/, result];
                    case 7:
                        e_4 = _a.sent();
                        console.error(e_4);
                        return [4 /*yield*/, this.disconnect()];
                    case 8:
                        _a.sent();
                        return [2 /*return*/, ""];
                    case 9: return [2 /*return*/];
                }
            });
        });
    };
    BLESmartConfig.prototype.check_ok = function () {
        return __awaiter(this, void 0, void 0, function () {
            var result;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0: return [4 /*yield*/, this.uart.readline()];
                    case 1:
                        result = _a.sent();
                        return [4 /*yield*/, this.uart.waitBlank()];
                    case 2:
                        _a.sent();
                        return [2 /*return*/, !!result.match(/^OK/)];
                }
            });
        });
    };
    BLESmartConfig.prototype.run_command = function (command) {
        return __awaiter(this, void 0, void 0, function () {
            var result, e_5;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0: return [4 /*yield*/, this.state.moveTo(exports.State.Processing)];
                    case 1:
                        _a.sent();
                        _a.label = 2;
                    case 2:
                        _a.trys.push([2, 6, , 8]);
                        this.uart.clear();
                        return [4 /*yield*/, this.uart.sendln(command)];
                    case 3:
                        _a.sent();
                        return [4 /*yield*/, this.check_ok()];
                    case 4:
                        result = _a.sent();
                        return [4 /*yield*/, this.state.moveTo(exports.State.Connected)];
                    case 5:
                        _a.sent();
                        return [2 /*return*/, result];
                    case 6:
                        e_5 = _a.sent();
                        console.error(e_5);
                        return [4 /*yield*/, this.disconnect()];
                    case 7:
                        _a.sent();
                        return [2 /*return*/, false];
                    case 8: return [2 /*return*/];
                }
            });
        });
    };
    BLESmartConfig.prototype.send_text = function (key, value) {
        return __awaiter(this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0: return [4 /*yield*/, this.uart.sendln("SET_STR " + key + " " + JSON.stringify(value))];
                    case 1:
                        _a.sent();
                        return [4 /*yield*/, this.check_ok()];
                    case 2: return [2 /*return*/, _a.sent()];
                }
            });
        });
    };
    BLESmartConfig.prototype.send_text_multiline = function (key, value) {
        return __awaiter(this, void 0, void 0, function () {
            var lines, _i, lines_1, line;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        lines = value.replaceAll(/\r/g, "").replace(/\n+$/, "").split(/\n+/);
                        return [4 /*yield*/, this.uart.sendln("SET_MULTI " + key + " " + lines.length)];
                    case 1:
                        _a.sent();
                        _i = 0, lines_1 = lines;
                        _a.label = 2;
                    case 2:
                        if (!(_i < lines_1.length)) return [3 /*break*/, 5];
                        line = lines_1[_i];
                        return [4 /*yield*/, this.uart.sendln(line)];
                    case 3:
                        _a.sent();
                        _a.label = 4;
                    case 4:
                        _i++;
                        return [3 /*break*/, 2];
                    case 5: return [4 /*yield*/, this.check_ok()];
                    case 6: return [2 /*return*/, _a.sent()];
                }
            });
        });
    };
    return BLESmartConfig;
}());
exports.BLESmartConfig = BLESmartConfig;
//# sourceMappingURL=index.js.map