export interface ITransition<STATE> {
    fromState: STATE;
    toState: STATE;
    cb?: (...args: any[]) => void | Promise<void>;
}
export declare function tFrom<STATE>(fromState: STATE, toState: STATE, cb?: (...args: any[]) => void | Promise<void>): ITransition<STATE>;
export declare class StateMachine<STATE> {
    protected current: STATE;
    protected transitions: Array<ITransition<STATE>>;
    protected onChange?: (prevState: STATE, currentState: STATE) => void | Promise<void>;
    constructor(initState: STATE, transitions: ITransition<STATE>[] | undefined, onChange: (prevState: STATE, currentState: STATE) => void | Promise<void>);
    addTransitions(transitions: Array<ITransition<STATE>>): void;
    getState(): STATE;
    isFinal(): boolean;
    canMove(toState: STATE | STATE[]): boolean;
    guard(toState: STATE | STATE[]): void;
    moveTo(toState: STATE, ...args: any[]): Promise<void>;
}
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
//# sourceMappingURL=fsm.d.ts.map