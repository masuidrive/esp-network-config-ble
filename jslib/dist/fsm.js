"use strict";
/*
 * StateMachine.ts
 * TypeScript finite state machine class with async transformations using promises.
 */
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
exports.StateMachine = exports.tFrom = void 0;
function tFrom(fromState, toState, cb) {
    return { fromState: fromState, toState: toState, cb: cb };
}
exports.tFrom = tFrom;
var StateMachine = /** @class */ (function () {
    // initalize the state-machine
    function StateMachine(initState, transitions, onChange) {
        if (transitions === void 0) { transitions = []; }
        this.current = initState;
        this.transitions = transitions;
        this.onChange = onChange;
    }
    StateMachine.prototype.addTransitions = function (transitions) {
        var _this = this;
        transitions.forEach(function (tran) { return _this.transitions.push(tran); });
    };
    StateMachine.prototype.getState = function () {
        return this.current;
    };
    StateMachine.prototype.isFinal = function () {
        // search for a transition that starts from current state.
        // if none is found it's a terminal state.
        for (var _i = 0, _a = this.transitions; _i < _a.length; _i++) {
            var trans = _a[_i];
            if (trans.fromState === this.current) {
                return false;
            }
        }
        return true;
    };
    StateMachine.prototype.canMove = function (toState) {
        for (var _i = 0, _a = toState; _i < _a.length; _i++) {
            var state = _a[_i];
            if (this.current === state)
                return true;
            for (var _b = 0, _c = this.transitions; _b < _c.length; _b++) {
                var tran = _c[_b];
                if (tran.fromState === this.current && tran.toState === state)
                    return true;
            }
        }
        return false;
    };
    StateMachine.prototype.guard = function (toState) {
        for (var _i = 0, _a = toState; _i < _a.length; _i++) {
            var state = _a[_i];
            if (this.current === state)
                return true;
        }
        throw new Error("Can't move " + this.current + " to " + toState + ".");
    };
    // post event asynch
    StateMachine.prototype.moveTo = function (toState) {
        var args = [];
        for (var _i = 1; _i < arguments.length; _i++) {
            args[_i - 1] = arguments[_i];
        }
        return __awaiter(this, void 0, void 0, function () {
            var found, prevState, _a, _b, tran, prevState_1, e_1, e_2, e_3;
            return __generator(this, function (_c) {
                switch (_c.label) {
                    case 0:
                        // delay execution to make it async
                        if (this.current === toState)
                            return [2 /*return*/];
                        found = false;
                        prevState = this.current;
                        _a = 0, _b = this.transitions;
                        _c.label = 1;
                    case 1:
                        if (!(_a < _b.length)) return [3 /*break*/, 16];
                        tran = _b[_a];
                        if (!(tran.fromState === this.current && tran.toState === toState)) return [3 /*break*/, 15];
                        prevState_1 = this.current;
                        this.current = tran.toState;
                        found = true;
                        if (!tran.cb) return [3 /*break*/, 10];
                        _c.label = 2;
                    case 2:
                        _c.trys.push([2, 4, , 5]);
                        return [4 /*yield*/, tran.cb(args)];
                    case 3:
                        _c.sent();
                        return [3 /*break*/, 5];
                    case 4:
                        e_1 = _c.sent();
                        throw new Error("Exception caught in callback");
                    case 5:
                        _c.trys.push([5, 8, , 9]);
                        if (!(this.onChange && prevState_1 !== this.current)) return [3 /*break*/, 7];
                        return [4 /*yield*/, this.onChange(prevState_1, this.current)];
                    case 6:
                        _c.sent();
                        _c.label = 7;
                    case 7: return [3 /*break*/, 9];
                    case 8:
                        e_2 = _c.sent();
                        throw new Error("Exception caught in onChange callback");
                    case 9: return [3 /*break*/, 14];
                    case 10:
                        _c.trys.push([10, 13, , 14]);
                        if (!this.onChange) return [3 /*break*/, 12];
                        return [4 /*yield*/, this.onChange(prevState_1, this.current)];
                    case 11:
                        _c.sent();
                        _c.label = 12;
                    case 12: return [3 /*break*/, 14];
                    case 13:
                        e_3 = _c.sent();
                        throw new Error("Exception caught in onChange callback");
                    case 14: return [3 /*break*/, 16];
                    case 15:
                        _a++;
                        return [3 /*break*/, 1];
                    case 16:
                        // no such transition
                        if (!found) {
                            throw new Error("no such transition: from " + this.current + " to " + toState);
                        }
                        console.log(prevState + " to " + this.current);
                        return [2 /*return*/];
                }
            });
        });
    };
    return StateMachine;
}());
exports.StateMachine = StateMachine;
/** Original license
https://github.com/eram/ts-fsm
* MIT License

Copyright (c) 2018 eram

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */
//# sourceMappingURL=fsm.js.map