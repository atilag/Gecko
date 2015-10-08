"use strict";

const DEBUG = true;
function debug(s) {
  if (DEBUG) dump("-*- VZWLoginAPI: " + s + "\n");
}

const Cc = Components.classes;
const Ci = Components.interfaces;
const Cu = Components.utils;

Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/Services.jsm");
Cu.import("resource://gre/modules/DOMRequestHelper.jsm");

XPCOMUtils.defineLazyServiceGetter(this, "cpmm",
                                   "@mozilla.org/childprocessmessagemanager;1",
                                   "nsIMessageSender");

function VZWLoginAPI() {
  debug("Constructor");
}

VZWLoginAPI.prototype = {
  __proto__: DOMRequestIpcHelper.prototype,

  classDescription: "VZWLoginAPI",
  classID: Components.ID("{a0fc01a0-5aa2-11e5-a837-0800200c9a66}"),
  contractID: "@mozilla.org/dom/vzw-login-api;1",

  QueryInterface: XPCOMUtils.generateQI([Ci.nsIDOMGlobalPropertyInitializer,
                                         Ci.nsISupportsWeakReference,
                                         Ci.nsIObserver]),

  init: function(aWindow) {
    this._window = aWindow;

    this.initDOMRequestHelper(aWindow, ["VZWLogin:GetToken:Result:OK",
                                        "VZWLogin:GetToken:Result:KO",
                                        "VZWLogin:DeleteToken:Result:OK",
                                        "VZWLogin:DeleteToken:Result:KO",
                                        "VZWLogin:GetAuthFromSim:Result:OK",
                                        "VZWLogin:GetAuthFromSim:Result:KO"]);

    cpmm.sendAsyncMessage("VZWLogin:Register", null);
  },

  uninit: function() {
    cpmm.sendAsyncMessage("VZWLogin:Unregister", null);
  },

  getToken: function getToken() {
    debug("getToken");
    let promiseInit = function(resolve, reject) {
      debug("promise init called for getToken");
      let resolverId = this.getPromiseResolverId({resolve: resolve,
                                                  reject: reject });
      debug("promise init " + resolverId);
      cpmm.sendAsyncMessage("VZWLogin:GetToken", {
        requestId: resolverId
      });
    }.bind(this);

    return this.createPromise(promiseInit);
  },

  deleteToken: function deleteToken() {
    debug("deleteToken");
    let promiseInit = function(resolve, reject) {
      debug("promise init called for deleteToken");
      let resolverId = this.getPromiseResolverId({resolve: resolve,
                                                  reject: reject });
      debug("promise init " + resolverId);
      cpmm.sendAsyncMessage("VZWLogin:DeleteToken", {
        requestId: resolverId
      });
    }.bind(this);

    return this.createPromise(promiseInit);
  },

  // temporary test. it will be deleted
  getAuthFromSim: function getAuthFromSim(aAuthData) {
    debug("getAuthFromSim");
    let promiseInit = function(resolve, reject) {
      debug("promise init called for GetAuthFromSim " + aAuthData);
      let resolverId = this.getPromiseResolverId({resolve: resolve,
                                                  reject: reject });
      debug("promise init " + resolverId);
      cpmm.sendAsyncMessage("VZWLogin:GetAuthFromSim", {
        requestId: resolverId,
        authData: aAuthData
      });
    }.bind(this);

    return this.createPromise(promiseInit);
  },

  set onmessage(aHandler) {
    this.__DOM_IMPL__.setEventHandler("onmessage", aHandler);
  },

  get onmessage() {
    return this.__DOM_IMPL__.getEventHandler("onmessage");
  },

  receiveMessage: function(aMessage) {
    debug("receiveMessage: name: " + aMessage.name);
    let resolver = null;
    let data = aMessage.data;

    switch (aMessage.name) {
      case "VZWLogin:OnMessage":
        let detail = Cu.cloneInto(data, this._window);
        let event = new this._window.CustomEvent("message", {"detail": detail});
        this.__DOM_IMPL__.dispatchEvent(event);
        break;
      case "VZWLogin:GetToken:Result:OK":
      case "VZWLogin:DeleteToken:Result:OK":
      case "VZWLogin:GetAuthFromSim:Result:OK":
        resolver = this.takePromiseResolver(data.requestId);
        if (!resolver) {
          return;
        }
        let value = Cu.cloneInto(data.value, this._window);
        resolver.resolve(value);
        break;
      case "VZWLogin:GetToken:Result:KO":
      case "VZWLogin:DeleteToken:Result:KO":
      case "VZWLogin:GetAuthFromSim:Result:KO":
        resolver = this.takePromiseResolver(data.requestId);
        if (!resolver) {
          return;
        }
        resolver.reject(data.reason);
        break;
    }
  }
}

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([VZWLoginAPI]);
