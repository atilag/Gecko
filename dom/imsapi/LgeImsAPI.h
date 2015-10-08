#ifndef LGE_IMS_API_H
#define LGE_IMS_API_H

#include "nsCOMPtr.h"
#include "nsISupportsImpl.h"
#include "nsIModule.h"
#include "nsIWeakReference.h"
#include <map>

#include "nsIPrefBranch.h"
#include "nsICategoryManager.h"
#include "nsIObserver.h"
#include "nsISettingsService.h"
#include "nsINetworkManager.h"
#include "nsIDOMEventListener.h"
#include "nsThreadUtils.h"
#include "nsILgeImsAPI.h"
#include "nsTArray.h"
#include "nsITimer.h"
#include "LgeImsAPI.h"
#include "ImsInterfaceImpl.h"
#include "nsIMobileConnectionService.h"
#include "NativeWrapper.h"
//

#define LgeImsAPI_CID \
  { 0x97e8f753, 0x9f7b, 0x4180, \
  { 0x96, 0x2c, 0x77, 0x40, 0x18, 0x6c, 0xcf, 0x4d}}

#define LgeImsAPI_ContractID "@mozilla.org/lgeImsApi;1"
using namespace android;
using namespace mozilla;
typedef struct {
  int32_t TimerID;
  int32_t Duration;
} stTimer;

class LgeImsAPI : public nsILgeImsAPI,
                  public nsIObserver,
                  public nsIDOMEventListener,
                  public nsIMobileConnectionListener
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIOBSERVER
  NS_DECL_NSILGEIMSAPI
  NS_DECL_NSIMOBILECONNECTIONLISTENER

  static LgeImsAPI* GetSingleton();
  LgeImsAPI();
  void InitListenerService();
  void HandleWifiSettingStatusChanged(const char16_t* aData);
  void HandleAirplaneModeStatusChanged(nsISupports* aSubject);
  bool HandleNetIfaceStateChange(nsINetworkInterface* aNetworkInterface);
  void HandleDataChanged(const char16_t* aData);
  const char* GetLocalAddress (int apnType, int ipVersion);
  int GetWifiState();
  int GetWifiDetailedState();
  NS_IMETHODIMP HandleEvent(nsIDOMEvent* aDOMEvent);
  nsresult MyEvent();
  int mTImerId;
  int mDelay;
  //
  int IMS_STOPED = 0;
  int IMS_STARTED = 1;
  int DATA_STATE_CONNECTED = 2;
  int DATA_STATE_DISCONNECTED = 0;
  int DATA_STATE_CONNECT_FAILED = 4;
  int DATA_STATE_IP_CHANGED = 5;
  int mImsDataState = -1;
  int mImsApnRequestState;
  int mDefaultDataState = -1;
  bool bPcscfOnConnection;
  //
  int APN_IMS = 1;
  int APN_INTERNET = 2;
  int APN_EMERGENCY = 9;
  //
  int APN_REQUEST_IDLE = 0;
  int APN_REQUEST_PENDING = 1;
  int APN_REQUEST_DONE = 2;
  int APN_REQUEST_BLOCKED = 3;
  bool Connect(int apnType);
  bool connectImsApn();
  bool disconnectImsApn();
  bool checkRoaming();
  bool mCloseImsTimeRun;
  long getDataCallStateByType(long aApnType);
  void onImsBootCompleted();
  bool isImsServiceStarted = false;
  bool mAlreadyPsiRead = false;
  bool GetPsiInfo();
  char mDataServiceState[64];
  char mRAT[64];
  char mRoaming[16];
  bool bAlreadyPdnReq = false;
  static void retryPcscfTimerCallback(nsITimer* aTimer, void* aClosure);
  void HandleRetryPcscfTimerEvent();
  void callHalTimer(int32_t iDelay, int32_t iTimerId);
  void StopHalTimer(int32_t iTimerId);
  void NotifyImsIncomingCall(nsString CallID);
  int mEmergecnyDataState = -1;
  bool connectEmergencyApn();
  bool disconnectEmergencyApn();
  bool mEmergencyAttached = false;
  static void retryDataAddObservers(nsITimer* aTimer, void* aClosure);
  void HandleRetryDataAddObservers();
  void notifyImsStatusToObserverService(int32_t iStatus);
  static void ImsWithoutSIMCard(nsITimer* aTimer, void* aClosure);
  void HandleImsWithoutSIMCard();

private:
  LgeImsAPI(const LgeImsAPI &);
  LgeImsAPI & operator = (const LgeImsAPI &);
  virtual ~LgeImsAPI();
  void Init();

  static LgeImsAPI* sSingleton;
  bool mWifiEnabled;
  bool mAirplaneModeEnabled;
  int mWifiState;
  int mWifiDetailedState;
  ImsInterfaceImpl* imsImpl;
  NativeWrapper* imsNativeWrapper;
  void PcscfUpdateConnect(int32_t iApnType);
  void notifyImsServiceStarted();
  void notifyToObserverService(nsString CallID);
  bool checkRadioTechnology();
  void sendConnectedEventToNative(int32_t iApnType);
  bool is1xRTT();
  bool is2G();
  bool is3G();
  bool bNotifyPcscfReceived = false;
  nsCOMPtr<nsITimer>  mRetryPcscfTimer;
  nsCOMPtr<nsIThread> imsXpcomThread;
  void startHalTimer(stTimer getVal);
  void callStopHalTimer(int32_t iTimerId);
  std::map<int32_t, int32_t> mAlarmTimerId;
  uint32_t mClientId;
  int32_t iCurrentApnType = 0;
  nsCOMPtr<nsITimer>  mRetryDataAddObserver;
  nsCOMPtr<nsITimer>  mImsWithoutSimCard;
};

class nsNetworkInterface : public nsINetworkInterface
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSINETWORKINTERFACE

  nsNetworkInterface();

private:
  virtual ~nsNetworkInterface();

protected:
  /*                    */
};

#endif //
