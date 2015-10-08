#ifndef LGE_IMS_INTERFACE_H
#define LGE_IMS_INTERFACE_H

#include "UC/UCInterface.h"

#include "nsThreadUtils.h"
#include "nsComponentManagerUtils.h"
#include "nsITimer.h"
#include <map>
#include <utils/Log.h>
#include <dlfcn.h>
#include "nsIIccInfo.h"
#include "nsTArray.h"
#include <binder/IInterface.h>
#include <binder/Parcel.h>

using namespace android ;

using namespace std;

namespace mozilla {
#define APN_STR_IMS "mobile_ims"
#define APN_STR_INTERNET "mobile_internet"
#define APN_STR_EMERGENCY "mobile_emergency"
enum
{
  NETWORK_TYPE_UNKNOWN = 0,
  NETWORK_TYPE_GPRS = 1,
  NETWORK_TYPE_EDGE = 2,
  NETWORK_TYPE_UMTS = 3,
  NETWORK_TYPE_CDMA = 4,
  NETWORK_TYPE_EVDO_0 = 5,
  NETWORK_TYPE_EVDO_A = 6,
  NETWORK_TYPE_1xRTT = 7,
  NETWORK_TYPE_HSDPA = 8,
  NETWORK_TYPE_HSUPA = 9,
  NETWORK_TYPE_HSPA = 10,
  NETWORK_TYPE_IDEN = 11,
  NETWORK_TYPE_EVDO_B = 12,
  NETWORK_TYPE_LTE = 13,
  NETWORK_TYPE_EHRPD = 14,
  NETWORK_TYPE_HSPAP = 15
};
class ImsInterfaceImpl : public BnUCInterface,  public IBinder::DeathRecipient
{
  public :
    ImsInterfaceImpl();
    virtual ~ImsInterfaceImpl();

   void binderDied(const wp<IBinder>&);
   virtual void onMessage(const android::Parcel &pParcel);
   virtual void onMessageEx(const android::Parcel &pParcel, Parcel *pOutParcel);
    void onMessageInternal(const android::Parcel& pParcel);
    void registrationConnected();
    void registrationDisconnected(const android::Parcel& pParcel);
    void notifyIncomingCall(int phoneID, int serviceID, const char* callId);
    void enteredECBM();
    void exitedECBM();
    void set911State(int32_t state, int32_t ecbm);
    void StartTimer(long timerId, int32_t delay);
    void StopTimer(long timerId);
    void PowerOff();
    void Reboot();
    const char* GetLocalAddress(int nApnType,int nIPVersion);
    int GetWifiState();
    int GetWifiDetailedState();
    void SendEvent( int nEvent, int nWParam, int nLParam );
    const char* GetPhoneNumber();
    const char* GetSubscriberId();
    const char* GetDeviceSoftwareVersion();
    const char* GetApn(int32_t nApnType);
    const char* GetDeviceID();
    const char** getPcscfAddress(int nApnType, int nIPVersion );
    int RequestRouteToHostAddress(int nApnType, char* strHostAddress);
    int DisableDataConnectivity(int nApnType);
    const char* GetMcc(bool fromSIM);
    const char* GetMnc(bool fromSIM);
    bool GetRoamingState();
    int GetNetworkType();
    int getDataState(int nApnType);
    int GetServiceState();
    bool enableDataConnectivity(int nApnType);
    static void sStartTimerCallback(nsITimer* aTimer, void* aESM );
    void HandleTimerExpireEvent(nsITimer* aTimer);
    void startRetryPsiTimer(int duration);
    static void retryPsiTimerCallback(nsITimer* aTimer, void* aClosure);
    void HandleRetryPsiTimerEvent(nsITimer* aTimer);
    int nGetPsiInfoRetryCnt = 0;
    void closeImsForaPeriodofTime(int duration);
    void disconnectImsApn(int intervalForApnBlock);
    void startCloseImsPdnTimer(int duration);
    void stopCloseImsPdnTimer(bool stopRequired);
    static void detachCloseImsPdnTimerCallback(nsITimer* aTimer, void* aClosure);
    void handleCloseImsPdnTimerCallback();
    virtual const char** getAccessNetworkInfo();
    void startRetryGetAccessInfoTimer(int duration);
    static void retryGetAccessInfoCallback(nsITimer* aTimer, void* aClosure);
    void HandleGetAccessInfoTimerEvent(nsITimer* aTimer);
    void SendEventInternal(int nEvent, int nWParam, int nLParam);
    void notifyEvent(int32_t nEvent, int32_t nWParam, int32_t nLParam);
    int32_t GetIfaceId(int32_t nApnType);
    //
    int APN_IMS = 1;
    int APN_INTERNET = 2;
    int APN_EMERGENCY = 9;

  private:
    std::map<long, nsCOMPtr<nsITimer>> mTimerObjectMap;
    nsCOMPtr<nsITimer>  mImsTimer;
    nsCOMPtr<nsIThread> xpcomThread;
    void SetImsRegStatus(bool bStatus);
    void SetImsStatusForDan(int iStatus);
    nsCOMPtr<nsITimer>  mCloseImsPdnTimer;
    nsCOMPtr<nsITimer>  mRetryPsiTimer;
    bool bPsiTimerRunning;
    nsCOMPtr<nsITimer>  mRetryPanInfoTimer;
    nsString mPrevLteInfo;
    nsString mPrevEhrpdInfo;
    const char **splitAccessInfo(const char *srcString, char delimeter);
    int mRegisteredEvent;
    bool isEventRegistered(int event);
//

/*                                                                      */
    void sendDefaultAttachProfile(int profileId);
/*                                                                    */

};
}
#endif //
