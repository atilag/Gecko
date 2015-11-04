/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

"use strict";

var {classes: Cc, interfaces: Ci, utils: Cu} = Components;

Cu.import("resource://gre/modules/Services.jsm");
Cu.import("resource://gre/modules/NetUtil.jsm");
Cu.import("resource://gre/modules/XPCOMUtils.jsm");

XPCOMUtils.defineLazyModuleGetter(this, "OS", "resource://gre/modules/osfile.jsm");
const PARTITION_MAGIC =  0x19901873;
const DIGEST_SIZE_BYTES = 32;

function debug(aStr) {
  do_print("head_persistentdatablock: " + aStr + "\n");
}

var frpIinitializedCounter = 0;

function _prepareConfig(_args) {

  var args = _args || {};
  args.digest = args.digest || new Uint8Array([1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32]);
  args.dataLength = args.dataLength || 6;
  args.data = args.data || new Uint8Array(["P","A","S","S","W","D"]);
  args.oem = args.oem || true;
  args.oemUnlockAllowed = args.oemUnlockAllowed || true;
  
  debug("_prepareConfig: args.digest = " + args.digest );
  debug("_prepareConfig: args.dataLength = " + args.dataLength );
  debug("_prepareConfig: args.data = " + args.data );
  debug("_prepareConfig: args.oem = " + args.oem );
  debug("_prepareConfig: args.oemUnlockAllowed = " + args.oemUnlockAllowed );

  PersistentDataBlock._libcutils.property_set("ro.frp.pst", "/data/local/tmp/frp.test");
  PersistentDataBlock._libcutils.property_set("sys.oem_unlock_allowed", args.oemUnlockAllowed === true ? "true" : "false");
  PersistentDataBlock.init();
  // Create the test file with the same structure as the partition will be
  var tempFile;
  return OS.File.open(PersistentDataBlock._dataBlockFile, {write:true, append:false, truncate: true}).then( _tempFile => {
    debug("_prepareConfig: Writing DIGEST...");
    tempFile = _tempFile;
    return tempFile.write(args.digest);
  }).then( bytes => {
    debug("_prepareConfig: Writing the magic: " + PARTITION_MAGIC);
    return tempFile.write(new Uint32Array([PARTITION_MAGIC]));
  }).then( bytes => {
    return tempFile.write(new Uint32Array([args.dataLength]));
  }).then( bytes => {
    let data = new Uint8Array(PersistentDataBlock._getBlockDeviceSize() - (32 + 4 + 4 + 1));
    data.set(args.data);
    return tempFile.write( data );
  }).then( bytes => {
    return tempFile.write(new Uint8Array([ args.oem === true ? 1 : 0 ]));
  }).then( bytes => {
    return tempFile.close();
  }).then( () =>{
    debug("_prepareConfig: copying frp.test to frp.initial...");
    return OS.File.copy(PersistentDataBlock._dataBlockFile, "/data/local/tmp/frp.initial." + frpIinitializedCounter++);
  }).then( () => {
    debug("_prepareConfig end");
    return Promise.resolve(true);
  }).catch( ex => {
    debug("_prepareConfig: ERROR: ex = " + ex);
    return Promise.reject();
  });
}


function utils_getByteAt( pos ){
  var file;
  var byte;
  return OS.File.open(PersistentDataBlock._dataBlockFile, {read:true, existing:true, append:false}).then( _file => {
    file = _file;
    return file.setPosition(pos, OS.File.POS_START);
  }).then( () => {
    return file.read(1);
  }).then( _byte => {
    byte = _byte;
    return file.close();
  }).then( () => {
    return Promise.resolve(byte[0]);
  }).catch(ex => {
    return Promise.reject();
  });
}

function utils_getHeader(){
  var file;
  var header = {};
  return OS.File.open(PersistentDataBlock._dataBlockFile, {read:true, existing:true, append:false}).then( _file => {
    file = _file;
    return file.read(32);
  }).then( digest => {
    header.digest = digest;
    return file.read(4);
  }).then( magic => {
    header.magic = magic;
    return file.read(4);
  }).then( dataLength => {
    header.dataLength = dataLength;
    return file.close();
  }).then( () => {
    return Promise.resolve(header);
  }).catch(ex => {
    return Promise.reject();
  });
}

function utils_getData(){
  var file;
  var data;
  return OS.File.open(PersistentDataBlock._dataBlockFile, {read:true, existing:true, append:false}).then( _file => {
    file = _file;
    return file.setPosition(36, OS.File.POS_START);
  }).then( () => {
    return file.read(4);
  }).then( _dataLength => {
    let dataLength = new Uint32Array(_dataLength.buffer);
    debug("utils_getData: dataLength = " + dataLength[0]);
    return file.read(dataLength[0]);
  }).then( _data => {
    data = _data;
    return file.close();
  }).then( () => {
    return Promise.resolve(data);
  }).catch(ex => {
    return Promise.reject();
  });
}


function _installTests() {

  add_test(function test_getBlockDeviceSize() {
    let blockSize = PersistentDataBlock._getBlockDeviceSize();
    //strictEqual( blockSize, 1024 );
    ok( blockSize > 0, "test_getBlockDeviceSize: Block device size should be greater than 0");
    run_next_test();
  });

  add_test(function test_wipe() {
    PersistentDataBlock.wipe().then( () => {
      run_next_test();
    }).catch(ex => {
      ok(false, "test_wipe failed!: ex: " + ex);
    });
  });

  add_test(function test_computeDigest() {
    _prepareConfig().then( () => {
      PersistentDataBlock._computeDigest().then( digest => {
        strictEqual( digest, "H3AZJO/N8kclnurNSUxeJAsyLGu7jivVvviP4HXEtQc=");
        run_next_test();
      }).catch( ex => {
        ok(false, "test_computeDigest failed!: ex: " + ex);
      });
    });
  });

  add_test(function test_getDataFieldSize() {
    PersistentDataBlock.getDataFieldSize().then( dataFieldLength => {
      debug("test_getDataFieldSize: dataFieldLength is " + dataFieldLength);
      strictEqual( dataFieldLength, 6);
      run_next_test();
    }).catch( ex => {
      ok(false, "test_getOemUnlockedEnabled failed: ex:" + ex);
    });
  });  

  add_test(function test_getOemUnlockedEnabled() {
    PersistentDataBlock.getOemUnlockEnabled().then( enabled => {
      debug("test_getOemUnlockedEnabled: enabled is " + enabled);
      ok(enabled, "test_getOemUnlockedEnabled: should get enabled");
      ok(enabled === false || enabled === true, "test_getOemUnlockedEnabled failed, value of enabled is not true or false");
      run_next_test();
    }).catch( ex => {
      ok(false, "test_getOemUnlockedEnabled failed: ex:" + ex);
    });
  });

  add_test(function test_setOemUnlockedEnabled() {
    PersistentDataBlock.setOemUnlockEnabled(true).then(() => {
      return utils_getByteAt(PersistentDataBlock._getBlockDeviceSize() - 1);
    }).then( byte => {
      debug("test_setOemUnlockedEnabled: 1. byte = " + byte );
      strictEqual( byte, 1 );
      OS.File.copy( PersistentDataBlock._dataBlockFile, "/data/local/tmp/frp.1").then(()=>{});
      return PersistentDataBlock.setOemUnlockEnabled(false);
    }).then( () => {
      OS.File.copy( PersistentDataBlock._dataBlockFile, "/data/local/tmp/frp.2").then(()=>{});
      return utils_getByteAt(PersistentDataBlock._getBlockDeviceSize() - 1);
    }).then( byte => {
      debug("test_setOemUnlockedEnabled: 2. byte = " + byte );
      strictEqual( byte, 0 );
      run_next_test();
    }).catch(ex => {
      ok(false, "test_setOemUnlockedEnabled failed!: ex: " + ex);
    });
  });

  add_test(function test_computeAndWriteDigest() {
    PersistentDataBlock._computeAndWriteDigest().then( () => {
      OS.File.copy( PersistentDataBlock._dataBlockFile, "/data/local/tmp/frp.3").then(()=>{});
      return utils_getHeader();
    }).then(header => {
      debug("test_computeAndWriteDigest: header = " + header);
      let magicRead = new Uint32Array(header.magic.buffer);
      let magicSupposed = new Uint32Array([PARTITION_MAGIC]);
      strictEqual(magicRead[0], magicSupposed[0]);
      let dataLength = new Uint32Array([header.dataLength]);
      strictEqual(header.dataLength[0], 6);
      run_next_test();
    }).catch(ex => {
      ok(false, "test_computeAndWriteDigest failed!: ex: " + ex);
    });
  });

  add_test(function test_formatIfOemUnlockEnabled() {
    _prepareConfig({oem:true}).then(()=>{});
    PersistentDataBlock._formatIfOemUnlockEnabled().then( (result) => {
      OS.File.copy( PersistentDataBlock._dataBlockFile, "/data/local/tmp/frp.formatted").then(()=>{});
      return utils_getByteAt(PersistentDataBlock._getBlockDeviceSize() - 1);
    }).then( byte => {
      //Check if the last byte is 1
      strictEqual(byte, 1);
      run_next_test();
    }).catch(ex => {
      ok(false, "test_computeAndWriteDigest failed!: ex: " + ex);
    });
  });

  add_test(function test_formatPartition() {
    //Restore a fullfilled partition so we can check if formatting works...
    _prepareConfig({oem:true}).then(()=>{});
    PersistentDataBlock._formatPartition(true).then( () => {
      return utils_getByteAt(PersistentDataBlock._getBlockDeviceSize() - 1);
    }).then(byte => {
      //Check if the last byte is 1
      strictEqual( byte, 1 );
      return utils_getHeader();
    }).then(header => {
      //The Magic number should exists in a formatted partition
      let magicRead = new Uint32Array(header.magic.buffer);
      let magicSupposed = new Uint32Array([PARTITION_MAGIC]);
      strictEqual(magicRead[0], magicSupposed[0]);
      //In a formatted partition, the digest field is always 32 bytes of zeros.
      let digestSupposed = new Uint8Array(DIGEST_SIZE_BYTES);
      strictEqual(header.digest.join(""), "94227253995810864198417798821014713171138121254110134189198178208133167236184116199");
      return PersistentDataBlock._formatPartition(false);
    }).then( () => {
      return utils_getByteAt(PersistentDataBlock._getBlockDeviceSize() - 1);
    }).then(byte => {
      //In this case OEM Unlock enabled byte should be set to 0
      strictEqual(byte, 0);
      run_next_test();
    }).catch(ex => {
      ok(false, "test_formatPartition failed!: ex: " + ex);
    });
  });

  add_test(function test_enforceChecksumValidity() {
    PersistentDataBlock._enforceChecksumValidity().then( () => {
      run_next_test();
    }).catch(ex => {
      ok(false, "test_enforceChecksumValidity failed!: ex: " + ex);
    });
  });

  add_test(function test_read() {
    PersistentDataBlock.read().then( bytes => {
      debug("test_read: bytes: " + bytes);
      run_next_test();
    }).catch(ex => {
      ok(false, "test_read failed!: ex: " + ex);
    });
  });

  add_test(function test_write() {
    let data = new Uint8Array(['1','2','3','4','5']);
    PersistentDataBlock.write(data).then( bytesWrittenLength => {
      debug("test_write: bytesWrittenLength = " + bytesWrittenLength);
      return utils_getData();
    }).then(data => {
      strictEqual( data[0], 1);
      strictEqual( data[1], 2);
      strictEqual( data[2], 3);
      strictEqual( data[3], 4);
      strictEqual( data[4], 5);
      run_next_test();
    }).catch(ex => {
      ok(false, "test_write failed!: ex: " + ex);
    });
  });

}
