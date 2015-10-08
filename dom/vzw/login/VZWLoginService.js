"use strict";

const DEBUG = true;
function debug(s) {
  if (DEBUG) dump("-*- VZWLoginService: " + s + "\n");
}

const Cc = Components.classes;
const Ci = Components.interfaces;
const Cu = Components.utils;

Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/Services.jsm");
Cu.import("resource://gre/modules/Promise.jsm");

XPCOMUtils.defineLazyServiceGetter(this, "ppmm",
                                   "@mozilla.org/parentprocessmessagemanager;1",
                                   "nsIMessageBroadcaster");

XPCOMUtils.defineLazyServiceGetter(this, "gRIL",
                                   "@mozilla.org/ril;1",
                                   "nsIRadioInterfaceLayer");

XPCOMUtils.defineLazyServiceGetter(this, "gTelephonyService",
                                   "@mozilla.org/telephony/telephonyservice;1",
                                   "nsITelephonyService");

XPCOMUtils.defineLazyServiceGetter(this, "gIccProvider",
                                   "@mozilla.org/ril/content-helper;1",
                                   "nsIIccProvider");

XPCOMUtils.defineLazyServiceGetter(this, "gIccAuthenticator",
                                   "@mozilla.org/ril/content-helper;1",
                                   "nsIIccAuthenticator");

const PROD_SPC_URI_STR = "https://spc.myvzw.com/ium-RoadrunnerInterface/servlet/IUMServlet";
const TEST_SPC_URI_STR = "https://spctest.vzw.com:8181/ium-RoadrunnerInterface/servlet/IUMServlet";
const DEV_SIM_HTTP = "http://testproto.vzw.mcore.com/eap/IUMServlet.aspx";
const DEV_SIM_HTTPS = "https://testproto.azurewebsites.net/eap/IUMServlet.aspx";

const EAP_SUBTYPE_IDENTITY = 0x01;
const EAP_TYPE_REQUEST = 0x02;

const PREFERRED_CLIENTID = 0; // Choose Slot#1.

function VZWLoginService() {
  this.doingAuth = false;
  this.tasks = [];
  this.token = null;
  this.gRequestId = 1;
}

VZWLoginService.prototype = {

  classDescription: "VZWLoginService",
  classID: Components.ID("{c163e7a0-5aa2-11e5-a837-0800200c9a66}"),
  contractID: "@mozilla.org/vzw-login-service;1",
  QueryInterface: XPCOMUtils.generateQI([Ci.nsIVZWLoginService,
                                         Ci.nsIObserver]),

  observe: function(aSubject, aTopic, aData) {
    debug("-- init");

    switch(aTopic) {
      case "profile-after-change":
        Services.obs.addObserver(this, "xpcom-shutdown", false);
        ppmm.addMessageListener("VZWLogin:Register", this);
        ppmm.addMessageListener("VZWLogin:Unregister", this);
        ppmm.addMessageListener("VZWLogin:GetToken", this);
        ppmm.addMessageListener("VZWLogin:DeleteToken", this);
        ppmm.addMessageListener("VZWLogin:GetAuthFromSim", this);
        this._clients = [];
        break;
      case "xpcom-shutdown":
        Services.obs.removeObserver(this, "xpcom-shutdown");
        ppmm.removeMessageListener("VZWLogin:Register", this);
        ppmm.removeMessageListener("VZWLogin:Unregister", this);
        ppmm.removeMessageListener("VZWLogin:GetToken", this);
        ppmm.removeMessageListener("VZWLogin:DeleteToken", this);
        ppmm.removeMessageListener("VZWLogin:GetAuthFromSim", this);
        this._clients = null;
        break;
    }
  },

  receiveMessage: function(aMessage) {
    debug("receiveMessage: name: " + aMessage.name);

    // TODO : Check app permission for VZW's APP
    //if (!aMessage.target.assertPermission("vzw-login")) {
    //  debug(aMessage.name + " from a content process with no 'vzw-login' privileges.");
    //  return;
    //}

    switch (aMessage.name) {
      case "VZWLogin:Register":
        this._clients.push(aMessage.target);
        break;

      case "VZWLogin:Unregister":
        let index = this._clients.indexOf(aMessage.target);
        if (index > -1) {
          this._clients.splice(index, 1);
        }
        break;

      case "VZWLogin:GetToken":
        this.getToken().then(aValue => {
          aMessage.target.sendAsyncMessage("VZWLogin:GetToken:Result:OK", {
            requestId: aMessage.data.requestId,
            value: aValue
          });
        }, aError => {
          aMessage.target.sendAsyncMessage("VZWLogin:GetToken:Result:KO", {
            requestId: aMessage.data.requestId,
            reason: aError
          });
        });
        break;

      case "VZWLogin:DeleteToken":
        let _token = this.token;
        this.token = null;
        aMessage.target.sendAsyncMessage("VZWLogin:DeleteToken:Result:OK", {
          requestId: aMessage.data.requestId,
          value: _token
        });
        break;

      case "VZWLogin:GetAuthFromSim":
        this.lgeEapAkaAuth(aMessage.data.authData).then(aResponse => {
          aMessage.target.sendAsyncMessage("VZWLogin:GetAuthFromSim:Result:OK", {
            requestId: aMessage.data.requestId,
            value: aResponse
          });
        }, aReason => {
          aMessage.target.sendAsyncMessage("VZWLogin:GetAuthFromSim:Result:KO", {
            requestId: aMessage.data.requestId,
            reason: aReason
          });
        });
        break;
    }
  },

  getToken: function() {
    debug("getToken()");
    if (this.token) {
      return Promise.resolve(this.token);
    }

    let defer = {};

    this.tasks.push(defer);

    let promise = new Promise(function(resolve, reject) {
      defer.resolve = resolve;
      defer.reject = reject;
    });

    this.doAuth();

    return promise;
  },

  doAuth: function() {
    debug("doAuth()");
    if (this.doingAuth) {
      return;
    }
    this.doingAuth = true;

    // TODO: Check LTE
    if (gRIL.numRadioInterfaces < 1) {
      this.taskResult(false, "No Sim");
      return;
    }

    this.ril = gRIL.getRadioInterface(0);

    this.authStep1().then(aAuthData => {
      return this.authStep2(aAuthData);
    }).then(aSimResp => {
      return this.authStep3(aSimResp);
    }).then(aToken => {
      this.token = aToken;
      this.taskResult(true, aToken);
    }).catch(aReason => {
      this.taskResult(false, aReason);
    });
  },

  taskResult: function(aResolved, aResult) {
    debug("taskResult()");
    let _tasks = this.tasks;
    this.tasks = [];
    this.doingAuth = false;

    _tasks.forEach(defer => {
      if (aResolved) {
        defer.resolve(aResult);
      } else {
        defer.reject(aResult);
      }
    });
  },

  authStep1: function() {
    debug("authStep1()");
    return new Promise((aResolve, aReject) => {
      this.buildGetAuthBody().then(postBody => {
        return this.compressBody(postBody);
      }).then(aData => {
        return this.callSpc(aData.dataStr, aData.dataLength);
      }).then(aData => {
        let arrBuff = this.processResponse(aData);
        if (!arrBuff) {
          return Promise.reject("arrBuff is null");
        }
        let authData = this.handleChallenge(arrBuff);
        if (!authData) {
          return Promise.reject("authData is null");
        }
        debug("Step1:OK Result=" + authData);
        aResolve(authData);
      }).catch(aReason => {
        debug("Step1:KO aReason=" + aReason);
        aReject(aReason);
      });
    });
  },

  authStep2: function(aAuthData) {
    debug("authStep2()");
    return new Promise((aResolve, aReject) => {
      // MOZ: mozEapAkaAuth, LGE: lgeEapAkaAuth
      this.lgeEapAkaAuth(aAuthData).then(aResponse => {
        debug("Step2:OK aResult=" + aResponse);
        aResolve(aResponse);
      }, aReason => {
        debug("Step2:KO aReason=" + aReason);
        aReject(aReason);
      });
    });
  },

  authStep3: function(aSimResp) {
    debug("authStep3()");
    return new Promise((aResolve, aReject) => {
      // TODO
      debug("Step3:OK aResponse=" + aSimResp);
      aResolve(aSimResp);
    });
  },

  deleteToken: function() {
    debug("deleteToken()");
    let _token = this.token;
    this.token = null;
    return _token;
  },

  mozEapAkaAuth: function(aAuthData) {
    debug("mozEapAkaAuth()");
    let deferred = Promise.defer();
    let aid = "a0000000871002"; // AID prefix of USIM defined in
                                // |Table E.1: Allocated 3GPP PIX numbers| in TS 101 220.

    gIccProvider.iccOpenChannel(PREFERRED_CLIENTID, aid, {
      notifyOpenChannelSuccess: (channel) => {
        // See |7.1.2 Command parameters and data| in TS 31 102.
        gIccProvider.iccExchangeAPDU(PREFERRED_CLIENTID,
                                     channel,
                                     0x00, // CLA
                                     0x88, // INS: EVEN INS Code
                                     0x00, // P1
                                     0x81, // P2: 3G context
                                     (aAuthData.length / 2), // P3: Lc in octets.
                                     aAuthData,
                                     {
          notifyExchangeAPDUResponse: (aSw1, aSw2, aResponse) => {
            gIccProvider.iccCloseChannel(PREFERRED_CLIENTID, channel, null);
            deferred.resolve({ sw1: aSw1, sw2: aSw2, response: aResponse });
          },
          notifyError: (aReason) => {
            gIccProvider.iccCloseChannel(PREFERRED_CLIENTID, channel, null);
            deferred.reject(aReason);
          }
        });
      },
      notifyError: (aReason) => {
        deferred.reject(aReason);
      }
    });
    return deferred.promise;
  },

  lgeEapAkaAuth: function(aAuthData) {
    debug("lgeEapAkaAuth()");
    let deferred = Promise.defer();
    gIccAuthenticator.getIccChallengeResponse(Ci.nsIIccAuthenticator.CARD_APPTYPE_USIM,
                                              Ci.nsIIccAuthenticator.SECURITY_CONTEXT_3G,
                                              aAuthData, {
      notifySuccess: (aSw1, aSw2, aResponse) => {
        deferred.resolve({ sw1: aSw1, sw2: aSw2, response: aResponse });
      },
      notifyError: (aReason) => {
        deferred.reject(aReason);
      }
    });
    return deferred.promise;
  },

  onMessage: function(aMessage) {
    debug("onMessage()");
    this._clients.forEach(function(aClient) {
      aClient.sendAsyncMessage("VZWLogin:OnMessage", {
        data: aMessage
      });
    });
  },

  buildGetAuthBody: function() {
    debug("buildGetAuthBody()");
    return new Promise((aResolve, aReject) => {
      this.getImei().then(imei => {
        var getAuth = {
          "action-name"   : "getAuthentication",
          "request-id"    : this.getRequestId(this.getImsi()),
          "auth-type"     : "EAP-AKA",
          "subscriber-id" : this.getSubscriberId(this.getImsi()),
          "unique-id"     : imei
        };

        var req = [getAuth];
        var reqStr = JSON.stringify(req);

        debug("getAuthRequest : " + reqStr);
        aResolve(reqStr);
      });
    });
  },

  // FIXME
  getUserAgent: function() {
    debug("getUserAgent()");
    return "EAP Client/2.0/12345 (1.2.3; smartphone) Android/5.0.2 (lg; foo)";
  },

  getRIL: function() {
    debug("getRIL()");
    return gRIL.getRadioInterface(PREFERRED_CLIENTID);
  },

  getImsi: function() {
    debug("getImsi()");
    //return "311480003679318";
    return this.getRIL().rilContext.imsi;
  },

  // FIXME
  getImei: function() {
    debug("getImei()");
    return new Promise((aResolve, aReject) => {
      // temporary test. it will be deleted
      aResolve("990003372215505");
/*
      gTelephonyService.dial(PREFERRED_CLIENTID, "*#06#", false, {
        notifyDialMMISuccess: function(imei) {
          debug("my IMEI is " + imei);
          //aResolve("990003372215505");
          aResolve(imei);
        }
      });
*/
    });
  },

  getIccid: function() {
    debug("getIccid()");
    return this.getRIL().rilContext.iccInfo.iccid;
  },

  getRequestId: function() {
    debug("getRequestId()");
    return this.gRequestId++;
  },

  getSubscriberId: function(imsi) {
    debug("getSubscriberId()");
    var nai = this.buildNai(imsi);
    var len = 5 + nai.length;
    var buffer = new Uint8Array(len);

    buffer[0] = EAP_TYPE_REQUEST;
    buffer[1] = 0x00;
    buffer[2] = 0x00; // this is the upper byte of size, but always 0 here;
    buffer[3] = len;
    buffer[4] = EAP_SUBTYPE_IDENTITY;

    for (var i = 0; i < nai.length; i++) {
      buffer[i+5] = nai.charCodeAt(i);
    }

    var res = this.base8ArrayBuffer(buffer); //FIXME;
    debug("subscriber-id: " + res + " from nai:" + nai + " w len:" + len);

    return res;
  },

  buildNai: function(imsi) {
    debug("buildNai()");
    var mcc = imsi.substring(0,3);
    var mnc = imsi.substring(3,6);
    var msin = imsi.substring(6);
    return "0" + imsi + "@nai.epc.mnc" + mnc + ".mcc" + mcc + ".3gppnetwork.org";
  },

  compressBody: function(postBody) {
    debug("compressBody()");
    return new Promise((aResolve, aReject) => {
    let scs = Cc["@mozilla.org/streamConverters;1"].getService(Ci.nsIStreamConverterService);
    let listener = Cc["@mozilla.org/network/stream-loader;1"].createInstance(Ci.nsIStreamLoader);

    listener.init({
      onStreamComplete: function (loader, context, status, dataLen, data) {
        var dataStr = '';
        for (var i = 0; i < data.length; i++) {
          var ival = data[i] | 0;
          var cval = String.fromCharCode(ival);
          dataStr = dataStr + cval;
          //debug("compress construct i: " + i + " int:" + ival + " int.typeof:" + typeof(ival) +
          //  " char:" + cval + " cval.typeof: " + typeof(cval) + " dataStr: " + dataStr);
        }
        debug("compressionComplete:  " + status   + " dataStr.length: " + dataStr.length );
        aResolve({ dataStr: dataStr, dataLength: data.length });
      }
    });

    let converter = scs.asyncConvertData("uncompressed", "gzip", listener, null);

    let stringStream = Cc["@mozilla.org/io/string-input-stream;1"]
                     .createInstance(Ci.nsIStringInputStream);
    stringStream.data = postBody;

    converter.onStartRequest(null, null);
    converter.onDataAvailable(null, null, stringStream, 0, postBody.length);
    converter.onStopRequest(null, null, null);
    });
  },

  callSpc : function(postData, postDataLen, callback) {
    debug("callSpc()");
    return new Promise((aResolve, aReject) => {
    debug("posting with: " + postData);

    let ios = Cc["@mozilla.org/network/io-service;1"].getService(Ci.nsIIOService);
    let uri = ios.newURI(PROD_SPC_URI_STR, null, null);
    let channel = ios.newChannelFromURI(uri);
    let httpChannel = channel.QueryInterface(Ci.nsIHttpChannel);

    let inputStream = Cc["@mozilla.org/io/string-input-stream;1"].createInstance(Ci.nsIStringInputStream);
    inputStream.setData(postData, postDataLen);

    //let inputStream = Cc["@mozilla.org/io/binary-input-stream;1"].createInstance(Ci.nsIBinaryInputStream);
    //inputStream.setData(postData, postData.length);

    let uploadChannel = channel.QueryInterface(Components.interfaces.nsIUploadChannel);
    uploadChannel.setUploadStream(inputStream, "application/json", -1);

    httpChannel.setRequestHeader("User-Agent", this.getUserAgent() , false);
    httpChannel.setRequestHeader("x-protocol-version", "2", false);
    httpChannel.setRequestHeader("accept", "application/json", false);
    httpChannel.setRequestHeader("accept-encoding", "gzip", false);
    httpChannel.setRequestHeader("content-type" , "application/json", false);
    httpChannel.setRequestHeader("content-encoding", "gzip", false);
    httpChannel.requestMethod = "POST";

    debug("making getAuth request");
    httpChannel.asyncOpen({
      _data: "",

      onDataAvailable: function(aRequest, aContext, aStream, aSourceOffset, aLength) {
        debug("onDataAvailable - top");
        var ins = Cc["@mozilla.org/scriptableinputstream;1"].createInstance(Ci.nsIScriptableInputStream);
        ins.init(aStream);
        this._data += ins.read(ins.available());
        //debug("onDataAvailable - called with len:" + aLength +
        //      " offset:" + aSourceOffset + " dataSoFar:" + this._data);
      },

      onStartRequest: function() {
        debug("request started")
        httpChannel.visitRequestHeaders({
          visitHeader: function(aHeader, aValue) {
            debug("request header " + aHeader  + ": " + aValue);
          }
        });
      },

      onStopRequest: function(aRequest, aContext, aStatus) {
        debug("response: status:" + aStatus + " data:" + this._data);
        if (aStatus != 0) {
          aReject("network error : " + aStatus);
          return;
        }
        //debug("responsStatusText: " + aRequest.responseStatusText);
        //debug("responsStatus: " + aRequest.responseStatus);
        httpChannel.visitResponseHeaders({
          visitHeader: function(aHeader, aValue) {
            debug("response header " + aHeader  + ": " + aValue);
          }
        });
        aResolve(this._data);
      },

      QueryInterface: XPCOMUtils.generateQI([Ci.nsIStreamListener, Ci.nsIRequestObserver])
    }, null);
    });
  },

  processResponse: function(data) {
    debug("processResponse() : " + data);

    var respArr = JSON.parse(data);
    var respObj = respArr[0];

    if (respObj.status == 6302) {
      var dsChallenge = "data:text/plain;base64,"+respObj.challenge;

      let ios = Cc["@mozilla.org/network/io-service;1"].getService(Ci.nsIIOService);
      let uri = ios.newURI(dsChallenge, null, null);
      let channel = ios.newChannelFromURI(uri);

      var inputStream = channel.open();
      var ins = Cc["@mozilla.org/binaryinputstream;1"].createInstance(Ci.nsIBinaryInputStream);
      ins.setInputStream(inputStream);

      var arrBuff = new ArrayBuffer(ins.available());
      ins.readArrayBuffer(ins.available(), arrBuff);

      debug("processResponse - challenge :" + dsChallenge + " ->  data.len:" + arrBuff.length);
      return arrBuff;
    }
  },

  handleChallenge: function(buf) {
    debug("handleChallenge()");
    var buf = Uint8Array(buf);
    debug("handleChallenge buf: " + buf);
    var eapType = buf[0];
    var eapSeq  = buf[1];
    var eapLen  = buf[2]*256 + buf[3];

    if (eapType != 1) {
      debug("handleChallenge:  eap type not REQUEST (1) :" + eapType);
      return;
    }

    var akaType = buf[4];
    if (akaType != 23) {
      debug("handleChallenge: aka type not AKA (23) :" + akaType);
    }

    var akaSubType = buf[5];

    if (akaSubType != 1) {  // AKA-Challenge)
      debug("handleChallenge: aka subtype not AKA-Challenge(1) : " + akaSubType);
      return;
    }

    var randHex;
    var autnHex;

    var i = 8;
    while (i < eapLen) {
      var bl = buf[i+1]*4 - 4;
      if (buf[i] == 1) {  // RAND
        randHex = this.byteArrSegToHex(buf, i+4, bl);
        debug("AT_RAND: i:" + i + " type:" + buf[i] + " data: " + autnHex);
      } else if (buf[i] == 2) { // AUTN
        autnHex = this.byteArrSegToHex(buf, i+4, bl);
        debug("AT_AUTN: i:" + i + " type:" + buf[i] + " data: " + autnHex);
      } else {
        debug("AT_????: i:" + i + " type:" + buf[i] + " bl: " + bl);
      }
      i += 4 + bl;
    }

    return "10" + randHex + "10" + autnHex;
  },

  byteArrSegToHex: function(buf, start, len) {
    debug("byteArrSegToHex()");
    var out = '';
    for(var i = start; i < start+len; i++) {
      out += (buf[i] < 16 ? '0' : '') + buf[i].toString(16).toUpperCase();
    }
    return out;
  },

  // call with uint8ByteArray
  base8ArrayBuffer: function(bytes) {
    debug("base8ArrayBuffer()");
    //nsIScriptableBase64Encoder
    var base64    = '';
    var encodings = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';

    var byteLength    = bytes.length;
    var byteRemainder = byteLength % 3;
    var mainLength    = byteLength - byteRemainder;

    debug("base8ArrayBuffer called with len:" + byteLength + " reamin:" + byteRemainder);

    var a, b, c, d;
    var chunk;

    // Main loop deals with bytes in chunks of 3
    for (var i = 0; i < mainLength; i = i + 3) {
      // Combine the three bytes into a single integer
      chunk = (bytes[i] << 16) | (bytes[i + 1] << 8) | bytes[i + 2];

      // Use bitmasks to extract 6-bit segments from the triplet
      a = (chunk & 16515072) >> 18; // 16515072 = (2^6 - 1) << 18
      b = (chunk & 258048)   >> 12; // 258048   = (2^6 - 1) << 12
      c = (chunk & 4032)     >>  6; // 4032     = (2^6 - 1) << 6
      d = chunk & 63;               // 63       = 2^6 - 1

      // Convert the raw binary segments to the appropriate ASCII encoding
      base64 += encodings[a] + encodings[b] + encodings[c] + encodings[d];
    }

    // Deal with the remaining bytes and padding
    if (byteRemainder == 1) {
      chunk = bytes[mainLength];

      a = (chunk & 252) >> 2; // 252 = (2^6 - 1) << 2

      // Set the 4 least significant bits to zero
      b = (chunk & 3)   << 4; // 3   = 2^2 - 1

      base64 += encodings[a] + encodings[b] + '==';
    } else if (byteRemainder == 2) {
      chunk = (bytes[mainLength] << 8) | bytes[mainLength + 1];

      a = (chunk & 64512) >> 10; // 64512 = (2^6 - 1) << 10
      b = (chunk & 1008)  >>  4; // 1008  = (2^6 - 1) << 4

      // Set the 2 least significant bits to zero
      c = (chunk & 15)    <<  2; // 15    = 2^4 - 1

      base64 += encodings[a] + encodings[b] + encodings[c] + '=';
    }

    return base64;
  }

}

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([VZWLoginService]);
