#ifndef APN_MANG_H
#define APN_MANG_H
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
#include "nsTArray.h"
#include "nsITimer.h"
#include "nsIMobileConnectionService.h"
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include "nsINetworkService.h"

using namespace android ;

using namespace mozilla;

class ApnMng
{
  public:
    ApnMng();
    ~ApnMng();
    void SetApnType(int32_t apnType);
    void SetConnectionState(int32_t iApnType, int32_t iState);
    int32_t GetConnectionState(int32_t iApnType);
    char* GetLocalAddress(int32_t ApnType, int32_t IPVersion);
    void SetLocalAddress(const char * mLocalAddress, int32_t iApnType);
    void SetPcscfAddress(nsTArray<nsString> &aPcfcsAddresses_ipv4, nsTArray<nsString> &aPcfcsAddresses_ipv6, int32_t iApnType);
    const char** GetPcscfAddress(int32_t ApnType, int32_t IPVersion);
    void SetName(int32_t ApnType, nsString mName);
    nsString GetName(int32_t ApnType);
    void SetGatewayCount(int32_t ApnType, int32_t iCount);
    int32_t GetGatewayCount(int32_t ApnType);
    void SetGateways(int32_t ApnType, char16_t **pGateWay, uint32_t GatewayCnt);
    void SetNotifyPcscfReceived(int32_t ApnType, bool bReceived);
    void sendConnectedEventToApnMang(int32_t iApnType, int32_t iState);
    char16_t **GetGateways(int32_t ApnType);
    void SetIsEnabledPreviously(int32_t iApnType, bool bEnabled);
    static ApnMng* GetApnMng();
    void SetInterfaceID(int32_t iApnType);
    int32_t GetInterfaceID(int32_t iApnType);

  private:
    static ApnMng* sApnMng;
    //
    int APN_IMS = 1;
    int APN_INTERNET = 2;
    int APN_EMERGENCY = 9;
};

class ImsApn
{
  public:
    ImsApn();
    ~ImsApn();
    int32_t ApnType;
    int32_t state;
    nsTArray<nsString> mPcfcsAddresses_ipv4;
    nsTArray<nsString> mPcfcsAddresses_ipv6;
    char * mLocalAddress;
    nsString mName;
    char16_t **mGateway;
    uint32_t GatewayCnt;
    static ImsApn* GetImsApn();
    bool bNotifyPcscfReceived = false;
    void sendConnectedEventToNative(int32_t iState);
    static void retryPcscfTimerCallback(nsITimer* aTimer, void* aClosure);
    void HandleRetryPcscfTimerEvent();
    bool isEnabledPreviously = false;
    int32_t nIfaceId = -1;
    class ImsApnCB : public nsIGetNetIdCallback
    {
       public:
         NS_DECL_ISUPPORTS
         NS_DECL_NSIGETNETIDCALLBACK
         ImsApnCB();
         ~ImsApnCB();
    };
    ImsApnCB *CBObj;

  private:
    static ImsApn* sImsApn;
    int APN_IMS = 1;
    int APN_INTERNET = 2;
    int APN_EMERGENCY = 9;
    //
    int DATA_STATE_CONNECTED = 2;
    int DATA_STATE_DISCONNECTED = 0;
    int DATA_STATE_CONNECT_FAILED = 4;
    int DATA_STATE_IP_CHANGED = 5;
    nsCOMPtr<nsITimer>  mRetryPcscfTimer;
    nsCOMPtr<nsIThread> imsApnThread;
};

class EmergencyApn
{
  public:
    EmergencyApn();
    ~EmergencyApn();
    int32_t ApnType;
    int32_t state;
    nsTArray<nsString> mPcfcsAddresses_ipv4;
    nsTArray<nsString> mPcfcsAddresses_ipv6;
    char * mLocalAddress;
    nsString mName;
    char16_t **mGateway;
    uint32_t GatewayCnt;
    static EmergencyApn* GetEmergencyApn();
    bool bNotifyPcscfReceived = false;
    void sendConnectedEventToNative(int32_t iState);
    static void retryPcscfTimerCallback(nsITimer* aTimer, void* aClosure);
    void HandleRetryPcscfTimerEvent();
    bool isEnabledPreviously = false;
    int32_t nIfaceId = -1;
    class EmergencyApnCB : public nsIGetNetIdCallback
    {
       public:
         NS_DECL_ISUPPORTS
         NS_DECL_NSIGETNETIDCALLBACK
         EmergencyApnCB();
         ~EmergencyApnCB();
    };
    EmergencyApnCB *CBObj;

  private:
    static EmergencyApn* sEmergencyApn;
    int APN_IMS = 1;
    int APN_INTERNET = 2;
    int APN_EMERGENCY = 9;
    //
    int DATA_STATE_CONNECTED = 2;
    int DATA_STATE_DISCONNECTED = 0;
    int DATA_STATE_CONNECT_FAILED = 4;
    int DATA_STATE_IP_CHANGED = 5;
    nsCOMPtr<nsITimer>  mRetryPcscfTimer;
    nsCOMPtr<nsIThread> emergencyApnThread;
};

#endif //
