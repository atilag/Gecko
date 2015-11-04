/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */


/**
 * The Persistent Partition has this layout:
 *
 * Bytes:     32       4        4          <DATA_LENGTH>               1
 * Fields: [[DIGEST][MAGIC][DATA_LENGTH][        DATA        ][OEM_UNLOCK_ENABLED]]
 *
 */

"use strict";

this.EXPORTED_SYMBOLS = [ "PersistentDataBlock" ];

const {classes: Cc, interfaces: Ci, utils: Cu, results: Cr} = Components;
const DEBUG = false;
// This is a marker that will be written after digest in the partition.
const PARTITION_MAGIC = 0x19901873;
// This is the limit in Android because of issues with Binder if blocks are > 100k
const MAX_DATA_BLOCK_SIZE = 1024 * 100;
const DIGEST_SIZE_BYTES = 32;
const HEADER_SIZE = 8;
const XPCOM_SHUTDOWN_OBSERVER_TOPIC = "xpcom-shutdown";
// This property will have the path to the persistent partition 
const PERSISTENT_DATA_BLOCK_PROPERTY = "ro.frp.pst";
const OEM_UNLOCK_PROPERTY = "sys.oem_unlock_allowed";

Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/Services.jsm");

XPCOMUtils.defineLazyModuleGetter(this, "OS", "resource://gre/modules/osfile.jsm");
XPCOMUtils.defineLazyModuleGetter(this, "Promise", "resource://gre/modules/Promise.jsm");


#ifdef MOZ_WIDGET_GONK
  XPCOMUtils.defineLazyGetter(this, "libcutils", function () {
    Cu.import("resource://gre/modules/systemlibs.js");
    return libcutils;
  });
#else
  this.libcutils = null;
#endif

var inParent = Cc["@mozilla.org/xre/app-info;1"]
                 .getService(Ci.nsIXULRuntime)
                 .processType == Ci.nsIXULRuntime.PROCESS_TYPE_DEFAULT;

function debug(aStr) {
  dump("PersistentDataBlock.jsm: " + aStr + "\n");
}

function toHexString(charCode) {
  return ("0" + charCode.toString(16).slice(-2));
}

function convertStrToUint8Array(str) {
  let res = new Uint8Array(str.length);
  let index = 0;
  for (let i = 0; i < res.byteLength; i++) {
    res[i] = str.charCodeAt(i);
  }
  return res;
}

//https://dxr.mozilla.org/mozilla-central/source/browser/components/loop/content/shared/js/utils.js#509
function _b64ToUint6(chr) {
  return chr > 64 && chr < 91 ? chr - 65 :
         chr > 96 && chr < 123 ? chr - 71 :
         chr > 47 && chr < 58 ? chr + 4 :
         chr === 43 ? 62 :
         chr === 47 ? 63 : 0;
}

//https://dxr.mozilla.org/mozilla-central/source/browser/components/loop/content/shared/js/utils.js#442
function atob(base64str) {
  var strippedEncoding = base64str.replace(/[^A-Za-z0-9\+\/]/g, "");
  var inLength = strippedEncoding.length;
  var outLength = inLength * 3 + 1 >> 2;
  var result = new Uint8Array(outLength);

  var mod3;
  var mod4;
  var uint24 = 0;
  var outIndex = 0;

  for (var inIndex = 0; inIndex < inLength; inIndex++) {
    mod4 = inIndex & 3;
    uint24 |= _b64ToUint6(strippedEncoding.charCodeAt(inIndex)) << 6 * (3 - mod4);

    if (mod4 === 3 || inLength - inIndex === 1) {
      for (mod3 = 0; mod3 < 3 && outIndex < outLength; mod3++, outIndex++) {
        result[outIndex] = uint24 >>> (16 >>> mod3 & 24) & 255;
      }
      uint24 = 0;
    }
  }

  return result;
}



this.PersistentDataBlock = {

  /**
   * libc funcionality. Accessed via ctypes
   */
   _libc: {
      handler: null,
      open: function(){},
      close: function(){},
      ioctl: function(){}
   },

  /**
   * Component to access property_get/set functions
   */
  _libcutils: null,

  /**
   * The size of a device block. This is assigned by querying the kernel.
   */
  _blockDeviceSize: -1,

  /**
   * Data block file
   */
  _dataBlockFile: "",

  init: function() {
    debug("init");

    if(libcutils) {
      this._libcutils = libcutils;
    }

    if(!this.ctypes){
      Cu.import("resource://gre/modules/ctypes.jsm", this);
    }

    if(this._libc.handler == null) {
#ifdef MOZ_WIDGET_GONK
      try {
        this._libc.handler = this.ctypes.open(this.ctypes.libraryName("c"));
        this._libc.close = this._libc.handler.declare("close",
                                                      this.ctypes.default_abi,
                                                      this.ctypes.int, 
                                                      this.ctypes.int
                                                     );
        this._libc.open = this._libc.handler.declare("open", 
                                                     this.ctypes.default_abi,
                                                     this.ctypes.int,
                                                     this.ctypes.char.ptr,
                                                     this.ctypes.int
                                                    );
        this._libc.ioctl = this._libc.handler.declare("ioctl", 
                                                      this.ctypes.default_abi,
                                                      this.ctypes.int,
                                                      this.ctypes.int,
                                                      this.ctypes.unsigned_long,
                                                      this.ctypes.unsigned_long.ptr
 
      } catch (ex) {
        debug("Unable to open libc.so: ex = " + ex);
        throw Cr.NS_ERROR_FAILURE;
      }
#endif
    }


    this._dataBlockFile = this._libcutils.property_get(PERSISTENT_DATA_BLOCK_PROPERTY);
    Services.obs.addObserver(this, XPCOM_SHUTDOWN_OBSERVER_TOPIC, false);
  },

  start: function() {
    debug("start");
    return this._enforceChecksumValidity().then(() => {
      return this._formatIfOemUnlockEnabled().then(() => {
        return Promise.resolve(true);
      })
    }).catch(() => {
      return Promise.reject();
    });
  },

  uninit: function() {
    debug("_uninit");
    this._libc.handler.close();
    Services.obs.removeObserver(this, XPCOM_SHUTDOWN_OBSERVER_TOPIC);
  },

  checkLibcUtils: function() {
    debug("checkLibcUtils");
    if (!this._libcutils) {
      debug("No proper libcutils binding, aborting.");
      throw Cr.NS_ERROR_NO_INTERFACE;
    }

    return true;
  },

  // addObserver
  observe: function(aSubject, aTopic, aData) {
    debug("observe");
    switch (aTopic) {
      case XPCOM_SHUTDOWN_OBSERVER_TOPIC:
        this.uninit();
        break;

      default:
        debug("Wrong observer topic: " + aTopic);
        break;
    }
  },

  /**
   * Computes the digest of the entire data block. 
   * The digest is saved in the first 32 bytes of the block.
   *
   * @param storedDigest An out parameter wich will contains the digest read from
   *                     the digest filed in the block, NOT the one which is about
   *                     to be calculated.
   *
   * @return Promise<digest> {String} The calculated digest
   */
  _computeDigest: function (storedDigest) {
    debug("_computeDigest");
    var digest;
    var partition;
    return OS.File.open(this._dataBlockFile, {existing:true, append:false, read:true}).then( _partition => {
      debug("_computeDigest(): reading digest bytes");
      partition = _partition;
      return partition.read(DIGEST_SIZE_BYTES);
    }).then( digestDataRead => {
      debug("_computeDigest(): digest read");
      //If storedDigest is passed as a parameter, the caller will likely compare the 
      //one is already stored in the partition with the one we are going to compute later.
      if(storedDigest && storedDigest.length == DIGEST_SIZE_BYTES ) {
        debug("_computeDigest(): get stored digest from the partition");
        storedDigest = digestDataRead;
      }
      return partition.read();
    }).then(data => {
      debug("_computeDigest(): computing digest from the rest of the data");
      //Calculate Digest with the data retrieved after the digest
      let hasher = Cc["@mozilla.org/security/hash;1"].createInstance(Ci.nsICryptoHash);
      hasher.init(hasher.SHA256);
      hasher.update(data, data.length);
      let _digest = hasher.finish(true);
      digest = atob(_digest);
      debug("_computeDigest(): Digest [" + typeof(digest) + "]: "  + _digest + "(" + digest.length + ")");
      return partition.close();
    }).then( () => {
      return Promise.resolve(digest);
    }).catch(ex => {
      debug("_computeDigest(): Failed to read partition: ex = " + ex);
      return Promise.reject(false);
    }); 
  },

  /**
   * Returns the size of a block from the undelaying filesystem
   *
   * @return {Number} The size of the block
   */
  _getBlockDeviceSize: function() {
    debug("_getBlockDeviceSize");
#ifdef MOZ_WIDGET_GONK
    const O_READONLY = 0;
    const O_NONBLOCK = 1 << 11;

    const BLKGETSIZE64 = 2147750514; //TODO: That's not the way. Gonk should provide a realiable method
    debug("_dataBlockFile: _dataBlockFile = " + this._dataBlockFile);
    let fd = this._libc.open(this._dataBlockFile, O_READONLY | O_NONBLOCK);
    if(fd < 0){
      debug("_getBlockDeviceSize: couldn't open partition!: error = " + fd);
      throw Cr.NS_ERROR_FAILURE; 
    }

    let size = new this.ctypes.unsigned_long(); 
    let sizeAddress = size.address();
    let ret = this._libc.ioctl(fd, BLKGETSIZE64, sizeAddress);
    if( ret < 0 ){
      debug("_getBlockDeviceSize: couldn't get block device size!: error = " + this.ctypes.errno);
      this._libc.close(fd);
      throw Cr.NS_ERROR_FAILURE;
    }

    this._libc.close(fd);
    debug("_getBlockDeviceSize: size =" + size.value);
    return size.value;
#else
  debug("_getBlockDeviceSize: ERROR: This funcionality is only supported in Gonk!");
  return -1;
#endif
  },


  /**
   * Sets the byte into the partition which represents the OEM Unlock Enabled feature.
   * A value of "1" means that the user doesn't want to enable KillSwitch.
   * The byte is the last one byte into the data block.
   *
   * @param isSetOemUnlockEnabled {bool} If true, sets the OEM Unlock Enabled byte to 1.
   *                                     Otherwise, sets it to 0.
   */
  _doSetOemUnlockEnabled: function(isSetOemUnlockEnabled){
    debug("_doSetOemUnlockEnabled");
    var self = this;
    var partition;
    return OS.File.open(this._dataBlockFile, {existing:true, append:false, write:true}).then( _partition => {
      partition = _partition;
      return partition.setPosition(self._getBlockDeviceSize() - 1, OS.File.POS_START);
    }).then( () => {
      return partition.write(new Uint8Array([ isSetOemUnlockEnabled === true ? 1 : 0 ]));
    }).then( bytesWrittenLength => {
      return partition.close();
    }).then( () => {
      let oemUnlockByte = (isSetOemUnlockEnabled === true ? "1" : "0");
      debug("_doSetOemUnlockEnabled: OEM unlock enabled written to " + oemUnlockByte);
      this._libcutils.property_set(OEM_UNLOCK_PROPERTY, oemUnlockByte);
      return Promise.resolve();
    }).catch( ex => {
      return Promise.reject();
    });   
  },

  /**
   * Gets the byte from the partition which represents the OEM Unlock Enabled state.
   *
   * @return 1 Promise<Bool> The user didn't activate KillSwitch.
   * @return 0 Promise<Bool> The user did activate KillSwitch.
   */
  _doGetOemUnlockEnabled: function() {
    debug("_doGetOemUnlockEnabled");
    var ret = false;
    var partition;
    var self = this;
    return OS.File.open(this._dataBlockFile, {existing:true, append:false, read:true}).then( _partition => {
      debug("_doGetOemUnlockEnabled: persistent partition opened");
      partition = _partition;
      return partition.setPosition( self._getBlockDeviceSize() - 1 , OS.File.POS_START);
    }).then( () => {
      debug("_doGetOemUnlockEnabled: position setted.");
      return partition.read(1);
    }).then( data => {
      debug("_doGetOemUnlockEnabled: OEM unlock enabled byte = '" + data[0] + "'");
      ret = (data[0] === 1 ? true : false);
      return partition.close();
    }).then( () => {
      debug("_doGetOemUnlockEnabled: persistent partition closed");
      return Promise.resolve(ret);
    }).catch( ex => {
      debug("_doGetOemUnlockEnabled: Error reading OEM unlock enabled byte from partition: ex = " + ex);
      return Promise.resolve(false);
    });
  },

  /**
   * Computes the digest by reading the entire block of data and write it to the digest field
   *
   * @return true Promise<bool> Operation succeed
   * @return false Promise<bool> Operation failed
   */
  _computeAndWriteDigest: function(){
    debug("_computeAndWriteDigest");
    var digest;
    var self = this;
    var partition;
    return this._computeDigest().then( _digest => {
      digest = _digest;
      return OS.File.open(self._dataBlockFile, {write:true, existing:true, append:false});
    }).then( _partition => {
      partition = _partition;
      return partition.write(digest);
    }).then( bytesWrittenLength => {
      return partition.close();
    }).then( () => {
      debug("_computeAndWriteDigest: digest written to partition");
      return Promise.resolve(true);
    }).catch( ex => {
      debug("_computeAndWriteDigest: Couldn't write digest in the persistent partion. ex = " + ex );        
      return Promise.reject(false);
    });
  },

  /**
   * Formats the persistent partition if the OEM Unlock Enabled field is set to true, and
   * write the Unlock Property accordingly.
   * 
   * @return true Promise<bool> OEM Unlock was enabled, so the partition has been formated
   * @return false Promise<bool> OEM Unlock was disabled, so the partition hasn't been formated
   */
  _formatIfOemUnlockEnabled: function () {
    debug("_formatIfOemUnlockEnabled");
    var self = this;
    return this._doGetOemUnlockEnabled().then(enabled => {
      self._libcutils.property_set(OEM_UNLOCK_PROPERTY,(enabled == true ? "1" : "0"));
      if(enabled === true){
        return self._formatPartition(enabled);
      }
      return Promise.resolve(false);
    }).then( (result) => {
      if( result && result === false )
        return Promise.resolve(false);
      else
        return Promise.resolve(true);
    }).catch(err => {
      return Promise.reject();
    });
  },


  /**
   * Formats the persistent data partition with the propper strucutre.
   *
   * @param isSetOemUnlockEnabled {bool} If true, writes a "1" in the OEM Unlock Enabled field (last 
   *                                     byte of the block). If false, writes a "0".
   *
   */
  _formatPartition: function(isSetOemUnlockEnabled) {
    debug("_formatPartition");
    var self = this;
    var partition;
    return OS.File.open(this._dataBlockFile, {write:true, existing:true, append:false}).then( _partition => {
      partition = _partition;
      return partition.write(new Uint8Array(DIGEST_SIZE_BYTES));
    }).then( bytesWrittenLength => {
      return partition.write(new Uint32Array([PARTITION_MAGIC]));
    }).then( bytesWrittenLength => {
      return partition.write(new Uint8Array(4));
    }).then( bytesWrittenLength => {
      return partition.close();
    }).then( () => {
      return self._doSetOemUnlockEnabled(isSetOemUnlockEnabled);
    }).then( () => {
      return self._computeAndWriteDigest();
    }).then( () => {
      return Promise.resolve();
    }).catch( ex => {
      debug("_formatPartition: Failed to format block");
      return Promise.reject();
    });
  },

  /**
   * Check digest validity. If it's not, format the persistent partition
   */
  _enforceChecksumValidity: function() {
    debug("_enforceChecksumValidity");
    return this._computeDigest().then( digest => {
      debug("_enforceChecksumValidity: Digest computation succeed.");
      return Promise.resolve(true);
    }).catch( ex => {
      debug("_enforceChecksumValidity: Digest computation failed: ex = " + ex + ". Formatting FRP partition...");
      return this._formatPartition(false).then( () => {
          return Promise.reject(false);
      });
    });
  },

  /**
   * Reads the entire data block.
   *
   * @return bytes Promise<Uint8Array> A promise resolved with the bytes read or undefined if there were errors.
   */
  read: function() {
    var partition;
    var bytes;
    var totalDataSize = this.getDataFieldSize();
    if( totalDataSize == 0 ){
       return Promise.resolve(0);
    }
    var self = this;
    return OS.File.open(this._dataBlockFile, {read:true, existing:true, append:false}).then(_partition => {
      partition = _partition;
      return partition.read(totalDataSize);
    }).then( _bytes => {
      bytes = _bytes;
      return partition.close();
    }).then( () => {
      if( bytes.byteLength < totalDataSize ){
        debug("read: Failed to read entire data block. Bytes read: " + bytes.byteLength + "/" + totalDataSize);
        return Promise.reject();
      }
      return Promise.resolve(bytes);
    }).catch( ex => {
      debug("read: Failed to read entire data block. Exception: " + ex);
      return Promise.reject();
    });
  },

  /**
   * Writes an entire block to the persistent partition
   *
   * @param data {Uint8Array}
   * 
   * @return Promise<Number> Promise resolved to the number of bytes written.
   *                         If there's an error, the promise get rejected with a negative value.
   */

  write: function(data) {
    let maxBlockSize = this._getBlockDeviceSize() - HEADER_SIZE - 1;
    if(data.byteLength > maxBlockSize){
      debug("write: Couldn't write more than " + maxBlockSize + " bytes to the partition. " +
            maxBlockSize + " bytes given.");
      return Promise.reject(-data.byteLength);
    }

    var partition;
    var self = this;
    return OS.File.open(this._dataBlockFile, {write:true, existing:true, append:false}).then( _partition => {
      let digest = new Uint8Array(DIGEST_SIZE_BYTES);
      let magic = new Uint32Array([PARTITION_MAGIC]);
      let dataLength = new Uint32Array([data.byteLength]);
      let bufferToWrite = new Uint8Array(digest.byteLength + magic.byteLength + dataLength.byteLength + data.byteLength );
      let offset = 0;
      bufferToWrite.set(digest, offset);
      offset += digest.byteLength;
      bufferToWrite.set(magic, offset);
      offset += magic.byteLength;
      bufferToWrite.set(dataLength, offset);
      offset += dataLength.byteLength;
      bufferToWrite.set(data, offset);

      partition = _partition;
      return partition.write(bufferToWrite);
    }).then( bytesWrittenLength => {
      return partition.close();
    }).then( () => {
      return self._computeAndWriteDigest();
    }).then( (couldComputeAndWriteDigest) => {
      if(couldComputeAndWriteDigest === true){
        return Promise.resolve(data.byteLength);
      }else{
        debug("write: Failed to compute and write the digest");
        return Promise.reject(-1);
      }
    }).catch( ex => {
      debug("write: Failed to write to the persistent partition");
      return Promise.reject(-1);
    });
  },

  /**
   * Wipes the persistent partition. 
   *
   * @return 0  Promise<Number> The promise has been rejected because cannot get block device size
   * @return -1 Promise<Number> The promise has been rejected because of some other error
   * @return Prmoise<Number> If no errors, the promise is resolved
   */
  wipe: function() {

#ifdef MOZ_WIDGET_GONK
    const O_READONLY = 0;
    const O_RDWR = 2;
    const O_NONBLOCK = 1 << 11;
    const BLKSECDISCARD = 4733;
    const BLKDISCARD = 4727;

    var self = this;

    return new Promise((resolve, reject) => {
      debug("1");
      //let range = self.ctypes.unsigned_long.array(2)();
      let range = new self.ctypes.unsigned_long();
      debug("2");
      let rangeAddress = range.address();
      debug("3");
      let blockDeviceLength = self._getBlockDeviceSize();
      debug("4");
      debug("5");
      range[0] = 0;
      debug("6");
      range[1] = blockDeviceLength;
      debug("7");
      if(range[1] == 0){
        debug("wipe: Block device size is 0!");
        return reject(0);
      }
      debug("8");
      let fd = self._libc.open(self._dataBlockFile, O_RDWR);
      debug("9");
      if(fd < 0){
        debug("wipe: ERROR couldn't open partition!: error = " + fd);
        return reject(-1);
      }
      debug("10");
      let ret = self._libc.ioctl(fd, BLKSECDISCARD, rangeAddress);
      debug("11");
      if(ret < 0){
        debug("wipe: Something went wrong secure discarding block: errno: " + self.ctypes.errno);
        range[0] = 0;
        range[1] = blockDeviceLength;
        ret = self._libc.ioctl(fd, BLKDISCARD, rangeAddress);
        if(ret < 0){
          debug("wipe: ERROR: discard failed: errno: " + self.ctypes.errno);
          return reject(-1);
        }else{
          debug("wipe: non-secure discard used and succeed, because secure discard failed");
          return resolve();
        }
      }
      debug("wipe: secure discard succeed");
      return resolve();
    });
#else
    debug("wipe: ERROR: This funcionality is only supported in Gonk!");
    return Promise.reject();
#endif
  },

  /**
   * 
   */
  setOemUnlockEnabled: function(enabled) {
    debug("setOemUnlockEnabled");
    var self = this;
    return this._doSetOemUnlockEnabled(enabled).then(() => {
      return self._computeAndWriteDigest();
    }).then(() => {
      return Promise.resolve();
    });
  },

  /**
   *
   */
  getOemUnlockEnabled: function() {
    debug("getOemUnlockEnabled");
    return this._doGetOemUnlockEnabled();
  },

  /**
   * Gets the size of the data block by reading the data-length field
   *
   * @return Promise<Number> A promise resolved to the number of bytes os the data field.
   */
  getDataFieldSize: function() {
    debug("getDataFieldSize");
    var partition
    var dataLength = 0;
    return OS.File.open(this._dataBlockFile, {read:true, existing:true, append:false}).then( _partition => {
      partition = _partition;
      //Jump the digest field
      return partition.setPosition(DIGEST_SIZE_BYTES, OS.File.POS_START);
    }).then( () => {
      //Read the Magic field
      return partition.read(Uint32Array.BYTES_PER_ELEMENT);
    }).then( _magic => {
      let magic = new Uint32Array(_magic.buffer);
      if( magic[0] === PARTITION_MAGIC )
        return partition.read(Uint32Array.BYTES_PER_ELEMENT);
      else
        return Promise.reject(0);
    }).then( _dataLength => {
      dataLength = new Uint32Array(_dataLength.buffer)[0];
      return partition.close();
    }).then( () => {
        if(dataLength)
          return Promise.resolve(dataLength);
        else
          return Promise.reject(0);
    }).catch( ex => {
      debug("getDataFieldSize: Couldn't get data field size: ex = " + ex);
      return Promise.reject(0);
    });
  },

  /**
   * Gets the maximun possible size of a data field
   *
   * @return Promise<Number> A Promise resolved to the maximun number of bytes allowed for the data field
   *
   */
  getMaximunDataBlockSize: function() {
    debug("getMaximunDataBlockSize");
    return new Promise((resolve, reject) => {
      let actualSize = this._getBlockDeviceSize() - HEADER_SIZE - 1;
      resolve(actualSize <= MAX_DATA_BLOCK_SIZE ? actualSize : MAX_DATA_BLOCK_SIZE);
    });
  }

};

// This code should ALWAYS be living only on the parent side.
if (!inParent) {
  debug("PersistentDataBlock should only be living on parent side.");
  throw Cr.NS_ERROR_ABORT;
} else {
  this.PersistentDataBlock.init();
}
