/*
 * StateMachine.ts
 * TypeScript finite state machine class with async transformations using promises.
 */

// tslint:disable:no-any

export interface ITransition<STATE> {
  fromState: STATE;
  toState: STATE;
  cb?: (...args: any[]) => void | Promise<void>;
}

export function tFrom<STATE>(
  fromState: STATE,
  toState: STATE,
  cb?: (...args: any[]) => void | Promise<void>
): ITransition<STATE> {
  return { fromState, toState, cb };
}

export class StateMachine<STATE> {
  protected current: STATE;
  protected transitions: Array<ITransition<STATE>>;
  protected onChange?: (
    prevState: STATE,
    currentState: STATE
  ) => void | Promise<void>;

  // initalize the state-machine
  constructor(
    initState: STATE,
    transitions: Array<ITransition<STATE>> = [],
    onChange: (prevState: STATE, currentState: STATE) => void | Promise<void>
  ) {
    this.current = initState;
    this.transitions = transitions;
    this.onChange = onChange;
  }

  addTransitions(transitions: Array<ITransition<STATE>>): void {
    transitions.forEach((tran) => this.transitions.push(tran));
  }

  getState(): STATE {
    return this.current;
  }

  isFinal(): boolean {
    // search for a transition that starts from current state.
    // if none is found it's a terminal state.
    for (const trans of this.transitions) {
      if (trans.fromState === this.current) {
        return false;
      }
    }

    return true;
  }

  canMove(toState: STATE | STATE[]): boolean {
    let toStates: STATE[] = (
      typeof toState === "string" ? [toState] : toState
    ) as STATE[];

    for (const state of toState as STATE[]) {
      if (this.current === state) return true;
      for (const tran of this.transitions) {
        if (tran.fromState === this.current && tran.toState === state)
          return true;
      }
    }

    return false;
  }

  guard(toState: STATE | STATE[]) {
    if (!this.canMove(toState))
      throw new Error(`Can't move ${this.current} to ${toState}.`);
  }

  // post event asynch
  moveTo(toState: STATE, ...args: any[]): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      // delay execution to make it async
      setTimeout(
        async (me: this) => {
          if (me.current === toState) return;

          let found = false;

          // find transition
          for (const tran of me.transitions) {
            if (tran.fromState === me.current && tran.toState === toState) {
              const prevState = me.current;
              me.current = tran.toState;
              found = true;
              if (tran.cb) {
                try {
                  await tran.cb(args);
                } catch (e) {
                  return reject("Exception caught in callback");
                }
                try {
                  if (me.onChange) await me.onChange(prevState, me.current);
                } catch (e) {
                  return reject("Exception caught in onChange callback");
                }
                resolve();
              } else {
                try {
                  if (me.onChange) await me.onChange(prevState, me.current);
                } catch (e) {
                  return reject("Exception caught in onChange callback");
                }
                resolve();
              }
              break;
            }
          }

          // no such transition
          if (!found) {
            reject(`no such transition: from ${me.current} to ${toState}`);
          }
        },
        1,
        this
      );
    });
  }
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
