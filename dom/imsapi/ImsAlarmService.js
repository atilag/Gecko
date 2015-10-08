"use strict";

const {classes: Cc, interfaces: Ci, utils: Cu, results: Cr} = Components;
Components.utils.import("resource://gre/modules/XPCOMUtils.jsm");
Components.utils.import("resource://gre/modules/Services.jsm");

XPCOMUtils.defineLazyModuleGetter(this, "AlarmService",
                                 "resource://gre/modules/AlarmService.jsm");

let DEBUG = false;

function debug(msg) {
  dump("-*- ImsAlarmService: " + msg + "\n");
}

/**
 * This component watches for network interfaces changing state and then
 * adjusts routes etc. accordingly.
 */
function ImsAlarmService() {
  debug("Starting IMS Alarm Service.");
}


ImsAlarmService.prototype = {
  classID: Components.ID("{1ebf7e51-ca96-4d31-9a9f-f721c6825b26}"),
  classDescription: "IMS Alarm Service",
  contractID: "@mozilla.org/ImsAlarmservice;1",
  QueryInterface: XPCOMUtils.generateQI([Components.interfaces.nsIImsAlarmService]),

  onAlarmFired: function(aAlarm) {
    dump("ImsAlarmService  Alarm fired ");
    let NotifyOnAlarmFired = Cc["@mozilla.org/lgeImsApi;1"];
    if(NotifyOnAlarmFired){
      debug("IMS Alarm Service NotifyOnAlarmFired Success");
      NotifyOnAlarmFired = NotifyOnAlarmFired.getService(Ci.nsILgeImsAPI);
      if(NotifyOnAlarmFired) {
        debug("IMS Alarm Service NotifyOnAlarmFired Call NotifyOnAlarmFired"+aAlarm.id);
        NotifyOnAlarmFired.NotifyOnAlarmTimerFired(aAlarm.id);
      }else {
        debug("IMS Alarm Service NotifyOnAlarmFired getService failed");
      }
      AlarmService.remove(aAlarm.id);
    }else {
      debug("IMS Alarm Service NotifyOnAlarmFired failed");
    }
  },

  ImsSetAlarm: function(sec, timerId) {
  dump("ImsSetAlarm" +sec);

  AlarmService.add(
   {
     date: new Date(Date.now() + sec),
     ignoreTimezone: true
   },
  this.onAlarmFired.bind(this),
  function onSuccess(alarmID) {
   this._alarmID = alarmID;
   dump("ImsAlarmService Set alarm "+ alarmID);
   dump("ImsAlarmService Set alarm "+ sec);
   let NotifyAlarmId = Cc["@mozilla.org/lgeImsApi;1"];
   if(NotifyAlarmId){
     debug("IMS Alarm Service NotifyAlarmId Success");
     NotifyAlarmId = NotifyAlarmId.getService(Ci.nsILgeImsAPI);
     if(NotifyAlarmId) {
       debug("IMS Alarm Service NotifyAlarmId Call NotifyAlarmId "+timerId);
       NotifyAlarmId.NotifyOnAlarmId(timerId, alarmID);
     }else {
       debug("IMS Alarm Service NotifyAlarmId getService failed");
     }
   }else {
     debug("IMS Alarm Service NotifyAlarmId failed");
   }
   }.bind(this)
  )
  },
  ImsStopAlarm: function(alarmID) {
    dump("ImsStopAlarm" + alarmID);
    AlarmService.remove(alarmID)
    dump("ImsStopAlarm Alarm removed");
  },

};

if (XPCOMUtils.generateNSGetFactory)
    this.NSGetFactory = XPCOMUtils.generateNSGetFactory([ImsAlarmService]);
else
    var NSGetModule = XPCOMUtils.generateNSGetModule([ImsAlarmService]);
