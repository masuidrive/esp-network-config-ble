/*
 * StateMachine.ts
 * TypeScript finite state machine class with async transformations using promises.
 */

// tslint:disable:no-any

export interface ITransition<STATE> {
  fromState: STATE
  toState: STATE
  cb?: (...args: any[]) => void | Promise<void>
}

export function tFrom<STATE>(
  fromState: STATE,
  toState: STATE,
  cb?: (...args: any[]) => void | Promise<void>
): ITransition<STATE> {
  return { fromState, toState, cb }
}

export class StateMachine<STATE> {
  protected current: STATE
  protected transitions: Array<ITransition<STATE>>
  protected onChange?: (
    prevState: STATE,
    currentState: STATE
  ) => void | Promise<void>

  // initalize the state-machine
  constructor(
    initState: STATE,
    transitions: Array<ITransition<STATE>> = [],
    onChange?: (prevState: STATE, currentState: STATE) => void | Promise<void>
  ) {
    this.current = initState
    this.transitions = transitions
    this.onChange = onChange
  }

  addTransitions(transitions: Array<ITransition<STATE>>): void {
    transitions.forEach((tran) => this.transitions.push(tran))
  }

  getState(): STATE {
    return this.current
  }

  isFinal(): boolean {
    // search for a transition that starts from current state.
    // if none is found it's a terminal state.
    for (const trans of this.transitions) {
      if (trans.fromState === this.current) {
        return false
      }
    }

    return true
  }

  canMove(toState: STATE[]): boolean {
    for (const state of toState as STATE[]) {
      if (this.current === state) return true
      for (const tran of this.transitions) {
        if (tran.fromState === this.current && tran.toState === state)
          return true
      }
    }

    return false
  }

  guard(toState: STATE[]) {
    for (const state of toState as STATE[]) {
      if (this.current === state) return true
    }
    throw new Error(`Can't move ${this.current} to ${toState}.`)
  }

  // post event asynch
  async moveTo(toState: STATE, ...args: any[]): Promise<void> {
    // delay execution to make it async
    if (this.current === toState) return

    let found = false
    const prevState = this.current

    // find transition
    for (const tran of this.transitions) {
      if (tran.fromState === this.current && tran.toState === toState) {
        const prevState = this.current
        this.current = tran.toState
        found = true
        if (tran.cb) {
          try {
            await tran.cb(args)
          } catch (e) {
            throw new Error("Exception caught in callback")
          }
          try {
            if (this.onChange && prevState !== this.current)
              await this.onChange(prevState, this.current)
          } catch (e) {
            throw new Error("Exception caught in onChange callback")
          }
        } else {
          try {
            if (this.onChange) await this.onChange(prevState, this.current)
          } catch (e) {
            throw new Error("Exception caught in onChange callback")
          }
        }
        break
      }
    }

    // no such transition
    if (!found) {
      throw new Error(`no such transition: from ${this.current} to ${toState}`)
    }
    console.log(`${prevState} to ${this.current}`)
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
