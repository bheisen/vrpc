/*
__/\\\________/\\\____/\\\\\\\\\______/\\\\\\\\\\\\\_________/\\\\\\\\\_
__\/\\\_______\/\\\__/\\\///////\\\___\/\\\/////////\\\____/\\\////////__
 __\//\\\______/\\\__\/\\\_____\/\\\___\/\\\_______\/\\\__/\\\/___________
  ___\//\\\____/\\\___\/\\\\\\\\\\\/____\/\\\\\\\\\\\\\/__/\\\_____________
   ____\//\\\__/\\\____\/\\\//////\\\____\/\\\/////////___\/\\\_____________
    _____\//\\\/\\\_____\/\\\____\//\\\___\/\\\____________\//\\\____________
     ______\//\\\\\______\/\\\_____\//\\\__\/\\\_____________\///\\\__________
      _______\//\\\_______\/\\\______\//\\\_\/\\\_______________\////\\\\\\\\\_
       ________\///________\///________\///__\///___________________\/////////__


Non-intrusively binds code and provides access in form of asynchronous remote
procedure calls (RPC).
Author: Dr. Burkhard C. Heisen (https://github.com/bheisen/vrpc)


Licensed under the MIT License <http://opensource.org/licenses/MIT>.
Copyright (c) 2018 - 2019 Dr. Burkhard C. Heisen <burkhard.heisen@xsmail.com>.

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

const EventEmitter = require('events')
const crypto = require('crypto')

class VrpcLocal {

  constructor (adapter) {
    this._eventEmitter = new EventEmitter()
    this._invokeId = 0
    this._adapter = adapter

    // Register callback handler
    this._adapter.onCallback((jsonString, jsonObject) => {
      // Only JS Adapters will provide the second argument
      const { id, data } = jsonObject || JSON.parse(jsonString)
      this._eventEmitter.emit(id, data)
    })
  }

  create (className, ...args) {
    if (typeof className === 'string') {
      return this._create({ className, args })
    }
    return this._create({
      className: className.className,
      instance: className.instance,
      args: className.args
    })
  }

  getInstance ({ className, instance }) {
    const json = {
      targetId: className,
      method: '__getNamed__',
      data: { _1: instance }
    }
    return this._createProxy(className, json)
  }

  callStatic (className, functionName, ...args) {
    const json = {
      targetId: className,
      method: functionName,
      data: this._packData(className, functionName, ...args)
    }
    return JSON.parse(this._adapter.call(JSON.stringify(json))).data.r
  }

  // private:

  _create ({ className, instance, args = [] }) {
    const data = instance ? { _1: instance } : {}
    const offset = instance ? 2 : 1
    args.forEach((value, index) => {
      data[`_${index + offset}`] = value
    })
    const json = {
      targetId: className,
      method: instance ? '__createNamed__' : '__create__',
      data
    }
    return this._createProxy(className, json)
  }

  _createProxy (className, json) {
    // Create instance
    const ret = JSON.parse(this._adapter.call(JSON.stringify(json)))
    const instanceId = ret.data.r
    const proxyId = crypto.randomBytes(2).toString('hex')
    const proxy = {
      _targetId: instanceId,
      _proxyId: proxyId
    }
    let functions = JSON.parse(this._adapter.getMemberFunctions(className)).functions
    // Strip off argument signature
    functions = functions.map(name => {
      const pos = name.indexOf('-')
      if (pos > 0) return name.substring(0, pos)
      else return name
    })
    // Remove overloads
    const uniqueFuncs = new Set(functions)
    // Build proxy
    uniqueFuncs.forEach(name => {
      proxy[name] = (...args) => {
        const json = {
          targetId: instanceId,
          method: name,
          data: this._packData(proxyId, name, ...args)
        }
        const { data } = JSON.parse(this._adapter.call(JSON.stringify(json)))
        if (data.e) throw new Error(data.e)
        // Handle functions returning a promise
        if (typeof data.r === 'string' && data.r.substr(0, 5) === '__p__') {
          return new Promise((resolve, reject) => {
            this._eventEmitter.once(data.r, promiseData => {
              if (promiseData.e) reject(new Error(promiseData.e))
              else resolve(promiseData.r)
            })
          })
        }
        return data.r
      }
    })
    return proxy
  }

  _packData (proxyId, functionName, ...args) {
    const data = {}
    args.forEach((value, index) => {
      // Check whether provided argument is a function
      if (this._isFunction(value)) {
        const id = `__f__${proxyId}-${functionName}-${index}-${this._invokeId++ % Number.MAX_SAFE_INTEGER}`
        data[`_${index + 1}`] = id
        this._eventEmitter.once(id, data => {
          const args = Object.keys(data).sort()
            .filter(value => value[0] === '_')
            .map(key => data[key])
          value.apply(null, args) // This is the actual function call
        })
      } else if (this._isEmitter(value)) {
        const { emitter, event } = value
        const id = `__f__${proxyId}-${functionName}-${index}-${event}`
        data[`_${index + 1}`] = id
        this._eventEmitter.on(id, data => {
          const args = Object.keys(data).sort()
            .filter(value => value[0] === '_')
            .map(key => data[key])
          emitter.emit(event, ...args)
        })
      } else {
        data[`_${index + 1}`] = value
      }
    })
    return data
  }

  _isFunction (variable) {
    const getType = {}
    return variable && getType.toString.call(variable) === '[object Function]'
  }

  _isEmitter (variable) {
    return (
      typeof variable === 'object' &&
      variable.hasOwnProperty('emitter') &&
      variable.hasOwnProperty('event') &&
      typeof variable.emitter === 'object' &&
      typeof variable.emitter.emit === 'function'
    )
  }
}

module.exports = VrpcLocal
