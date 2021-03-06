/* Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/
 */

/* File in use inside removed dir partial MAR file patch apply success test */

function run_test() {
  if (!shouldRunServiceTest()) {
    return;
  }

  setupTestCommon();
  gTestFiles = gTestFilesPartialSuccess;
  gTestDirs = gTestDirsPartialSuccess;
  setupUpdaterTest(FILE_PARTIAL_MAR, false, true);

  let fileInUseBin = getApplyDirFile(gTestDirs[2].relPathDir +
                                     gTestDirs[2].files[0]);
  // Remove the empty file created for the test so the helper application can
  // replace it.
  fileInUseBin.remove(false);

  let helperBin = getTestDirFile(FILE_HELPER_BIN);
  let fileInUseDir = getApplyDirFile(gTestDirs[2].relPathDir);
  helperBin.copyTo(fileInUseDir, gTestDirs[2].files[0]);

  // Launch an existing file so it is in use during the update.
  let args = [getApplyDirPath() + "a/b/", "input", "output", "-s",
              HELPER_SLEEP_TIMEOUT];
  let fileInUseProcess = AUS_Cc["@mozilla.org/process/util;1"].
                         createInstance(AUS_Ci.nsIProcess);
  fileInUseProcess.init(fileInUseBin);
  fileInUseProcess.run(false, args, args.length);

  setupAppFilesAsync();
}

function setupAppFilesFinished() {
  do_timeout(TEST_HELPER_TIMEOUT, waitForHelperSleep);
}

function doUpdate() {
  runUpdateUsingService(STATE_PENDING_SVC, STATE_SUCCEEDED);
}

function checkUpdateFinished() {
  setupHelperFinish();
}

function checkUpdate() {
  checkFilesAfterUpdateSuccess();
  checkUpdateLogContains(ERR_BACKUP_DISCARD);
  checkCallbackServiceLog();
}
