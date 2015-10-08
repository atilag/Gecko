/*








                                                                   */
#ifndef __IMSMANAGER_H_
#define __IMSMANAGER_H_

#include <string.h>
#include <strings.h>
#include "nsCOMPtr.h"
#include "nsISupportsImpl.h"
#include "nsIModule.h"
#include "nsIWeakReference.h"
#include <map>

#include "nsIPrefBranch.h"
#include "nsICategoryManager.h"
#include "nsIObserver.h"
#include "nsISettingsService.h"
#include "nsIDOMEventListener.h"
#include "nsThreadUtils.h"
#include "nsTArray.h"

#include "nsILgeImsUCManager.h"

#include "IImsService.h"
#include "ImsCallProfile.h"
#include "ImsCallSession.h"

#include <binder/IInterface.h>
#include "UCInterface.h"
#include "ImsEcbm.h"

using namespace android ;

class ImsManager : public BnUCInterface,
    public nsIImsUCManager, public IBinder::DeathRecipient
{
private:
  static ImsManager* m_pIUCIManagerObj;
  nsIImsConnectionStateListener* m_pImsManagerListener;
  int m_nServiceID;
  sp<IImsService> mImsService = NULL;
  ImsEcbm* mEcbm;

public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIIMSUCMANAGER

  ImsManager();
  virtual ~ImsManager();

  static ImsManager* getInstance();
  static void deleteInstance();
  virtual void binderDied(const wp<IBinder>& who);

  virtual void onMessage(const android::Parcel &pParcel);
  virtual void onMessageEx(const android::Parcel &pParcel, Parcel *pOutParcel);
 //
  void registrationConnected();
  void registrationDisconnected(const android::Parcel &pParcel);
  void notifyIncomingCall(int phoneID, int serviceID, const char* callId);
  void enteredECBM();
  void exitedECBM();
  void checkAndThrowExceptionIfServiceUnavailable();
  void set911State(int32_t state, int32_t ecbm);
private:
  bool createImsService();
  ImsCallSession* createCallSession(int serviceId,ImsCallProfile *profile);
};
#endif //
