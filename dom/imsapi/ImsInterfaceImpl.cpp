#include <time.h>
#include "ImsInterfaceImpl.h"
#include "LgeImsAPI.h"
#undef LOG_TAG
#define LOG_TAG "IMS_INTERFACE_IMPL"
#include <utils/Log.h>
#include "nsIPowerManagerService.h"
#include "nsComponentManagerUtils.h"
#include "ImsEventConstantMap.h"
#include "nsIRadioInterfaceLayer.h"
#include "nsServiceManagerUtils.h"
#include "nsIMobileNetworkInfo.h"
#include "nsIMobileConnectionInfo.h"
#include <android/log.h>
//
#include "mozilla/Services.h"
#include "nsContentUtils.h"
//
#include "nsINetworkService.h"
#include "nsIThread.h"
#include "nsXULAppAPI.h"
#include "jsfriendapi.h"
#include "nscore.h"
//
#include <binder/IInterface.h>
#include <binder/IServiceManager.h>
#include <binder/Parcel.h>
#include "mozilla/dom/network/NetUtils.h"
#include "nsIMobileConnectionService.h"
#include "IXPCOMCoreInterface.h"
#include <utils/String8.h>
#include "ApnMang.h"

#define LOGI(args...) \
   __android_log_print(ANDROID_LOG_INFO, LOG_TAG, ## args)

#define LOGE(args...) \
   __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, ## args)

using namespace mozilla;
using namespace mozilla::dom;


void
ImsInterfaceImpl::notifyEvent(int32_t nEvent, int32_t nWParam, int32_t nLParam)
{
  LOGE("LgeImsAPI::notifyEvent is called");

  if (!isEventRegistered(nEvent)) {
    LOGE("Event ( %d) is not registered", nEvent);
    return;
  }
    Parcel parcel ;
    parcel.writeInt32(nEvent);
    parcel.writeInt32(nWParam);
    parcel.writeInt32(nLParam);

  NativeWrapper* imsNativeWrapperImpl = NULL;
  imsNativeWrapperImpl = NativeWrapper::GetInstance();

  imsNativeWrapperImpl->sendData2NativeImpl(NOTIFY_EVENT, parcel);

}

class ImsRunnable : public nsRunnable
{
public:
  ImsRunnable(int nAPNType, char* strHostAddress)
    : mAPNType(nAPNType)
    , mstrHostAddress(strHostAddress)
  { }
  //
  int APN_IMS = 1;
  int APN_INTERNET = 2;
  int APN_EMERGENCY = 9;

  NS_IMETHOD Run()
  {
  LOGE("ImsRunnable  is called");

  //
#if 0
//
     JSRuntime *mRuntime;
     JSContext *aCx;
     int Result = 1;
     mRuntime = JS_NewRuntime(768 * 1024);
     NS_ENSURE_TRUE(mRuntime, NS_ERROR_OUT_OF_MEMORY);

     JS_SetNativeStackQuota(mRuntime, 128 * sizeof(size_t) * 1024);

     aCx = JS_NewContext(mRuntime, 0);
     NS_ENSURE_TRUE(aCx, NS_ERROR_OUT_OF_MEMORY);
     LOGE("ImsRunnable aCx created");
     char16_t **nGateway = (char16_t**)malloc(sizeof(char16_t*) * agent->GatewayCnt + 1);

     for(int iCount = 0; iCount < agent->GatewayCnt; iCount++){
       nGateway[iCount] = NS_strdup(agent->mGateway[iCount]);
     }
     nsCOMPtr<nsINetworkService> gNetworkService(do_GetService("@mozilla.org/network/service;1"));
     nsresult rv;
     JS::AutoValueArray<1> str(aCx);

     JS::Rooted<JSString*> string(aCx);
     JS::Rooted<JSObject*> GatewayAddrArray(aCx);
     JS::Rooted<JS::Value> params_addr(aCx);
     JS::Rooted<JS::Value> params_addr2(aCx);

     LOGE("temp.Assign  is called");
     //
     nsString temp;
     temp.Assign(agent->mGateway[0]);
     NS_LossyConvertUTF16toASCII converGateway(agent->mGateway[0]);
     LOGE("IMS Network Gateway is %s", converGateway.get());

     NS_LossyConvertUTF16toASCII converTemp(temp);
     LOGE("IMS Network Gateway is %s", converTemp.get());

     string = JS_NewStringCopyN(aCx, converGateway.get(),
                                          strlen(converGateway.get()));

     LOGE("JS_NewUCStringCopyN is called");
     //


     LOGE("After JS_NewUCStringCopyN is called");
     str[0].setString(string);

     LOGE("Before calling JS_NewArrayObject");

     GatewayAddrArray = JS_NewArrayObject(aCx, str);
     params_addr.setObject(*GatewayAddrArray);

     //
     nsNetworkInterface *alnetwork = new nsNetworkInterface();

     if(agent->mGateway[0]) free(agent->mGateway[0]);
     agent->mGateway[0] = NS_strdup(MOZ_UTF16("::0"));
    NS_LossyConvertUTF16toASCII agentmgateway(agent->mGateway[0]);
     LOGE("Before calling AddHostRouteWithResolve, agent->mGateway[0]=%s", agentmgateway.get());

     rv = gNetworkService->AddHostRouteWithResolve(static_cast<nsINetworkInterface*>(alnetwork), params_addr);
     if (NS_FAILED(rv)) {
       Result = 0;
     }

     LOGE("AddHostRouteWithResolve is Called First time");
     JS::Rooted<JSObject*> PcscfAddrArray(aCx);

     nsString nspcscfAddr;
     nspcscfAddr.AssignASCII(mstrHostAddress);
     LOGE("added pcscf");
     string = JS_NewUCStringCopyN(aCx, nspcscfAddr.get(), nspcscfAddr.Length());
     str[0].setString(string);
     LOGE("JS_NewUCStringCopyN :: string");
     PcscfAddrArray = JS_NewArrayObject(aCx, str);
     params_addr2.setObject(*PcscfAddrArray);
     LOGE("params_addr2.setObject");
#if 0
     for(int iCount = 0; iCount < agent->GatewayCnt; iCount++){
       free(agent->mGateway[iCount]);
     }
     LOGE("free(agent->mGateway[iCount])");
     free(agent->mGateway);
     LOGE("free(agent->mGateway)");
#endif
     LOGE("agent->mGateway = nGateway");
     agent->mGateway[0] = NS_strdup(nGateway[0]);
     LOGE("AddHostRouteWithResolve is Called second time");
     rv = gNetworkService->AddHostRouteWithResolve(static_cast<nsINetworkInterface*>(alnetwork), params_addr2);
     if (NS_FAILED(rv)) {
       Result = 0;
     }
#else
    NetUtils* mNetUtils = new NetUtils();
    ApnMng* Apn = ApnMng::GetApnMng();
    if(nullptr == Apn)
      LOGI("GetApnMng fail");
    char16_t **mGateway = nullptr;
    if(mAPNType == APN_IMS) {
      NS_ConvertUTF16toUTF8 ifname(Apn->GetName(nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS));
      mGateway = Apn->GetGateways(nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS);
      NS_ConvertUTF16toUTF8 converGateway(mGateway[0]);
      nsString nspcscfAddr;
      nspcscfAddr.AssignASCII(mstrHostAddress);
      NS_ConvertUTF16toUTF8 coverPcscf(nspcscfAddr);
      LOGE("Add Route %s %s", ifname.get(), converGateway.get());
      mNetUtils->do_ifc_add_route(ifname.get(), converGateway.get(), 128, NULL);
      LOGE("Add Route %s %s %s", ifname.get(), coverPcscf.get(), converGateway.get());
      mNetUtils->do_ifc_add_route(ifname.get(), coverPcscf.get(), 128, converGateway.get());
    } else if(mAPNType == APN_EMERGENCY){
      NS_ConvertUTF16toUTF8 ifname(Apn->GetName(nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY));
      mGateway = Apn->GetGateways(nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY);
      NS_ConvertUTF16toUTF8 converGateway(mGateway[0]);
      nsString nspcscfAddr;
      nspcscfAddr.AssignASCII(mstrHostAddress);
      NS_ConvertUTF16toUTF8 coverPcscf(nspcscfAddr);
      LOGE("Add Route %s %s", ifname.get(), converGateway.get());
      mNetUtils->do_ifc_add_route(ifname.get(), converGateway.get(), 128, NULL);
      LOGE("Add Route %s %s %s", ifname.get(), coverPcscf.get(), converGateway.get());
      mNetUtils->do_ifc_add_route(ifname.get(), coverPcscf.get(), 128, converGateway.get());
    }
#endif
     free(mstrHostAddress);
     LOGE("ImsRunnable is End");
     return NS_OK;
  }

private:
  int mAPNType;
  char* mstrHostAddress;
};

class IMSToImsInterfaceRunnable: public nsRunnable {
public:
  IMSToImsInterfaceRunnable(ImsInterfaceImpl* interface, int Event, int WParam, int LParam) :
    mInterface(interface) {
    nEvent = Event;
    nWParam = WParam;
    nLParam = LParam;
  }

  NS_IMETHOD Run() {
    if (mInterface)
      mInterface->SendEventInternal(nEvent, nWParam, nLParam);
    return NS_OK;
  }
private:
  ImsInterfaceImpl* mInterface;
  int nEvent;
  int nWParam;
  int nLParam;
};

using namespace android;
ImsInterfaceImpl::ImsInterfaceImpl()
{
  NS_GetCurrentThread(getter_AddRefs(xpcomThread));
  if(xpcomThread == nullptr)
  {
    LOGE("ImsInterfaceImpl::ImsInterfaceImpl::xpcomThread == null");
  }
  nGetPsiInfoRetryCnt = 0;
  bPsiTimerRunning = false;
  mCloseImsPdnTimer = nullptr;
  mRegisteredEvent = 0;
}

ImsInterfaceImpl::~ImsInterfaceImpl()
{
  xpcomThread = nullptr;
}

void
ImsInterfaceImpl::StartTimer(long timerId, int32_t delay)
{
  nsresult rv;
  if(delay < 400000)
  {
  mImsTimer = do_CreateInstance("@mozilla.org/timer;1");
  if (!mImsTimer) {
    LOGE("ImsInterfaceImpl::StartTimer::mImsTimer == null");
    return;
  }

  nsIEventTarget* target = static_cast<nsIEventTarget*>(xpcomThread);
  rv = mImsTimer->SetTarget(target);
  if(NS_FAILED(rv)) {
    LOGE("ImsInterfaceImpl::StartTimer::SetTarget failed");
    return;
  }
  rv = mImsTimer->InitWithFuncCallback(sStartTimerCallback, this, delay, nsITimer::TYPE_ONE_SHOT);
  if(NS_FAILED(rv)) {
    LOGE("ImsInterfaceImpl::StartTimer::InitWithFuncCallback failed") ;
    return;
  }
  LOGE("ImsInterfaceImpl::StartTimer::nsITimer timer ID is %lu",timerId) ;
      mTimerObjectMap.insert ( std::pair<long,nsCOMPtr<nsITimer>>(timerId,mImsTimer) );
  }else{
      LOGE("ImsInterfaceImpl::StartTimer::ImsAlarmservice") ;
      LgeImsAPI* agent = LgeImsAPI::GetSingleton();

      if (!agent) {
        LOGE("ImsInterfaceImpl::GetLocalAddress:: agent == null");
      return ;
    }
    agent->callHalTimer(delay,timerId);
  }
}

void
ImsInterfaceImpl::StopTimer(long timerId)
{
  if (mTimerObjectMap.find(timerId) != mTimerObjectMap.end()) {
    nsCOMPtr<nsITimer> timer = mTimerObjectMap[timerId];
    if (!timer) {
      LOGE("ImsInterfaceImpl::StopTimer::timer == Null");
      return;
    }
    timer->Cancel();
    timer = nullptr;
    mTimerObjectMap.erase(timerId);
      LOGE("ImsInterfaceImpl::StopTimer::nsITimer timer ID is %lu",timerId) ;
  }
  else
  {
    LOGE("ImsInterfaceImpl::StopTimer::ImsAlarmservice") ;
    LgeImsAPI* agent = LgeImsAPI::GetSingleton();
    if (!agent) {
      LOGE("ImsInterfaceImpl::GetLocalAddress:: agent == null");
      return ;
    }
    agent->StopHalTimer(timerId);
  }
}

void
ImsInterfaceImpl::PowerOff()
{
  nsCOMPtr<nsIPowerManagerService> aPowerOff;
  aPowerOff = do_CreateInstance("@mozilla.org/power/powermanagerservice;1");
  aPowerOff->PowerOff();
  aPowerOff = nullptr;
}

void
ImsInterfaceImpl::Reboot()
{
  nsCOMPtr<nsIPowerManagerService> aReboot;
  aReboot = do_CreateInstance("@mozilla.org/power/powermanagerservice;1");
  aReboot->Reboot();
  aReboot = nullptr;
}

void
ImsInterfaceImpl::sStartTimerCallback(nsITimer *aTimer, void* aESM)
{
  ImsInterfaceImpl* self = static_cast<ImsInterfaceImpl*>(aESM);
  if (self) {
    LOGE("ImsInterfaceImpl::sStartTimerCallback is called inside self");
    if (aTimer) {
      self->HandleTimerExpireEvent(aTimer);
      aTimer->Cancel();
      aTimer = nullptr;
    }
    self = nullptr;
  }
}

void
ImsInterfaceImpl::HandleTimerExpireEvent(nsITimer* aTimer)
{
  std::map<long, nsCOMPtr<nsITimer>>::iterator it;
  for (it = mTimerObjectMap.begin(); it != mTimerObjectMap.end(); ++it )
    if (it->second == aTimer)
      break;
  if(it == mTimerObjectMap.end()) {
    LOGE("ImsInterfaceImpl::HandleTimerExpireEvent:: end of timer object array");
    return;
  }
  LOGE("ImsInterfaceImpl::HandleTimerExpireEvent is called for Timer ID :%lu ", it->first);
  //
  android::Parcel parcel;
  parcel.writeInt64(it->first);
  NativeWrapper* imsNativeWrapperImpl = NULL;
  imsNativeWrapperImpl = NativeWrapper::GetInstance();
  imsNativeWrapperImpl->sendData2NativeImpl(NOTIFY_ALARM_EXPIRED, parcel);

  aTimer->Cancel();
  aTimer = nullptr;
  mTimerObjectMap.erase(it->first);
}

const char*
ImsInterfaceImpl::GetLocalAddress(int nApnType,  int nIPVersion )
{
  LgeImsAPI* agent = LgeImsAPI::GetSingleton();

  if (!agent) {
    LOGE("ImsInterfaceImpl::GetLocalAddress:: agent == null");
    return nullptr;
  }
  return agent->GetLocalAddress(nApnType,nIPVersion);
}

int
ImsInterfaceImpl::GetWifiState()
{
  LgeImsAPI* agent = LgeImsAPI::GetSingleton();
  if (!agent) {
    LOGE("ImsInterfaceImpl::GetWifiState:: agent == null");
    return 0;
  }
  return agent->GetWifiState();
}

int
ImsInterfaceImpl::GetWifiDetailedState()
{
  LgeImsAPI* agent = LgeImsAPI::GetSingleton();
  if (!agent) {
    LOGE("ImsInterfaceImpl::GetWifiDetailedState:: agent == null");
    return 0;
  }
  return agent->GetWifiDetailedState();
}

const char*
ImsInterfaceImpl::GetApn(int32_t nApnType)
{
  LOGE("ImsInterfaceImpl::GetApnString:: - %d", nApnType);
  nsCOMPtr<nsIRadioInterfaceLayer> gRIL(do_GetService("@mozilla.org/ril;1"));
  nsCOMPtr<nsIRadioInterface> radioInterface;
  nsString strApn;
  if (gRIL) {
    gRIL->GetRadioInterface(0, getter_AddRefs(radioInterface));
  }
  if(nApnType == APN_IMS) {
    radioInterface->GetActiveImsApnStringIF(strApn);
    NS_LossyConvertUTF16toASCII convertAPN(strApn);
    LOGE("GetApn = %s", convertAPN.get());
    if(convertAPN.Length() > 0){
      const char* pAPN = NULL;
      pAPN = (const char *)malloc(convertAPN.Length()+1);
      if(pAPN == NULL)
        return NULL;
      memset((char*)pAPN, 0, convertAPN.Length()+1);
      strcpy((char*)pAPN, convertAPN.get());
      return pAPN;
    }else{
      return NULL;
    }
  } else if(nApnType == APN_EMERGENCY) {
    radioInterface->GetActiveEmergencyApnStringIF(strApn);
    NS_LossyConvertUTF16toASCII convertAPN(strApn);
    LOGE("GetApn = %s", convertAPN.get());
    if(convertAPN.Length() > 0){
      const char* pAPN = NULL;
      pAPN = (const char *)malloc(convertAPN.Length()+1);
      if(pAPN == NULL)
        return NULL;
      memset((char*)pAPN, 0, convertAPN.Length()+1);
      strcpy((char*)pAPN, convertAPN.get());
      return pAPN;
    }else{
      return NULL;
    }
  } else {
    LOGE("ImsInterfaceImpl::GetApnString:: not handle");
  }
  return nullptr;
}


const char*
ImsInterfaceImpl::GetPhoneNumber()
{
  nsCOMPtr<nsIRadioInterfaceLayer> gRIL(do_GetService("@mozilla.org/ril;1"));
  nsCOMPtr<nsIRadioInterface> radioInterface;
  nsString Phonenum;
  if (gRIL) {
    gRIL->GetRadioInterface(0, getter_AddRefs(radioInterface));
  }
  radioInterface->GetPhoneInfoIF(1, Phonenum);
  NS_LossyConvertUTF16toASCII convertPhone(Phonenum);
  LOGE("GetPhoneNumber = %s", convertPhone.get());
  if(convertPhone.Length() > 0){
    const char* pPhoneNumber = NULL;
    pPhoneNumber = (const char *)malloc(convertPhone.Length()+1);
    if(pPhoneNumber == NULL)
      return NULL;
    memset((char*)pPhoneNumber, 0, convertPhone.Length()+1);
    strcpy((char*)pPhoneNumber, convertPhone.get());
    return pPhoneNumber;
  }else{
    return NULL;
  }
  return NULL;
}

int
ImsInterfaceImpl::GetServiceState()
{
  LgeImsAPI* agent = LgeImsAPI::GetSingleton();
  LOGE("GetServiceState() = %s = ", agent->mDataServiceState);
  if(strcmp(agent->mDataServiceState, "'registered'") == 0)
    return STATE_IN_SERVICE;
  else
    return STATE_OUT_OF_SERVICE;
}

const char*
ImsInterfaceImpl::GetSubscriberId()
{
  nsCOMPtr<nsIRadioInterfaceLayer> gRIL(do_GetService("@mozilla.org/ril;1"));
  nsCOMPtr<nsIRadioInterface> radioInterface;
  nsString imsi;

  if (gRIL) {
    gRIL->GetRadioInterface(0, getter_AddRefs(radioInterface));
  }
  radioInterface->GetPhoneInfoIF(2, imsi);
  NS_LossyConvertUTF16toASCII convertImsi(imsi);
  LOGE("GetSubscriberId = %s = ", convertImsi.get());
  if(convertImsi.Length() > 0){
    const char* pSubcribeId = NULL;
    pSubcribeId = (const char *)malloc(convertImsi.Length()+1);
    if(pSubcribeId == NULL)
      return NULL;
    memset((char*)pSubcribeId, 0, convertImsi.Length()+1);
    strcpy((char*)pSubcribeId, convertImsi.get());
    return pSubcribeId;
  }else{
    return NULL;
  }
  return NULL;
}

const char*
ImsInterfaceImpl::GetDeviceSoftwareVersion()
{
  //
  return "V01a";
}

const char*
ImsInterfaceImpl::GetDeviceID()
{
  nsCOMPtr<nsIRadioInterfaceLayer> gRIL(do_GetService("@mozilla.org/ril;1"));
  nsCOMPtr<nsIRadioInterface> radioInterface;
  nsString DeviceId;
  if (gRIL) {
    gRIL->GetRadioInterface(0, getter_AddRefs(radioInterface));
  }
  radioInterface->GetPhoneInfoIF(4, DeviceId);
  NS_LossyConvertUTF16toASCII convertDeviceId(DeviceId);
  LOGE("GetDeviceID = %s", convertDeviceId.get());
  if(convertDeviceId.Length() > 0){
    const char* pDeviceId = NULL;
    pDeviceId = (const char *)malloc(convertDeviceId.Length()+1);
    if(pDeviceId == NULL)
      return NULL;
    memset((char*)pDeviceId, 0, convertDeviceId.Length()+1);
    strcpy((char*)pDeviceId, convertDeviceId.get());
    return pDeviceId;
  }else{
    return NULL;
  }
  return NULL;
}

const char**
ImsInterfaceImpl::getPcscfAddress(int apnType, int ipVersion)
{
#if 0
  nsTArray<nsString> mPcfcsAddresses;
  const char** mPcscfAddress = nullptr;
  LgeImsAPI* agent = LgeImsAPI::GetSingleton();
  if (!agent) {
    LOGE("ImsInterfaceImpl::getPcscfAddress:: agent == null");
    return nullptr;
  }
 //
  if(4 == ipVersion) {
    mPcfcsAddresses = agent->mPcfcsAddresses_ipv4;
  }else if(6 == ipVersion) {
    mPcfcsAddresses = agent->mPcfcsAddresses_ipv6;
  }else {
    return nullptr;
  }
  if(mPcfcsAddresses.Length()) {
    uint32_t iCount = 0;
    mPcscfAddress = (const char**)malloc(sizeof(char*) * mPcfcsAddresses.Length() + 1);
    memset((const char**)mPcscfAddress, 0, mPcfcsAddresses.Length() + 1);
    for(iCount = 0; iCount < mPcfcsAddresses.Length(); iCount++){
      NS_LossyConvertUTF16toASCII convertAddr(mPcfcsAddresses[iCount]);
      mPcscfAddress[iCount] = (char*)malloc(sizeof(char*) * strlen(convertAddr.get())+1);
      strcpy((char*)mPcscfAddress[iCount], convertAddr.get());
    }
    mPcscfAddress[iCount] = (char*)malloc(sizeof(char*) * 2);
    if(nullptr == mPcscfAddress[iCount])
      return nullptr;
    memset((char*)mPcscfAddress[iCount], 0, 2);
    memcpy((char*)mPcscfAddress[iCount], (char *)"\0", 1);
    return mPcscfAddress;
  }else {
    return nullptr;
  }
#endif
  ApnMng* Apn = ApnMng::GetApnMng();
  if(nullptr == Apn)
    LOGI("GetApnMng fail");
  return Apn->GetPcscfAddress(apnType, ipVersion);
}

void
ImsInterfaceImpl::SetImsStatusForDan(int iStatus)
{
  LOGI("ImsInterfaceImpl::SetImsStatusForDan");
  nsCOMPtr<nsIRadioInterfaceLayer> gRIL = (do_GetService("@mozilla.org/ril;1"));
  nsCOMPtr<nsIRadioInterface> radioInterface;
  if(gRIL) {
    gRIL->GetRadioInterface(0, getter_AddRefs(radioInterface));
  }else {
    LOGI("ImsInterfaceImpl::SetImsStatusForDan::gRIL is Null");
  }
  if(radioInterface) {
    radioInterface->SetImsStatusForDanIF(iStatus);
  }else {
    LOGI("ImsInterfaceImpl::SetImsStatusForDan::radioInterface is Null");
  }
}
void
ImsInterfaceImpl::SetImsRegStatus(bool bStatus)
{
  nsCOMPtr<nsIRadioInterfaceLayer> gRIL(do_GetService("@mozilla.org/ril;1"));
  nsCOMPtr<nsIRadioInterface> radioInterface;
  if(gRIL) {
    gRIL->GetRadioInterface(0, getter_AddRefs(radioInterface));
  }
  radioInterface->SetIMSRegistateIF(bStatus);
  //
  if(bStatus) {
    LgeImsAPI* agent = LgeImsAPI::GetSingleton();
    if (!agent) {
      LOGE("ImsInterfaceImpl::GetWifiState:: agent == null");
      return ;
    }
    if(!agent->mAlreadyPsiRead) {
      startRetryPsiTimer(2000);
    }
  }
}

/*                                                                 */
void
ImsInterfaceImpl::sendDefaultAttachProfile(int profileId)
{
  nsCOMPtr<nsIRadioInterfaceLayer> gRIL(do_GetService("@mozilla.org/ril;1"));
  nsCOMPtr<nsIRadioInterface> radioInterface;
  if(gRIL) {
    gRIL->GetRadioInterface(0, getter_AddRefs(radioInterface));
  }
  radioInterface->SendDefaultAttachProfileIF(profileId);
}
/*                                                               */

int
ImsInterfaceImpl::DisableDataConnectivity(int apnType)
{
  nsCOMPtr<nsIRadioInterfaceLayer> gRIL = (do_GetService("@mozilla.org/ril;1"));
  nsCOMPtr<nsIRadioInterface> radioInterface;

  if(gRIL) {
    gRIL->GetRadioInterface(0, getter_AddRefs(radioInterface));
  }
  if(apnType == 1) {
    LgeImsAPI* agent = LgeImsAPI::GetSingleton();
    if (!agent) {
      LOGE("ImsInterfaceImpl::GetWifiState:: agent == null");
      return 0;
    }
    agent->disconnectImsApn();
  }
  return 1;
}

const char*
ImsInterfaceImpl::GetMcc(bool fromSIM)
{
  LOGI("ImsInterfaceImpl::GetMcc");
  const char* pcMcc = nullptr;
  nsCOMPtr<nsIRadioInterfaceLayer> gRIL = (do_GetService("@mozilla.org/ril;1"));
  nsCOMPtr<nsIRadioInterface> radioInterface;
  if (gRIL) {
    gRIL->GetRadioInterface(0, getter_AddRefs(radioInterface));
    if(radioInterface){
      nsCOMPtr<nsIRilContext> rilCtx;
      radioInterface->GetRilContext(getter_AddRefs(rilCtx));
      if(rilCtx) {
        if(true == fromSIM) {
          nsCOMPtr<nsIIccInfo> iccInfo;
          rilCtx->GetIccInfo(getter_AddRefs(iccInfo));
          if(iccInfo) {
            nsString mcc;
            iccInfo->GetMcc(mcc);
            NS_LossyConvertUTF16toASCII convertMcc(mcc);
            pcMcc = (char*)malloc(sizeof(char*) * strlen(convertMcc.get())+1);
            if(pcMcc) {
              strcpy((char*)pcMcc, convertMcc.get());
            }
          }
        }else {
          nsCOMPtr<nsIMobileConnectionInfo> voice;
          //
          nsCOMPtr<nsIMobileConnectionService> service = do_GetService(NS_MOBILE_CONNECTION_SERVICE_CONTRACTID);
          if (!service) {
            NS_WARNING("Cannot get MobileConnectionService");
            return nullptr;
          }
          nsCOMPtr<nsIMobileConnection> connection;
          //
          //
          service->GetItemByServiceId(0 /*           */, getter_AddRefs(connection));
          connection->GetVoice(getter_AddRefs(voice));
          if(voice) {
            bool isConnected;
            voice->GetConnected(&isConnected);
            if(true == isConnected) {
              nsCOMPtr<nsIMobileNetworkInfo> networkInfo;
              voice->GetNetwork(getter_AddRefs(networkInfo));
              if(networkInfo) {
                nsString mcc;
                networkInfo->GetMcc(mcc);
                NS_LossyConvertUTF16toASCII convertMnc(mcc);
                pcMcc = (char*)malloc(sizeof(char*) * strlen(convertMnc.get())+1);
                if(pcMcc) {
                  strcpy((char*)pcMcc, convertMnc.get());
                }
              }
            }
          }
        }
      }
    }
  }
  LOGI("ImsInterfaceImpl::GetMcc::return");
  return pcMcc;
}

const char*
ImsInterfaceImpl::GetMnc(bool fromSIM)
{
  LOGI("ImsInterfaceImpl::GetMnc");
  const char* pcMnc = nullptr;
  nsCOMPtr<nsIRadioInterfaceLayer> gRIL = (do_GetService("@mozilla.org/ril;1"));
  nsCOMPtr<nsIRadioInterface> radioInterface;
  if(gRIL) {
    gRIL->GetRadioInterface(0, getter_AddRefs(radioInterface));
    if(radioInterface) {
      nsCOMPtr<nsIRilContext> rilCtx;
      radioInterface->GetRilContext(getter_AddRefs(rilCtx));
      if(rilCtx) {
        if(true == fromSIM) {
          nsCOMPtr<nsIIccInfo> iccInfo;
          rilCtx->GetIccInfo(getter_AddRefs(iccInfo));
          if(iccInfo) {
            nsString mnc;
            iccInfo->GetMnc(mnc);
            NS_LossyConvertUTF16toASCII convertAddr(mnc);
            pcMnc = (char*)malloc(sizeof(char*) * strlen(convertAddr.get())+1);
            if(pcMnc) {
              strcpy((char*)pcMnc, convertAddr.get());
            }
          }
        }else {
          nsCOMPtr<nsIMobileConnectionInfo> voice;
          //
          nsCOMPtr<nsIMobileConnectionService> service = do_GetService(NS_MOBILE_CONNECTION_SERVICE_CONTRACTID);
          if (!service) {
            NS_WARNING("Cannot get MobileConnectionService");
            return nullptr;
          }
          nsCOMPtr<nsIMobileConnection> connection;
          //
          //
          service->GetItemByServiceId(0 /*           */, getter_AddRefs(connection));
          connection->GetVoice(getter_AddRefs(voice));
          if(voice) {
            bool isConnected;
            voice->GetConnected(&isConnected);
            if(true == isConnected) {
              nsCOMPtr<nsIMobileNetworkInfo> networkInfo;
              voice->GetNetwork(getter_AddRefs(networkInfo));
              if(networkInfo) {
                nsString mnc;
                networkInfo->GetMnc(mnc);
                NS_LossyConvertUTF16toASCII convertAddr(mnc);
                pcMnc = (char*)malloc(sizeof(char*) * strlen(convertAddr.get())+1);
                if(pcMnc) {
                    strcpy((char*)pcMnc, convertAddr.get());
                }
              }
            }
          }
        }
      }
    }
  }
  LOGI("ImsInterfaceImpl::GetMnc::return");
  return pcMnc;
}

bool
ImsInterfaceImpl::GetRoamingState()
{
  LOGI("ImsInterfaceImpl::GetRoaming");
  bool bRoaming = false;
  bool bDataRoaming = false;
  nsCOMPtr<nsIMobileConnectionService> service = do_GetService(NS_MOBILE_CONNECTION_SERVICE_CONTRACTID);
  if (!service) {
    LOGI("Cannot get MobileConnectionService");
    return nullptr;
  }
  nsCOMPtr<nsIMobileConnection> connection;
  //
  //
  service->GetItemByServiceId(0 /*           */, getter_AddRefs(connection));

  if (connection) {
    nsCOMPtr<nsIMobileConnectionInfo> data;
    connection->GetData(getter_AddRefs(data));
    if(data) {
      bool isConnected;
      data->GetConnected(&isConnected);
      if(true == isConnected) {
        data->GetRoaming(&bDataRoaming);
      }
    }
  }
  bRoaming = bDataRoaming;
  LOGI("ImsInterfaceImpl::GetRoamingState::return");
  return bRoaming;
}

int
ImsInterfaceImpl::GetNetworkType()
{
  LOGI("ImsInterfaceImpl::GetNetworkType");
  char* pcNetworkType = nullptr;
  int iNetworkType = NETWORK_TYPE_UNKNOWN;
  LgeImsAPI* agent = LgeImsAPI::GetSingleton();
  if (!agent) {
    LOGE("ImsInterfaceImpl::GetNetworkType:: agent == null");
    return NETWORK_TYPE_UNKNOWN;
  }
  pcNetworkType = (char*)malloc(sizeof(char*) * strlen(agent->mRAT)+1);
  if(pcNetworkType) {
    //
    strcpy((char*)pcNetworkType, agent->mRAT);
    if(!strcmp(pcNetworkType,"'unknown'")) {
    iNetworkType = NETWORK_TYPE_UNKNOWN;
    }else if(!strcmp(pcNetworkType,"'gprs'")) {
    iNetworkType = NETWORK_TYPE_GPRS;
    }else if(!strcmp(pcNetworkType,"'edge'")) {
    iNetworkType = NETWORK_TYPE_EDGE;
    }else if(!strcmp(pcNetworkType,"'umts'")) {
    iNetworkType = NETWORK_TYPE_UMTS;
    }else if(!strcmp(pcNetworkType,"'is95a'")) {
    iNetworkType = NETWORK_TYPE_CDMA;
    }else if(!strcmp(pcNetworkType,"'is95b'")) {
    iNetworkType = NETWORK_TYPE_CDMA;
    }else if(!strcmp(pcNetworkType,"'1xrtt'")) {
    iNetworkType = NETWORK_TYPE_1xRTT;
    }else if(!strcmp(pcNetworkType,"'evdo0'")) {
    iNetworkType = NETWORK_TYPE_EVDO_0;
    }else if(!strcmp(pcNetworkType,"'evdoa'")) {
    iNetworkType = NETWORK_TYPE_EVDO_A;
    }else if(!strcmp(pcNetworkType,"'hsdpa'")) {
    iNetworkType = NETWORK_TYPE_HSDPA;
    }else if(!strcmp(pcNetworkType,"'hsupa'")) {
    iNetworkType = NETWORK_TYPE_HSUPA;
    }else if(!strcmp(pcNetworkType,"'hspa'")) {
    iNetworkType = NETWORK_TYPE_HSPA;
    }else if(!strcmp(pcNetworkType,"'evdob'")) {
    iNetworkType = NETWORK_TYPE_EVDO_B;
    }else if(!strcmp(pcNetworkType,"'ehrpd'")) {
    iNetworkType = NETWORK_TYPE_EHRPD;
    }else if(!strcmp(pcNetworkType,"'lte'")) {
    iNetworkType = NETWORK_TYPE_LTE;
    }else if(!strcmp(pcNetworkType,"'hspa+'")) {
    iNetworkType = NETWORK_TYPE_HSPAP;
    }else if(!strcmp(pcNetworkType,"'gsm'")) {
    iNetworkType = NETWORK_TYPE_UNKNOWN;
    }else {
    iNetworkType = NETWORK_TYPE_UNKNOWN;
    }
    free(pcNetworkType);
  }
  LOGI("ImsInterfaceImpl::GetNetworkType::return");
  return iNetworkType;
}
void
ImsInterfaceImpl::disconnectImsApn(int intervalForApnBlock)
{
  LgeImsAPI* agent = LgeImsAPI::GetSingleton();
  if (!agent) {
    LOGE("ImsInterfaceImpl::GetWifiState:: agent == null");
    return;
  }
  nsCOMPtr<nsIRadioInterfaceLayer> gRIL(do_GetService("@mozilla.org/ril;1"));
  nsCOMPtr<nsIRadioInterface> radioInterface;
  if(gRIL) {
    gRIL->GetRadioInterface(0, getter_AddRefs(radioInterface));
  }
  //
  agent->disconnectImsApn();
  agent->mImsApnRequestState = agent->APN_REQUEST_IDLE;
  agent->mImsDataState = agent->DATA_STATE_DISCONNECTED;
  //
//

  android::Parcel parcel;
  parcel.writeInt32(agent->APN_IMS);
  parcel.writeInt32(agent->DATA_STATE_DISCONNECTED);
  NativeWrapper* imsNativeWrapperImpl = NULL;
  imsNativeWrapperImpl = NativeWrapper::GetInstance();
  imsNativeWrapperImpl->sendData2NativeImpl(NOTIFY_DATA_CONNECTION_STATE_CHANGED, parcel);

}
void
ImsInterfaceImpl::startCloseImsPdnTimer(int duration)
{
  LOGE("ImsInterfaceImpl::startCloseImsPdnTimer");
  nsresult rv;
  mCloseImsPdnTimer = do_CreateInstance("@mozilla.org/timer;1");
  if (!mCloseImsPdnTimer) {
    LOGE("ImsInterfaceImpl::StartTimer::mCloseImsPdnTimer == null");
    return;
  }
  nsIEventTarget* target = static_cast<nsIEventTarget*>(xpcomThread);
  rv = mCloseImsPdnTimer->SetTarget(target);
  if(NS_FAILED(rv)) {
    LOGE("ImsInterfaceImpl::startCloseImsPdnTimer::SetTarget failed");
    return;
  }
  rv = mCloseImsPdnTimer->InitWithFuncCallback(detachCloseImsPdnTimerCallback,
                                    this,
                                    duration,
                                    nsITimer::TYPE_ONE_SHOT);
  if(NS_FAILED(rv)) {
    LOGE("ImsInterfaceImpl::startCloseImsPdnTimer::InitWithFuncCallback failed") ;
    return;
  }
}
void
ImsInterfaceImpl::stopCloseImsPdnTimer(bool stopRequired)
{
  if (!mCloseImsPdnTimer) {
    LOGE("ImsInterfaceImpl::StopTimer::mCloseImsPdnTimer == null");
    return;
  }
  if(stopRequired) {
    mCloseImsPdnTimer->Cancel();
    mCloseImsPdnTimer = nullptr;
 }

 //
}
void
ImsInterfaceImpl::detachCloseImsPdnTimerCallback(nsITimer* aTimer, void* aClosure)
{
    LOGE("ImsInterfaceImpl::detachCloseImsPdnTimerCallback");
  ImsInterfaceImpl* self = static_cast<ImsInterfaceImpl*>(aClosure);
  if (self) {
    self->handleCloseImsPdnTimerCallback();
    self->stopCloseImsPdnTimer(true);
    self = nullptr;
  }
}
void
ImsInterfaceImpl::handleCloseImsPdnTimerCallback()
{
  LOGE("ImsInterfaceImpl::handleCloseImsPdnTimerCallback") ;

  LgeImsAPI* agent = LgeImsAPI::GetSingleton();
  if (!agent) {
    LOGE("ImsInterfaceImpl::GetWifiState:: agent == null");
    return;
  }
  agent->mCloseImsTimeRun = false;
  agent->Connect(agent->APN_IMS);
}
void
ImsInterfaceImpl::closeImsForaPeriodofTime(int duration)
{
  LgeImsAPI* agent = LgeImsAPI::GetSingleton();
  if (!agent) {
    LOGE("ImsInterfaceImpl::GetWifiState:: agent == null");
    return;
  }
  agent->mCloseImsTimeRun = true;
  disconnectImsApn(0);
  startCloseImsPdnTimer(duration);
}

int
ImsInterfaceImpl::getDataState(int nApnType)
{
  LgeImsAPI* agent = LgeImsAPI::GetSingleton();
  if (!agent) {
    LOGE("ImsInterfaceImpl::GetWifiState:: agent == null");
    return -1;
  }
  if(nApnType == agent->APN_IMS) {
    return agent->mImsDataState;
  } else if(nApnType == agent->APN_INTERNET){
    return agent->mDefaultDataState;
  } else {
    return -1; //
  }
}
bool
ImsInterfaceImpl::enableDataConnectivity(int nApnType)
{
  LgeImsAPI* agent = LgeImsAPI::GetSingleton();
  if (!agent) {
    LOGE("ImsInterfaceImpl::GetWifiState:: agent == null");
    return false;
  }
  return agent->Connect(nApnType);
}
void
ImsInterfaceImpl::startRetryPsiTimer(int duration)
{
  LOGE("ImsInterfaceImpl::startRetryPsiTimer()");
  nsresult rv;
  if(bPsiTimerRunning == false) {
    mRetryPsiTimer = do_CreateInstance("@mozilla.org/timer;1");
    if (!mRetryPsiTimer) {
    LOGE("ImsInterfaceImpl::startRetryPsiTimer::mRetryPsiTimer == null");
    return;
    }
    nsIEventTarget* target = static_cast<nsIEventTarget*>(xpcomThread);
    rv = mRetryPsiTimer->SetTarget(target);
    if(NS_FAILED(rv)) {
    LOGE("ImsInterfaceImpl::startRetryPsiTimer::SetTarget failed");
    return;
    }
    rv = mRetryPsiTimer->InitWithFuncCallback(retryPsiTimerCallback,
                                    this,
                                    duration,
                                    nsITimer::TYPE_ONE_SHOT);
    if(NS_FAILED(rv)) {
    LOGE("ImsInterfaceImpl::startRetryPsiTimer::InitWithFuncCallback failed") ;
    return;
    }
    bPsiTimerRunning = true;
  }
}
void
ImsInterfaceImpl::retryPsiTimerCallback(nsITimer* aTimer, void* aClosure)
{
  LOGE("ImsInterfaceImpl::retryPsiTimerCallback()");
  ImsInterfaceImpl* self = static_cast<ImsInterfaceImpl*>(aClosure);
  if (self) {
    if(aTimer) {
      self->HandleRetryPsiTimerEvent(aTimer);
      LOGE("ImsInterfaceImpl::mRetryPsiTimer->Cancel()");
      aTimer->Cancel();
      aTimer = nullptr;
  //
    }
    self = nullptr;
  } else {
    LOGE("ImsInterfaceImpl::retryPsiTimerCallback()::Creating Object failed");
  }
}
void
ImsInterfaceImpl::HandleRetryPsiTimerEvent(nsITimer* aTimer)
{
  LOGE("ImsInterfaceImpl::HandleRetryPsiTimerEvent()");
  LgeImsAPI* agent = LgeImsAPI::GetSingleton();
  if (!agent) {
    LOGE("ImsInterfaceImpl::GetWifiState:: agent == null");
    return ;
  }
  bPsiTimerRunning = false;
  if(!agent->mAlreadyPsiRead) {
    if(false == agent->GetPsiInfo()) {
      nGetPsiInfoRetryCnt++;
      LOGE("LgeImsAPI::HandleRetryPsiTimerEvent():Start PSI retry timer count - %d", nGetPsiInfoRetryCnt);
      if (nGetPsiInfoRetryCnt < IMS_RETRY_GET_PSI_INFO_MAX_COUNT) {
        startRetryPsiTimer(2000);
      }
    }
  }
}
void
ImsInterfaceImpl::startRetryGetAccessInfoTimer(int duration)
{
  LOGE("ImsInterfaceImpl::startRetryGetAccessInfoTimer()");
  nsresult rv;
  mRetryPanInfoTimer = do_CreateInstance("@mozilla.org/timer;1");
  if (!mRetryPanInfoTimer) {
    LOGE("ImsInterfaceImpl::startRetryGetAccessInfoTimer::mRetryPanInfoTimer == null");
    return;
  }
  nsIEventTarget* target = static_cast<nsIEventTarget*>(xpcomThread);
  rv = mRetryPanInfoTimer->SetTarget(target);
  if(NS_FAILED(rv)) {
    LOGE("ImsInterfaceImpl::startRetryGetAccessInfoTimer::SetTarget failed");
    return;
  }
  rv = mRetryPanInfoTimer->InitWithFuncCallback(retryGetAccessInfoCallback,
                                this,
                                duration,
                                nsITimer::TYPE_ONE_SHOT);
  if(NS_FAILED(rv)) {
    LOGE("ImsInterfaceImpl::startRetryGetAccessInfoTimer::InitWithFuncCallback failed") ;
    return;
  }

}
void
ImsInterfaceImpl::retryGetAccessInfoCallback(nsITimer* aTimer, void* aClosure)
{
  LOGE("ImsInterfaceImpl::retryGetAccessInfoCallback()");
  ImsInterfaceImpl* self = static_cast<ImsInterfaceImpl*>(aClosure);
  if (self) {
    if(aTimer) {
      self->HandleGetAccessInfoTimerEvent(aTimer);
      LOGE("ImsInterfaceImpl::mRetryPsiTimer->Cancel()");
      aTimer->Cancel();
      aTimer = nullptr;
  //
    }
    self = nullptr;
  } else {
    LOGE("ImsInterfaceImpl::retryGetAccessInfoCallback()::Creating Object failed");
  }
}
void
ImsInterfaceImpl::HandleGetAccessInfoTimerEvent(nsITimer* aTimer)
{
  LOGE("ImsInterfaceImpl::HandleGetAccessInfoTimerEvent()");
  nsCOMPtr<nsIRadioInterfaceLayer> gRIL(do_GetService("@mozilla.org/ril;1"));
  nsCOMPtr<nsIRadioInterface> radioInterface;
  if(gRIL) {
    gRIL->GetRadioInterface(0, getter_AddRefs(radioInterface));
  }
  int iNetworkType = GetNetworkType();
  if(NETWORK_TYPE_LTE == iNetworkType) {
    nsString mLteInfo;
    nsString& LteInfo = mLteInfo;
    radioInterface->GetLteInfoIF(LteInfo);
    if(LteInfo.IsEmpty()) {
      LOGE("ImsInterfaceImpl::HandleGetAccessInfoTimerEvent()::LteInfo.IsEmpty");
      return;
    }
    NS_LossyConvertUTF16toASCII convertString(LteInfo);
    LOGE("LteInfo : %s" ,convertString.get());
    mPrevLteInfo.AssignASCII(convertString.get());
  } else if(NETWORK_TYPE_EHRPD == iNetworkType) {
    nsString mEhrpdInfo;
    nsString& EhrpdInfo = mEhrpdInfo;
    radioInterface->GetEhrpdInfoIF(EhrpdInfo);
    if(EhrpdInfo.IsEmpty()) {
      LOGE("ImsInterfaceImpl::HandleGetAccessInfoTimerEvent()::EhrpdInfo.IsEmpty");
      return;
    }
    NS_LossyConvertUTF16toASCII convertString(EhrpdInfo);
    LOGE("EhrpdInfo.IsEmpty : %s" ,convertString.get());
    mPrevEhrpdInfo.AssignASCII(convertString.get());
  } else {
    return ;
  }
}
char *
FindNextSubstring(const char *string, int *strSize, char delimeter) {
  LOGE("ImsInterfaceImpl::FindNextSubstring");
  if(nullptr == string){
    LOGE("ImsInterfaceImpl::FindNextSubstring::string is empty");
    return nullptr;
  }
  char *pTempStr = nullptr;
  char *subStr = nullptr;
  int size = 0;
  pTempStr = strchr(string, delimeter);
  if(nullptr == pTempStr){
    LOGE("ImsInterfaceImpl::FindNextSubstring::pTempStr is null");
    size = strlen(string);
    LOGE("ImsInterfaceImpl::FindNextSubstring::string - %s", string);
    LOGE("ImsInterfaceImpl::FindNextSubstring::size - %d", size);
  }else {
    LOGE("ImsInterfaceImpl::FindNextSubstring::string - %s", string);
    LOGE("ImsInterfaceImpl::FindNextSubstring::pTempStr - %s", pTempStr);
    size = pTempStr - string;
  }
  subStr = (char*)malloc(sizeof(char*) * (size+1));
  if(nullptr != subStr){
    memset((char *)subStr, 0, (size+1));
    memcpy((char *)subStr, (const char *)string, size);
  }
  LOGE("ImsInterfaceImpl::FindNextSubstring::subStr - %s", subStr);
  LOGE("ImsInterfaceImpl::FindNextSubstring::size - %d", size);
  *strSize = size;
  return subStr;
}

const char **
ImsInterfaceImpl::splitAccessInfo(const char *srcString, char delimeter)
{
  LOGE("ImsInterfaceImpl::splitAccessInfo");
  if(nullptr == srcString){
    LOGE("ImsInterfaceImpl::splitAccessInfo::srcString is empty");
    return nullptr;
  }
  int iCount = 0;
  int subStrSize = 0;
  int iLoopCount = 0;
  const char **desString = nullptr;
  LOGE("ImsInterfaceImpl::splitAccessInfo::srcString - %s", srcString);
  int iNetworkType = GetNetworkType();
  if(NETWORK_TYPE_LTE == iNetworkType){
    iLoopCount = 4;
  } else if (NETWORK_TYPE_EHRPD == iNetworkType) {
    iLoopCount = 2;
  }
  desString = (const char**)malloc(sizeof(char*) * 5);
  while (iCount < iLoopCount) {
    char *pSubStr = nullptr;
    pSubStr = FindNextSubstring(srcString, &subStrSize, delimeter);
    if(nullptr == pSubStr){
     LOGE("ImsInterfaceImpl::splitAccessInfo::nullptr == desString[iCount]");
      break;
    }
    desString[iCount] = (const char*)malloc(sizeof(char*) * (strlen(pSubStr)+1));
    if(nullptr == desString[iCount])
      return nullptr;
    memset((char *)desString[iCount], 0, (strlen(pSubStr)+1));
    strcpy((char *)desString[iCount], pSubStr);
    free(pSubStr);
    iCount++;
    srcString = srcString+(subStrSize + 1);
  }
  if(NETWORK_TYPE_LTE == iNetworkType){
 #if LGE_IMS_KDDI //
    desString[iCount] = (const char*)malloc(sizeof(char*) * (strlen("FDD")+1));
    if(nullptr == desString[iCount])
      return nullptr;
    memset((char *)desString[iCount], 0, (strlen("FDD")+1));
    strcpy((char *)desString[iCount], "FDD");
    desString[iCount+1] = (const char*)malloc(sizeof(char*) * 2);
    if(nullptr == desString[iCount+1])
      return nullptr;
    memset((char *)desString[iCount+1], 0, 2);
    memcpy((char*)desString[iCount+1], "\0", 1);
#else
    desString[iCount] = (const char*)malloc(sizeof(char*) * 2);
    if(nullptr == desString[iCount])
      return nullptr;
    memset((char *)desString[iCount], 0, 2);
    memcpy((char*)desString[iCount], "\0", 1);
#endif
  } else if    (NETWORK_TYPE_EHRPD == iNetworkType) {
    desString[iCount] = (const char*)malloc(sizeof(char*) * 2);
    if(nullptr == desString[iCount])
    return nullptr;
    memset((char *)desString[iCount], 0, 2);
    memcpy((char*)desString[iCount], "\0", 1);
  }
  return desString;
}
const char**
ImsInterfaceImpl::getAccessNetworkInfo()
{
  LOGE("ImsInterfaceImpl::getAccessNetworkInfo()");
  const char** panInfo = nullptr;
  LgeImsAPI* agent = LgeImsAPI::GetSingleton();
  if (!agent) {
    LOGE("ImsInterfaceImpl::GetWifiState:: agent == null");
    return nullptr;
  }
  nsCOMPtr<nsIRadioInterfaceLayer> gRIL(do_GetService("@mozilla.org/ril;1"));
  nsCOMPtr<nsIRadioInterface> radioInterface;
  if(gRIL) {
    gRIL->GetRadioInterface(0, getter_AddRefs(radioInterface));
  }
  int iNetworkType = GetNetworkType();
//
  if(NETWORK_TYPE_LTE == iNetworkType) {
    nsString mLteInfo;
    nsString& LteInfo = mLteInfo;
    radioInterface->GetLteInfoIF(LteInfo);
    if(LteInfo.IsEmpty()){
      //
      LOGE("ImsInterfaceImpl::getAccessNetworkInfo::startRetryGetAccessInfoTimer");
      radioInterface->QueryInfoForImsIF();
      startRetryGetAccessInfoTimer(1000);
      if(mPrevLteInfo.IsEmpty())
        return nullptr;
      LteInfo = mPrevLteInfo;
    }
    NS_LossyConvertUTF16toASCII convertString(LteInfo);
    LOGE("LteInfo : %s" ,convertString.get());
    panInfo = splitAccessInfo(convertString.get(), ',');
    return panInfo;
  } else if(NETWORK_TYPE_EHRPD == iNetworkType) {
    nsString mEhrpdInfo;
    nsString& EhrpdInfo = mEhrpdInfo;
    radioInterface->GetEhrpdInfoIF(EhrpdInfo);
    if(EhrpdInfo.IsEmpty()){
      //
      LOGE("ImsInterfaceImpl::getAccessNetworkInfo::startRetryGetAccessInfoTimer");
      radioInterface->QueryInfoForImsIF();
      startRetryGetAccessInfoTimer(1000);
      if(mPrevEhrpdInfo.IsEmpty())
        return nullptr;
      EhrpdInfo = mPrevEhrpdInfo;
    }
    NS_LossyConvertUTF16toASCII convertString(EhrpdInfo);
    LOGE("EhrpdInfo : %s" ,convertString.get());
    panInfo = splitAccessInfo(convertString.get(), ':');
    return panInfo;
  } else {
    return nullptr;
  }
  return nullptr;
}

bool
ImsInterfaceImpl::isEventRegistered(int event)
{
  LOGE("ImsInterfaceImpl::isEventRegistered");
  return ((mRegisteredEvent & event) != 0);
}

int32_t ImsInterfaceImpl::GetIfaceId(int32_t nApnType)
{
  LOGI("ImsInterfaceImpl::GetIfaceId APN type - %d", nApnType);
  ApnMng* Apn = ApnMng::GetApnMng();
  if(nullptr == Apn) {
    LOGI("GetApnMng fail");
    return -1;
  }
  return Apn->GetInterfaceID(nApnType);
}

void
ImsInterfaceImpl::onMessage(const android::Parcel& pParcel)
{
}

void ImsInterfaceImpl::registrationConnected()
{
    return;
}

void ImsInterfaceImpl::registrationDisconnected(const android::Parcel &pParcel)
{
    return;
}

void ImsInterfaceImpl::notifyIncomingCall(int phoneID, int serviceID, const char* callId)
{
    return;
}

void ImsInterfaceImpl::enteredECBM()
{
    return;
}

void ImsInterfaceImpl::exitedECBM()
{
    return;
}

void ImsInterfaceImpl::set911State(int32_t state, int32_t ecbm)
{
    return;
}

void
ImsInterfaceImpl::onMessageEx(const android::Parcel &pParcel, Parcel *pOutParcel)
{
  LOGE("ImsInterfaceImpl::onMessageEx");
  //
  int method = 0;
  pParcel.setDataPosition(0);
  method = pParcel.readInt32(); //

  switch (method) {
      case GET_DEVICE_ID:
        LOGE("ImsInterfaceImpl::onMessageEx::GET_DEVICE_ID");
        pOutParcel->writeString16(android::String16(GetDeviceID()));
        break;
    case GET_DEVICE_SOFTWARE_VERSION:
        LOGE("ImsInterfaceImpl::onMessageEx::GET_DEVICE_SOFTWARE_VERSION");
        pOutParcel->writeString16(android::String16(GetDeviceSoftwareVersion()));
        break;
    case GET_PHONE_NUMBER:
        LOGE("ImsInterfaceImpl::onMessageEx::GET_PHONE_NUMBER");
        pOutParcel->writeString16(android::String16(GetPhoneNumber()));
        break;
    case GET_SUBSCRIBER_ID:
        LOGE("ImsInterfaceImpl::onMessageEx::GET_SUBSCRIBER_ID");
        pOutParcel->writeString16(android::String16(GetSubscriberId()));
        break;
    case GET_MCC:
        LOGE("ImsInterfaceImpl::onMessageEx::GET_MCC");
        pOutParcel->writeString16(android::String16(GetMcc(true)));
        break;
    case GET_MNC:
        LOGE("ImsInterfaceImpl::onMessageEx::GET_MNC");
        pOutParcel->writeString16(android::String16(GetMnc(true)));
        break;
    case GET_ISIM_STATE:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::GET_ISIM_STATE");
        break;
    case READ_ISIM_FILE_ATTR:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::READ_ISIM_FILE_ATTR");
        break;
    case READ_ISIM_RECORD:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::READ_ISIM_RECORD");
        break;
    case REQ_ISIM_AUTH:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::REQ_ISIM_AUTH");
        break;
    case REQ_USIM_AUTH:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::REQ_USIM_AUTH");
        break;
    case GET_CALL_STATE:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::GET_CALL_STATE");
        break;
    case SAVE_MISSED_CALL:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::SAVE_MISSED_CALL");
        break;
    case SAVE_REJECTED_CALL:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::SAVE_REJECTED_CALL");
        break;
    case IS_BLOCKED_NUMBER:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::IS_BLOCKED_NUMBER");
        break;
    case IS_EMERGENCY_NUMBER:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::IS_EMERGENCY_NUMBER");
        break;
    case GET_MODEL_NAME:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::GET_MODEL_NAME");
        break;
    case GET_DEVICE_NAME:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::GET_DEVICE_NAME");
        break;
    case GET_DIGEST_SHA1:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::GET_DIGEST_SHA1");
        break;
    case GET_NETWORK_TYPE:
        LOGE("ImsInterfaceImpl::onMessageEx::GET_NETWORK_TYPE");
        pOutParcel->writeInt32(GetNetworkType());
        break;
    case GET_ROAMING_STATE:
        LOGE("ImsInterfaceImpl::onMessageEx::GET_ROAMING_STATE");
        pOutParcel->writeInt32(GetRoamingState());
        break;
    case GET_SERVICE_STATE:
        LOGE("ImsInterfaceImpl::onMessageEx::GET_SERVICE_STATE");
        pOutParcel->writeInt32(GetServiceState());
        break;
    case GET_ACCESS_NETWORK_INFO:
    {
        LOGE("ImsInterfaceImpl::onMessageEx::GET_ACCESS_NETWORK_INFO");
        const char **panInfo = getAccessNetworkInfo();
        if (panInfo == NULL) {
            pOutParcel->writeInt32(0);
            break;
        }
        int nCnt ;
         for(nCnt =0; strcmp(panInfo[nCnt], "\0"); nCnt++);
        pOutParcel->writeInt32(nCnt);
        for(int nCount =0; strcmp(panInfo[nCount], "\0"); nCount++)
        {
          char *temp = (char*)malloc(sizeof(char*) * strlen(panInfo[nCount])+1);
          strcpy(temp, panInfo[nCount]);
          pOutParcel->writeString16(android::String16(temp));
          free((char*)panInfo[nCount]);
        }
        free(panInfo);
    /*


         */
        break;
    }
    case ACTIVATE_DATA_CONNECTION:
        LOGE("ImsInterfaceImpl::onMessageEx::ACTIVATE_DATA_CONNECTION");
        pOutParcel->writeInt32(enableDataConnectivity(pParcel.readInt32()));
        break;
    case DEACTIVATE_DATA_CONNECTION:
        LOGE("ImsInterfaceImpl::onMessageEx::DEACTIVATE_DATA_CONNECTION");
        pOutParcel->writeInt32(DisableDataConnectivity(pParcel.readInt32()));
        break;
    case GET_APN_NAME:
        LOGE("ImsInterfaceImpl::onMessageEx::GET_APN_NAME");
        pOutParcel->writeString16(android::String16(GetApn(pParcel.readInt32())));
        break;
    case GET_DATA_CONNECTION_STATE:
        LOGE("ImsInterfaceImpl::onMessageEx::GET_DATA_CONNECTION_STATE");
        pOutParcel->writeInt32(getDataState(pParcel.readInt32()));
        break;
    case GET_LOCAL_ADDRESS:
        LOGE("ImsInterfaceImpl::onMessageEx::GET_LOCAL_ADDRESS");
        pOutParcel->writeString16(android::String16(GetLocalAddress(pParcel.readInt32(), pParcel.readInt32())));
        break;
    case GET_PCSCF_ADDRESSES:
    {
        LOGE("ImsInterfaceImpl::onMessageEx::GET_PCSCF_ADDRESSES");
        const char **pcscfs = getPcscfAddress(pParcel.readInt32(), pParcel.readInt32());
        if (pcscfs == NULL) {
            pOutParcel->writeInt32(0);
            break;
        }
        int nCnt ;
         for(nCnt =0; strcmp(pcscfs[nCnt], "\0"); nCnt++);
        pOutParcel->writeInt32(nCnt);
        LOGE("ImsInterfaceImpl::onMessageEx::GET_PCSCF_ADDRESSES count - %d",nCnt);
        for(int nCount =0; strcmp(pcscfs[nCount], "\0"); nCount++)
        {
          char *temp = (char*)malloc(sizeof(char*) * strlen(pcscfs[nCount])+1);
          strcpy(temp, pcscfs[nCount]);
          pOutParcel->writeString16(android::String16(temp));
          free((char*)pcscfs[nCount]);
        }
        free(pcscfs);
    /*


         */
        break;
    }
    case REQUEST_ROUTE_TO_HOST_ADDRESS:
            {
        LOGE("ImsInterfaceImpl::onMessageEx::REQUEST_ROUTE_TO_HOST_ADDRESS");
        int napntype = pParcel.readInt32();
        String16 str16 = pParcel.readString16();
        String8 str8(str16);
        pOutParcel->writeInt32(RequestRouteToHostAddress(
                napntype, (char*) str8.string()));
        }
        break;
    case IS_IMS_EMERGENCY_CALL_SUPPPORTED:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::IS_IMS_EMERGENCY_CALL_SUPPPORTED");
        //
        break;
    case IS_IMS_VOICE_CALL_SUPPPORTED:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::IS_IMS_VOICE_CALL_SUPPPORTED");
        //
        break;
    case GET_IFACE_NAME:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::GET_IFACE_NAME");
        //
        break;
    case GET_LTE_RSRP_STRENGTH:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::GET_LTE_RSRP_STRENGTH");
        //
        break;
    case GET_LAST_ACCESS_NETWORK_INFO:
    {    LOGE("ImsInterfaceImpl::onMessageEx::GET_LAST_ACCESS_NETWORK_INFO");
        const char **lastANInfo ;//
        if (lastANInfo == NULL) {
            pOutParcel->writeInt32(0);
            break;
        }
        int nCnt ;
         for(nCnt =0; strcmp(lastANInfo[nCnt], "\0"); nCnt++);
        pOutParcel->writeInt32(nCnt);
        for(int nCount =0; strcmp(lastANInfo[nCount], "\0"); nCount++)
        {
          char *temp = (char*)malloc(sizeof(char*) * strlen(lastANInfo[nCount])+1);
          strcpy(temp, lastANInfo[nCount]);
          pOutParcel->writeString16(android::String16(temp));
          free((char*)lastANInfo[nCount]);
        }
        free(lastANInfo);
    /*


         */
        break;
    }
    case GET_IFACE_ID:
        LOGE("ImsInterfaceImpl::onMessageEx::GET_IFACE_ID");
        pOutParcel->writeInt32(GetIfaceId(pParcel.readInt32()));
        break;
    case GET_HOST_BY_NAME:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::GET_HOST_BY_NAME");
        break;
    case SEND_PING_TO_HOST_ADDRESS:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::SEND_PING_TO_HOST_ADDRESS");
        //
        break;
    case GET_WIFI_BSS_ID:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::GET_WIFI_BSS_ID");
        //
        break;
    case GET_WIFI_DETAILED_STATE:
        LOGE("ImsInterfaceImpl::onMessageEx::GET_WIFI_DETAILED_STATE");
        pOutParcel->writeInt32(GetWifiDetailedState());
        break;
    case GET_WIFI_STATE:
        LOGE("ImsInterfaceImpl::onMessageEx::GET_WIFI_STATE");
        pOutParcel->writeInt32(GetWifiState());
        break;
    case GET_WIFI_SSID:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::GET_WIFI_SSID");
        //
        break;
    case SET_ALARM:
        {
        LOGE("ImsInterfaceImpl::onMessageEx::SET_ALARM");

        int nDuration = pParcel.readInt32();
        long nAlarmID = pParcel.readInt64();
        StartTimer(nAlarmID, nDuration);
        }
        break;
    case KILL_ALARM:
        LOGE("ImsInterfaceImpl::onMessageEx::KILL_ALARM");
        StopTimer(pParcel.readInt64());
        break;
    case GET_PREFERENCE:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::GET_PREFERENCE");
        break;
    case SET_PREFERENCE:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::SET_PREFERENCE");
        break;
    case SEND_EVENT:
        LOGE("ImsInterfaceImpl::onMessageEx::SEND_EVENT");
        {
          nsCOMPtr<nsIRunnable> proc = new IMSToImsInterfaceRunnable(this, pParcel.readInt32(), pParcel.readInt32(), pParcel.readInt32());
          if (NS_FAILED(NS_DispatchToMainThread(proc))) {
            LOGE("ImsInterfaceImpl::onMessageEx error");
            return;
          }
        }
        break;
    case SET_EVENT:
        LOGE("ImsInterfaceImpl::onMessageEx::SET_EVENT event - %d",SET_EVENT);
        mRegisteredEvent |= pParcel.readInt32();
        pOutParcel->writeInt32(1);
        break;
    case RESET_EVENT:
        LOGE("ImsInterfaceImpl::onMessageEx::RESET_EVENT");
        mRegisteredEvent &= (~pParcel.readInt32());
        pOutParcel->writeInt32(1);
        break;
    case LISTEN_SRVCC_EVT:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::LISTEN_SRVCC_EVT");
        break;
    case UNLISTEN_SRVCC_EVT:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::UNLISTEN_SRVCC_EVT");
        break;
    case IS_WFC_ENABLED:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::IS_WFC_ENABLED");
        break;
    case GET_WFC_PREFERENCES:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::GET_WFC_PREFERENCES");
        break;
    case START_LOCATION_INFO:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::START_LOCATION_INFO");
        break;
    case STOP_LOCATION_INFO:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::STOP_LOCATION_INFO");
        break;
    case GET_LOCATION_INFO:
        LOGE("Unknown::ImsInterfaceImpl::onMessageEx::GET_LOCATION_INFO");
        break;
    default:
        LOGE("ImsInterfaceImpl::onMessageEx::default");
        return;
  }
}

void
ImsInterfaceImpl::SendEventInternal(int nEvent, int nWParam, int nLParam)
{
  LOGE("ImsInterfaceImpl::SendEventInternal nEvent - %d, nWParam - %d, nLParam - %d", nEvent, nWParam, nLParam);
  switch (nEvent) {
    case IMS_EVENT_NATIVE_BOOT_COMPLETED:
    {
      LgeImsAPI* agent = LgeImsAPI::GetSingleton();
      if (!agent) {
        LOGE("ImsInterfaceImpl::SendEvent:: agent == null");
        return ;
      }
      agent->onImsBootCompleted();
      //
      startRetryPsiTimer(2000);
    }
      break;
    case IMS_EVENT_WIFI_LOCK:

      break;
    case IMS_EVENT_REG_STATE:
      if(IMS_REG_ON == nLParam) {
        LOGE("IMS_EVENT_REG_STATE :: IMS_REG_ON");
        SetImsStatusForDan(1);
        SetImsRegStatus(true);
      }else if(IMS_REG_OFF_DONE == nLParam) {
        LOGE("IMS_EVENT_REG_STATE :: IMS_REG_OFF_DONE");
        SetImsStatusForDan(0);
      }else {
        LOGE("IMS_EVENT_REG_STATE :: else");
        SetImsRegStatus(false);
      }
      break;
    case IMS_EVENT_DEBUG_STRING:

      break;
    case IMS_EVENT_VT_CALL:

      break;
    case IMS_EVENT_VT_INDICATOR:

      break;
    case IMS_EVENT_VOLTE_INDICATOR:

      break;
    case IMS_EVENT_EHPRD_REG_SENDING:

      break;
    case IMS_EVENT_WAKE_LOCK:

      break;
    case IMS_EVENT_WIFI_SERVICE:

      break;
    case IMS_EVENT_CALL_BARRED_TOAST:

      break;
    case IMS_EVENT_SHOW_MESSAGE:

      break;
    case IMS_EVENT_TRACE_MOCA:

      break;
    case IMS_EVENT_MMTEL_SERVICE_READY:

      break;
    case IMS_EVENT_DATA_CONNECTION:
       if (nWParam == IMS_DEACTIVATE_REQ) {
         int iNetworkType = GetNetworkType();
         if(iNetworkType == NETWORK_TYPE_EHRPD) {
           closeImsForaPeriodofTime(720000);
           SetImsStatusForDan(2);
         }else if(iNetworkType == NETWORK_TYPE_LTE) {
          //
          //
          //
          nsCOMPtr<nsIRadioInterfaceLayer> gRIL(do_GetService("@mozilla.org/ril;1"));
          nsCOMPtr<nsIRadioInterface> radioInterface;
          if(gRIL) {
            gRIL->GetRadioInterface(0, getter_AddRefs(radioInterface));
          }
          switch (nLParam) {
            case IMS_DETACH_TIMER:
              LOGE("IMS_EVENT_DATA_CONNECTION :: IMS_DETACH_TIMER");
              //
              break;
            case IMS_DETACH_PERMANENT:
              LOGE("IMS_EVENT_DATA_CONNECTION :: REASON_PERMANENT_DETACH");
              //
              break;
            case IMS_DETACH_AND_ATTACH:
              LOGE("IMS_EVENT_DATA_CONNECTION :: REASON_DETACH_AND_ATTACH");
              //
              break;
          }
         }
       }
      break;
    case IMS_EVENT_PHONE_NUMBER_READY:

      break;
    default:
        LOGE("SendEvent :: Default: - %d", nEvent);
      break;
  }
}

int
ImsInterfaceImpl::RequestRouteToHostAddress(int nApnType, char* strHostAddress)
{
  LOGE("RequestRouteToHostAddress is called for ApnType -%d, pcscf addr = %s", nApnType, strHostAddress);
#if 0
  int Result = 1;
  int Prefix = 0;

//
  NetUtils* mNetUtils = new NetUtils();
  ApnMng* Apn = ApnMng::GetApnMng();
  if(nullptr == Apn)
    LOGI("GetApnMng fail");
  char16_t **mGateway = nullptr;
  if(nApnType == APN_IMS) {
    NS_ConvertUTF16toUTF8 ifname(Apn->GetName(nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS));
    mGateway = Apn->GetGateways(nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS);
    NS_ConvertUTF16toUTF8 converGateway(mGateway[0]);
    nsString nspcscfAddr;
    nspcscfAddr.AssignASCII(strHostAddress);
    NS_ConvertUTF16toUTF8 coverPcscf(nspcscfAddr);
    if(strstr(coverPcscf.get(), ":")) {
      Prefix = 128;
    } else if(strstr(coverPcscf.get(), ".")) {
      Prefix = 32;
    } else {
      return 0;
    }
    LOGE("Add Route %s %s %d", ifname.get(), converGateway.get(), Prefix);
    mNetUtils->do_ifc_add_route(ifname.get(), converGateway.get(), Prefix, NULL);
    LOGE("Add Route %s %s %s %d", ifname.get(), coverPcscf.get(), converGateway.get(), Prefix);
    Result = mNetUtils->do_ifc_add_route(ifname.get(), coverPcscf.get(), Prefix, converGateway.get());
  } else if(nApnType == APN_EMERGENCY){
    NS_ConvertUTF16toUTF8 ifname(Apn->GetName(nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY));
    mGateway = Apn->GetGateways(nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY);
    NS_ConvertUTF16toUTF8 converGateway(mGateway[0]);
    nsString nspcscfAddr;
    nspcscfAddr.AssignASCII(strHostAddress);
    NS_ConvertUTF16toUTF8 coverPcscf(nspcscfAddr);
    if(strstr(coverPcscf.get(), ":")) {
      Prefix = 128;
    } else if(strstr(coverPcscf.get(), ".")) {
      Prefix = 32;
    } else {
      return 0;
    }
    LOGE("Add Route %s %s %d", ifname.get(), converGateway.get(), Prefix);
    mNetUtils->do_ifc_add_route(ifname.get(), converGateway.get(), Prefix, NULL);
    LOGE("Add Route %s %s %s %d", ifname.get(), coverPcscf.get(), converGateway.get(), Prefix);
    Result = mNetUtils->do_ifc_add_route(ifname.get(), coverPcscf.get(), Prefix, converGateway.get());
  }
  LOGE("RequestRouteToHostAddress returned = %d", Result);
  delete mNetUtils;
  if(Result == 0)
    return 1;
  else
    return 0;
#endif
  return 1;
}

void
ImsInterfaceImpl::binderDied(const wp<IBinder>&) {
    LOGE("binder death detected");
}
