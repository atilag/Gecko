#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <android/log.h>
#include <time.h>
#include "LgeImsAPI.h"
#include "nsComponentManagerUtils.h"
#include <stdio.h>
#include "nsIWeakReferenceUtils.h"
#include "nsIDOMEventListener.h"
#include "nsIPrefService.h"
#include "nsIInterfaceRequestor.h"
#include "nsServiceManagerUtils.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsISupportsPrimitives.h"
#include "nsIObserverService.h"
#include "jsapi.h" //
#include "nsContentUtils.h"
//
#include "nsXULAppAPI.h"  //
#include "nsISupportsImpl.h"
#undef LOG_TAG
#define LOG_TAG "LGE_IMS_API"
#include <utils/Log.h>
#include "mozilla/Services.h"
#include "nsMemory.h"
#include "ImsInterfaceImpl.h"
#include "nsIRadioInterfaceLayer.h"
#include "nsINetworkService.h"
#include "ImsEventConstantMap.h"
#include "nsJSUtils.h"
#include "nsIMobileNetworkInfo.h"
#include "nsIMobileConnectionInfo.h"
#include "IImsDbSettings.h"
#include <time.h>
#include "nsIAlarmHalService.h"
#include "mozilla/dom/alarm/AlarmHalService.h"
#include "nsIImsAlarmService.h"
#include "nscore.h"
//
#include <binder/Parcel.h>
#include <binder/IServiceManager.h>
#include <utils/String16.h>
#include "mozilla/dom/ScriptSettings.h"
#include "nsIMobileConnectionService.h"
#include "mozilla/dom/BindingUtils.h"
#include "mozilla/dom/SettingChangeNotificationBinding.h"
#include "IXPCOMCoreInterface.h"
#include "UCInterface.h"
#include "ApnMang.h"
#include "nsIIccProvider.h"
#include "ImsManager.h"

#define LOGI(args...) \
     __android_log_print(ANDROID_LOG_INFO, LOG_TAG, ## args)

#define LOGE(args...) \
     __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, ## args)

nsRefPtr<ImsManager> pIImsUCManager;
static const char kWiFiSetting[] = "wifi.enabled";
static const char kAirplaneModeStatus[] =  "airplaneMode.status";
static const char kMozSettingsChanged[] = "mozsettings-changed";
static const char kXpcomShutdown[] = "profile-change-teardown";
static const char kNetIfaceStateChanged[] = "network-connection-state-changed";
static const char kXpcomStartup[] = "profile-after-change";
static const char kDataChanged[] = "data-changed";

using namespace mozilla;
using namespace mozilla::dom;

NS_IMPL_ISUPPORTS(LgeImsAPI, nsIObserver, nsILgeImsAPI, nsIMobileConnectionListener) //

LgeImsAPI* LgeImsAPI::sSingleton;
//
sp<IXpcomDbSettingsInterface> pImsDbInterface = NULL;
bool isEnabledPreviously = false;
void *lib_handle = NULL;
LgeImsAPI::LgeImsAPI(): mWifiEnabled(false),mWifiState(-1),mWifiDetailedState(-1)
{
  memset(mDataServiceState, 0, sizeof(mDataServiceState));
  memset(mRAT, 0, sizeof(mRAT));
  strcpy(mRoaming, "false");

  NS_INIT_ISUPPORTS();
  sSingleton = this;
  LOGE("LgeImsAPI is created");
  imsImpl = NULL;
  imsNativeWrapper = NULL;
  Init();
  bPcscfOnConnection = false;
  mCloseImsTimeRun = false;
  NS_GetCurrentThread(getter_AddRefs(imsXpcomThread));
  if(imsXpcomThread == nullptr)
  {
    LOGE("LgeImsAPI::LgeImsAPI::imsXpcomThread == null");
  }
}

LgeImsAPI::~LgeImsAPI()
{
  MOZ_ASSERT(NS_IsMainThread());
  sSingleton = nullptr;
  dlclose(lib_handle);
  imsXpcomThread = nullptr;
}

NS_IMETHODIMP
LgeImsAPI::HandleEvent(nsIDOMEvent* aDOMEvent) {
  return NS_OK;
}

LgeImsAPI*
LgeImsAPI::GetSingleton()
{
  MOZ_ASSERT(NS_IsMainThread());
  MOZ_ASSERT(sSingleton, "LgeImsAPI does not exist");
  NS_ADDREF(sSingleton);
  return sSingleton;
}
bool bCurrentAirplaneModeStatus = false;
void
LgeImsAPI::HandleAirplaneModeStatusChanged(nsISupports* aSubject)
{
  AutoSafeJSContext cx;
  if (!cx) {
    return;
  }
  RootedDictionary<SettingChangeNotification> setting(cx);
  if (!WrappedJSToDictionary(cx, aSubject, setting)) {
    return ;
  }
  if (!setting.mKey.EqualsASCII(kAirplaneModeStatus)) {
    return;
  }
  if (!setting.mValue.isString()) {
    return;
  }

  JS::Rooted<JSString*> Status(cx, setting.mValue.toString());
  nsAutoJSString StatusStr;
  if (!StatusStr.init(cx, Status)) {
    return;
  }

  if((StatusStr.EqualsLiteral("enabling")) || (StatusStr.EqualsLiteral("enabled")))
    mAirplaneModeEnabled = true;
  else if((StatusStr.EqualsLiteral("disabling")) || (StatusStr.EqualsLiteral("disabled")))
    mAirplaneModeEnabled = false;

  if(bCurrentAirplaneModeStatus == mAirplaneModeEnabled) {
    return;
  } else {
    android::Parcel parcel;
    if (mAirplaneModeEnabled) {
      LOGE("LgeImsAPI::mAirplaneModeStatus::Enabling Or Enabled");
      imsImpl->notifyEvent(IMS_EVENT_AIRPLANE_MODE, IMS_AIRPLANE_MODE_ON, 0);
    } else{
       LOGE("LgeImsAPI::mAirplaneModeStatus::Disabling Or Disabled");
       imsImpl->notifyEvent(IMS_EVENT_AIRPLANE_MODE, IMS_AIRPLANE_MODE_OFF, 0);
    }
    bCurrentAirplaneModeStatus = mAirplaneModeEnabled;
  }
}

void
LgeImsAPI::HandleWifiSettingStatusChanged(const char16_t* aData)
{
  AutoSafeJSContext cx;

  nsDependentString dataStr(aData);
  JS::Rooted<JS::Value> val(cx);
  if (!JS_ParseJSON(cx, dataStr.get(), dataStr.Length(), &val) || !val.isObject()) {
    return;
  }

  JS::Rooted<JSObject*> obj(cx, &val.toObject());
  JS::Rooted<JS::Value> key(cx);
  if (!JS_GetProperty(cx, obj, "key", &key) || !key.isString()) {
    return;
  }

  bool match;
  if (!JS_StringEqualsAscii(cx, key.toString(), kWiFiSetting, &match) || !match) {
    return;
  }
  JS::Rooted<JS::Value> value(cx);
  if (!JS_GetProperty(cx, obj, "value", &value) || !value.isBoolean()) {
    return;
  }

  if (match) {
    mWifiEnabled = value.toBoolean();

    /*


     */
    android::Parcel parcel;
    if (mWifiEnabled)
      //
      parcel.writeInt32(WIFI_STATE_ENABLED);
    else
      //
      parcel.writeInt32(WIFI_STATE_DISABLED);

    imsNativeWrapper->sendData2NativeImpl(NOTIFY_WIFI_STATE_CHANGED, parcel);

    return;
  }
}

class SendConnectionStateToApnMang: public nsRunnable {
public:
  SendConnectionStateToApnMang(ApnMng* interface, int32_t iApnType, int32_t iState) :
    mInterface(interface) {
    nApnType = iApnType;
    nState = iState;
  }

  NS_IMETHOD Run() {
    if (mInterface)
      mInterface->sendConnectedEventToApnMang(nApnType, nState);
    return NS_OK;
  }
private:
  ApnMng* mInterface;
  int32_t nApnType;
  int32_t nState;
};

bool
LgeImsAPI::HandleNetIfaceStateChange(nsINetworkInterface* aNetworkInterface)
{
  LOGE("LgeImsAPI::HandleNetIfaceStateChange");
  if (!aNetworkInterface) {
    return false;
  }
  int32_t type = -1;
  nsString aIp, aName;
  aNetworkInterface->GetType(&type);
  if ((-1 != type))
  {
    int32_t state = nsINetworkInterface::NETWORK_STATE_UNKNOWN;
    aNetworkInterface->GetState(&state);

    LOGE("=============>Network type: %d  state :%d" , type,state);
    bool isConnected = (state == nsINetworkInterface::NETWORK_STATE_CONNECTED);
    if (isConnected == true)
    {
      if(type == nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS)
      {
        ApnMng* Apn = ApnMng::GetApnMng();
        char16_t **pIPs = nullptr;
        char16_t **pGateWay = nullptr;
        uint32_t *prefixLengths = nullptr;
        uint32_t count = 0;
        aNetworkInterface->GetAddresses(&pIPs, &prefixLengths, &count);
        LOGE("=========>Get Ip count : %d " ,count);
        nsAutoString ip;
        if (0 < count) {
          ip.Assign(pIPs[0]);
        }
        nsMemory::Free(prefixLengths);
        NS_FREE_XPCOM_ALLOCATED_POINTER_ARRAY(count, pIPs);
        aNetworkInterface->GetName(aName);
        NS_LossyConvertUTF16toASCII convertName(aName);
        LOGE("=========>After Change --> Network Ip: %s " ,NS_ConvertUTF16toUTF8(ip).get());
        LOGE("=============>After Change --> Network   name : %s" ,convertName.get());
        //
        isEnabledPreviously = true;

        /*


         */
        //
      //
        LOGE("LgeImsAPI::HandleNetIfaceStateChange::sendConnectedEventToNative()");
        if(nullptr == Apn) {
          LOGE("LgeImsAPI::HandleNetIfaceStateChange::Apn is null");
          return false;
        }
        Apn->SetApnType(type);
        Apn->SetConnectionState(type, state);
        Apn->SetLocalAddress(NS_ConvertUTF16toUTF8(ip).get(), type);
        Apn->SetName(type, aName);
        Apn->SetIsEnabledPreviously(type, true);
        Apn->SetInterfaceID(type);
        nsCOMPtr<nsIRunnable> proc = new SendConnectionStateToApnMang(Apn, type, state);
        if (NS_FAILED(NS_DispatchToMainThread(proc))) {
          LOGE("LgeImsAPI::HandleNetIfaceStateChange::NS_DispatchToMainThread error");
          return false;
        }

 //

        mImsDataState = DATA_STATE_CONNECTED;
        bPcscfOnConnection = true;
        //
        aNetworkInterface->GetGateways(&count, &pGateWay);
        LOGE("=========>After Change --> Get Gateway count : %d " ,count);
       //
        Apn->SetGateways(type, pGateWay, count);
        Apn->SetGatewayCount(type, count);
        NS_FREE_XPCOM_ALLOCATED_POINTER_ARRAY(count, pGateWay);
        bAlreadyPdnReq = true;
      }
#if 0
      else if(type == nsINetworkInterface::NETWORK_TYPE_MOBILE) {
        char16_t **pIPs = nullptr;
        char16_t **pGateWay = nullptr;
        uint32_t *prefixLengths = nullptr;
        uint32_t count = 0;
        aNetworkInterface->GetAddresses(&pIPs, &prefixLengths, &count);
        LOGE("=========>Get Ip count : %d " ,count);
        nsAutoString ip;
        if (0 < count) {
          ip.Assign(pIPs[0]);
        }
        nsMemory::Free(prefixLengths);
        NS_FREE_XPCOM_ALLOCATED_POINTER_ARRAY(count, pIPs);
        aNetworkInterface->GetName(aName);
        NS_LossyConvertUTF16toASCII convertName(aName);
        LOGE("=========>Network Ip: %s " ,NS_ConvertUTF16toUTF8(ip).get());
        LOGE("=============>Network   name : %s" ,convertName.get());
        //
        mDefaultDataState = DATA_STATE_CONNECTED;
        pSysDispatcher->xpcomSystem_DataConnectionStateChanged(2, DATA_STATE_CONNECTED);
      }
#endif
      else if(type == nsINetworkInterface::NETWORK_TYPE_WIFI) {
        char16_t **pIPs = nullptr;
        //
        uint32_t *prefixLengths = nullptr;
        uint32_t count = 0;
        aNetworkInterface->GetAddresses(&pIPs, &prefixLengths, &count);
        LOGE("=========>After Change --> Get Ip count : %d " ,count);
        nsAutoString ip;
        if (0 < count) {
          ip.Assign(pIPs[0]);
        }
        nsMemory::Free(prefixLengths);
        NS_FREE_XPCOM_ALLOCATED_POINTER_ARRAY(count, pIPs);
        aNetworkInterface->GetName(aName);
        NS_LossyConvertUTF16toASCII convertName(aName);
        LOGE("=========>After Change --> Network Ip: %s " ,NS_ConvertUTF16toUTF8(ip).get());
        LOGE("=============>After Change --> Network   name : %s" ,convertName.get());
        mWifiState = WIFI_STATE_ENABLED;
        mWifiDetailedState = WIFI_NET_DETAILED_STATE_CONNECTED;
        /*


         */
        //

     android::Parcel parcel;
     parcel.writeInt32(WIFI_NET_DETAILED_STATE_CONNECTED);
        imsNativeWrapper->sendData2NativeImpl(NOTIFY_WIFI_DETAILED_STATE_CHANGED, parcel);
      }
      else if(type == nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY)
      {
        ApnMng* Apn = ApnMng::GetApnMng();
        char16_t **pIPs = nullptr;
        char16_t **pGateWay = nullptr;
        uint32_t *prefixLengths = nullptr;
        uint32_t count = 0;
        aNetworkInterface->GetAddresses(&pIPs, &prefixLengths, &count);
        LOGE("=========>Get Ip count : %d " ,count);
        nsAutoString ip;
        if (0 < count) {
          ip.Assign(pIPs[0]);
        }
        nsMemory::Free(prefixLengths);
        NS_FREE_XPCOM_ALLOCATED_POINTER_ARRAY(count, pIPs);
        aNetworkInterface->GetName(aName);
        NS_LossyConvertUTF16toASCII convertName(aName);
        LOGE("=========>After Change --> Network Ip: %s " ,NS_ConvertUTF16toUTF8(ip).get());
        LOGE("=============>After Change --> Network   name : %s" ,convertName.get());
        //
        isEnabledPreviously = true;
#if 0
        if(pSysDispatcher == IMS_NULL) {
          LOGE("=================>HandleNetIfaceStateChange   --------->pSysDispatcher is null");
          return false;
        }
#endif
        //
        LOGE("LgeImsAPI::HandleNetIfaceStateChange::sendConnectedEventToNative()");
        if(nullptr == Apn) {
          return false;
        }
        Apn->SetApnType(type);
        Apn->SetConnectionState(type, state);
        Apn->SetLocalAddress(NS_ConvertUTF16toUTF8(ip).get(), type);
        Apn->SetName(type, aName);
        Apn->SetIsEnabledPreviously(type, true);
        Apn->SetInterfaceID(type);

        nsCOMPtr<nsIRunnable> proc = new SendConnectionStateToApnMang(Apn, type, state);
        if (NS_FAILED(NS_DispatchToMainThread(proc))) {
          LOGE("LgeImsAPI::HandleNetIfaceStateChange::NS_DispatchToMainThread error");
          return false;
        }

//
        mEmergecnyDataState = DATA_STATE_CONNECTED;
        bPcscfOnConnection = true;
        //
        aNetworkInterface->GetGateways(&count, &pGateWay);
        Apn->SetGateways(type, pGateWay, count);
        Apn->SetGatewayCount(type, count);
        NS_FREE_XPCOM_ALLOCATED_POINTER_ARRAY(count, pGateWay);
        mEmergencyAttached = true;
      }
    } else if((state == nsINetworkInterface::NETWORK_STATE_DISCONNECTED) || (state == nsINetworkInterface::NETWORK_STATE_CONNECTING))    {
        LOGE("=============>nsINetworkInterface::NETWORK_STATE_DISCONNECTED, isEnabledPreviously - %d", isEnabledPreviously);
        if((type == nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS) && (isEnabledPreviously == true))
        {
          //
          isEnabledPreviously = false;
          ApnMng* Apn = ApnMng::GetApnMng();
          if(nullptr == Apn) {
            return false;
          }
          //
          bNotifyPcscfReceived = false;
          Apn->SetConnectionState(type, state);
          Apn->SetIsEnabledPreviously(type, false);
          /*





          */
          if(mImsDataState == DATA_STATE_CONNECTED){
            mImsDataState = DATA_STATE_DISCONNECTED;
            //
            android::Parcel parcel;
            parcel.writeInt32(APN_IMS);
             parcel.writeInt32(DATA_STATE_DISCONNECTED);
            imsNativeWrapper->sendData2NativeImpl(NOTIFY_DATA_CONNECTION_STATE_CHANGED, parcel);
          }
          if(state == nsINetworkInterface::NETWORK_STATE_DISCONNECTED) {
            bAlreadyPdnReq = false;
          }
        }
#if 0
        else if(type == nsINetworkInterface::NETWORK_TYPE_MOBILE)
        {
           mDefaultDataState = DATA_STATE_DISCONNECTED;
           pSysDispatcher->xpcomSystem_DataConnectionStateChanged(2, DATA_STATE_DISCONNECTED);
        }
#endif
        else if((type == nsINetworkInterface::NETWORK_TYPE_WIFI) && (mWifiState == WIFI_STATE_ENABLED)) {
          mWifiState = WIFI_STATE_DISABLED;
          mWifiDetailedState = WIFI_NET_DETAILED_STATE_DISCONNECTED;
          //
       android::Parcel parcel;
          parcel.writeInt32(WIFI_NET_STATE_DISCONNECTED);
       parcel.writeInt32(WIFI_NET_DETAILED_STATE_DISCONNECTED);
          imsNativeWrapper->sendData2NativeImpl(NOTIFY_WIFI_DETAILED_STATE_CHANGED, parcel);
        }
        else if((type == nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY) && ((isEnabledPreviously == true))) {
          isEnabledPreviously = false;
          ApnMng* Apn = ApnMng::GetApnMng();
          if(nullptr == Apn) {
            return false;
          }
          //
          bNotifyPcscfReceived = false;
          Apn->SetConnectionState(type, state);
          Apn->SetIsEnabledPreviously(type, false);
          if(mEmergecnyDataState == DATA_STATE_CONNECTED){
            mEmergecnyDataState = DATA_STATE_DISCONNECTED;
            //
            android::Parcel parcel;
            parcel.writeInt32(APN_EMERGENCY);
            parcel.writeInt32(DATA_STATE_DISCONNECTED);
            imsNativeWrapper->sendData2NativeImpl(NOTIFY_DATA_CONNECTION_STATE_CHANGED, parcel);
          }
          if(state == nsINetworkInterface::NETWORK_STATE_DISCONNECTED) {
            mEmergencyAttached = false;
          }
        }
      }
      return isConnected;
    }
    return false;
}

bool
LgeImsAPI::is1xRTT() {
  if((strcmp(mRAT, "'is95a'") == 0) || (strcmp(mRAT, "'is95b'") == 0) || (strcmp(mRAT, "'1xrtt'") == 0)) {
    return true;
  } else {
    return false;
  }
}

bool
LgeImsAPI::is2G() {
  if((strcmp(mRAT, "'gsm'") == 0) || (strcmp(mRAT, "'edge'") == 0)) {
    return true;
  } else {
    return false;
  }
}

bool
LgeImsAPI::is3G() {
  if((strcmp(mRAT, "'umts'") == 0) || (strcmp(mRAT, "'hsdpa'") == 0) || (strcmp(mRAT, "'hsupa'") == 0) || (strcmp(mRAT, "'hspa'") == 0) || (strcmp(mRAT, "'hspa+'") == 0)) {
    return true;
  } else {
    return false;
  }
}

void
LgeImsAPI::HandleDataChanged(const char16_t* aData)
{
  LOGI("LgeImsAPI::HandleDataChanged");
  bool dataServiceStateChanged = false;
  bool ratChanged = false;
  bool roamingChanged = false;

  int RAT_eHRPD = 1;
  int RAT_4G = 2;
  int RAT_3G = 3;
  int RAT_1xRTT = 4;
  int RAT_2G = 5;

  NS_LossyConvertUTF16toASCII convert(aData);
  LOGE("=======> HandleDataChanged Incoming String = %s", convert.get());

  char *tempStr = strstr(convert.get(), "connected : ");
  if(tempStr == NULL)
    return;
  char *valueStr = strstr(tempStr, ",");
  tempStr += strlen("connected : ");
  tempStr[valueStr - tempStr] = '\0';
  LOGE("=======> Value of connected = %s", tempStr);

  {
    //
    //
    NS_LossyConvertUTF16toASCII convertState(aData);
    tempStr = strstr(convertState.get(), "state :");
    if(tempStr == NULL)
        return;
    valueStr = strstr(tempStr, ",");
    tempStr += strlen("state : ");
    tempStr[valueStr - tempStr] = '\0';
    LOGE("=======> Value of state = %s", tempStr);

    if (strcmp(mDataServiceState, tempStr) != 0) {
      strcpy(mDataServiceState, tempStr);
      dataServiceStateChanged = true;
    }

    //
    NS_LossyConvertUTF16toASCII convertRAT(aData);
    tempStr = strstr(convertRAT.get(), "type : ");
    if(tempStr == NULL)
        return;
    valueStr = strstr(tempStr, ",");
    tempStr += strlen("type : ");
    tempStr[valueStr - tempStr] = '\0';
    LOGE("=======> Value of type = %s", tempStr);

    if (strcmp(mRAT, tempStr) != 0) {
      strcpy(mRAT, tempStr);
      LOGE("=======> mRaT is  %s", mRAT);
      ratChanged = true;
    }

    //
    NS_LossyConvertUTF16toASCII convertRoaming(aData);
    tempStr = strstr(convertRoaming.get(), "roaming : ");
    if(tempStr == NULL)
        return;
    valueStr = strstr(tempStr, ",");
    tempStr += strlen("roaming : ");
    tempStr[valueStr - tempStr] = '\0';
    LOGE("=======> Value of roaming = %s", tempStr);

    if (strcmp(mRoaming, tempStr) != 0) {
      strcpy(mRoaming, tempStr);
      roamingChanged = true;
    }

    if (dataServiceStateChanged )
    {
      android::Parcel parcel;
      if(strcmp(mDataServiceState, "'registered'") == 0) {
    //
        Connect(APN_IMS);
        //
     parcel.writeInt32(STATE_IN_SERVICE);
      } else {
        //
        parcel.writeInt32(STATE_OUT_OF_SERVICE);
      }
      imsNativeWrapper->sendData2NativeImpl(NOTIFY_SERVICE_STATE_CHANGED, parcel);
    }

    if (ratChanged) {
      //
      android::Parcel parcel;
      if (strcmp(mRAT,"'lte'") == 0){
          Connect(APN_IMS);
          //
          parcel.writeInt32(RAT_4G);
        } else if(strcmp(mRAT, "'ehrpd'") == 0){
          Connect(APN_IMS);
          //
          parcel.writeInt32(RAT_eHRPD);
        } else if(is3G()){
          //
            parcel.writeInt32(RAT_3G);
        } else if(is1xRTT()){
          //
          parcel.writeInt32(RAT_1xRTT);
        } else if(is2G()){
          //
        parcel.writeInt32(RAT_2G);
        }
        imsNativeWrapper->sendData2NativeImpl(NOTIFY_NETWORK_TYPE_CHANGED, parcel);
        if ((strcmp(mRAT,"'lte'") == 0) || (strcmp(mRAT, "'ehrpd'") == 0)) {
          nsCOMPtr<nsIRadioInterfaceLayer> gRIL(do_GetService("@mozilla.org/ril;1"));
          nsCOMPtr<nsIRadioInterface> radioInterface;
          if(gRIL) {
            gRIL->GetRadioInterface(0, getter_AddRefs(radioInterface));
          }
          radioInterface->QueryInfoForImsIF();
        }
        imsImpl->stopCloseImsPdnTimer(true);
     }

    if (roamingChanged) {
      android::Parcel parcel;
      if(strcmp(mRoaming, "true") == 0) {
        disconnectImsApn();
        //

      } else {
    //
          Connect(APN_IMS);
        //

      }
      imsImpl->notifyEvent(IMS_EVENT_ROAMING_STATE, IMS_ROAMING_STATE_OFF, 0);
    }
  }
  return;
}

NS_IMETHODIMP
LgeImsAPI::Observe(nsISupports* aSubject, const char* aTopic,  const char16_t* aData)
{
  nsCOMPtr<nsIObserverService> obs = services::GetObserverService();
  if (!obs) {
  LOGE("LgeImsAPI::Observe:: obs == null");
    return NS_ERROR_FAILURE;
  }

  LgeImsAPI* agent = LgeImsAPI::GetSingleton();
  if (!agent) {
    return NS_ERROR_FAILURE;
  }

  if(!strcmp(kXpcomStartup, aTopic)) {
    nsresult rv;
#if 0
    {
      fn3 funcptr3;
      funcptr3 = (IXPCOMInterface* (*)())dlsym(lib_handle, "GetIXPCOMInterfaceImpl");
      if (!funcptr3) {
        LOGE("Init() : dlsym1 is error");
        return NS_ERROR_FAILURE;

      }
      pSysDispatcher = funcptr3();
      if(!pSysDispatcher) {
        LOGE("LgeImsAPI::Observe:: pSysDispatcher == null");
        return NS_ERROR_FAILURE;
      }
      pSysDispatcher->xpcomSystem_BootCompleted();
    }
    {
      fn2 funcptr2;
      funcptr2 = (IXpcomDbSettingsInterface* (*)())dlsym(lib_handle, "GetIXPCOMDbSettingsInterface");
      if (!funcptr2) {
        LOGE("Init() : dlsym1 is error");
        return NS_ERROR_FAILURE;
      }
      pImsDbInterface = funcptr2();
      if(!pImsDbInterface) {
        LOGE("LgeImsAPI::Observe:: pImsDbInterface == null");
        return NS_ERROR_FAILURE;
      }
    }
#else
    LOGE("LgeImsAPI::Observe::kDataChanged AddObsever Startxpcom");
    sp<IServiceManager> ism = defaultServiceManager();
    sp<IBinder> binder;
    binder = ism->getService(String16("com.lge.ims.dbinterface"));
    if (binder != 0) {
      pImsDbInterface = interface_cast<IXpcomDbSettingsInterface>(binder);
    }
#endif
    {//
      LOGE("LgeImsAPI::Observe:: Set listener for data change");
      nsresult rv;
      nsCOMPtr<nsIMobileConnectionService> service =
      do_GetService(NS_MOBILE_CONNECTION_SERVICE_CONTRACTID);
      if(nullptr == service) {
        LOGE("LgeImsAPI::Observe:: service is null");
      } else {
        nsCOMPtr<nsIMobileConnection> connection;
        service->GetItemByServiceId(mClientId, getter_AddRefs(connection));
        if(nullptr == connection) {
          LOGE("LgeImsAPI::Observe:: connection is null");
          mRetryDataAddObserver = do_CreateInstance("@mozilla.org/timer;1");
          if (!mRetryDataAddObserver) {
            LOGE("LgeImsAPI::Observe::mRetryDataAddObserver == null");
            return NS_ERROR_FAILURE;
          }
          nsIEventTarget* target = static_cast<nsIEventTarget*>(imsXpcomThread);
          rv = mRetryDataAddObserver->SetTarget(target);
          if(NS_FAILED(rv)) {
            LOGE("LgeImsAPI::Observe::SetTarget failed");
            return NS_ERROR_FAILURE;
          }
          LgeImsAPI* agent = LgeImsAPI::GetSingleton();
          rv = mRetryDataAddObserver->InitWithFuncCallback(retryDataAddObservers,
                                                      agent,
                                                      2000,
                                                      nsITimer::TYPE_ONE_SHOT);
          if(NS_FAILED(rv)) {
            LOGE("LgeImsAPI::Observe::InitWithFuncCallback failed") ;
            return NS_ERROR_FAILURE;
          }
        } else {
          rv = connection->RegisterListener(this);
          if (NS_FAILED(rv)) {
            LOGE("LgeImsAPI::Observe:: RegisterListener to connection failed");
          }
        }
      }
    }

    rv = obs->AddObserver(static_cast<nsIObserver *>(agent), kMozSettingsChanged, false);
    if (NS_FAILED(rv)) {
      LOGE("LgeImsAPI::Observe::kMozSettingsChanged AddObsever failed");
    }

    LOGE("LgeImsAPI::Observe::kXpcomShutdown AddObsever");
    rv = obs->AddObserver(static_cast<nsIObserver *>(agent), kXpcomShutdown, false);
    if (NS_FAILED(rv)) {
      LOGE("LgeImsAPI::Observe::kXpcomShutdown AddObsever failed");
    }

    rv = obs->AddObserver(static_cast<nsIObserver *>(agent), kNetIfaceStateChanged, false);
    if (NS_FAILED(rv)) {
      LOGE("LgeImsAPI::Observe::kNetIfaceStateChanged AddObsever failed");
    }

    rv = obs->AddObserver(static_cast<nsIObserver *>(agent), kDataChanged, false);
    if (NS_FAILED(rv)) {
      LOGE("LgeImsAPI::Observe::kDataChanged AddObsever failed");
    }
    return NS_OK;
  } else  if (!strcmp(kMozSettingsChanged, aTopic)) {
 //
      HandleAirplaneModeStatusChanged(aSubject);
    return NS_OK;
  } else  if (!strcmp(kNetIfaceStateChanged, aTopic)) {
    LOGE("LgeImsAPI::Observe::kNetIfaceStateChanged");
    bool bSettingsChanged = false;
    nsCOMPtr<nsINetworkInterface> network = do_QueryInterface(aSubject);
    if (network) {
      bSettingsChanged |= HandleNetIfaceStateChange(network);
    }
    return NS_OK;
  } else  if (!strcmp(kDataChanged, aTopic)) {
      HandleDataChanged(aData);
      return NS_OK;
  } else  if (strcmp(kXpcomShutdown,aTopic) == 0) {
    /*


     */
    LOGE("LgeImsAPI::Observe::xpcomSystem_EventReceived::IMS_EVENT_POWER_OFF");
    //

    imsImpl->notifyEvent(IMS_EVENT_POWER_OFF, 0, 0);
    nsresult rv;
    {//
      nsCOMPtr<nsIMobileConnectionService> service =
      do_GetService(NS_MOBILE_CONNECTION_SERVICE_CONTRACTID);
      if(nullptr == service) {
        LOGE("LgeImsAPI::Observe:: service is null");
      } else {
        nsCOMPtr<nsIMobileConnection> connection;
        service->GetItemByServiceId(mClientId, getter_AddRefs(connection));
        if(nullptr == connection) {
          LOGE("LgeImsAPI::Observe:: connection is null");
        } else {
          rv = connection->UnregisterListener(this);
          if (NS_FAILED(rv)) {
            LOGE("LgeImsAPI::Observe:: UnregisterListener to connection failed");
          }
        }
      }
    }
    rv = obs->RemoveObserver(static_cast<nsIObserver *>(agent), kXpcomStartup);
    if (NS_FAILED(rv)) {
      LOGE("LgeImsAPI::Observe:: RemoveObserver kXpcomStartup failed");
    }

    rv = obs->RemoveObserver(static_cast<nsIObserver *>(agent), kMozSettingsChanged);
    if (NS_FAILED(rv)) {
      LOGE("LgeImsAPI::Observe:: RemoveObserver kMozSettingsChanged failed");
    }
    rv = obs->RemoveObserver(static_cast<nsIObserver *>(agent), kXpcomShutdown);
    if (NS_FAILED(rv)) {
      LOGE("LgeImsAPI::Observe:: RemoveObserver kXpcomShutdown failed");
    }
    rv = obs->RemoveObserver(static_cast<nsIObserver *>(agent), kNetIfaceStateChanged);
    if (NS_FAILED(rv)) {
      LOGE("LgeImsAPI::Observe:: RemoveObserver kNetIfaceStateChanged failed");
    }
    rv = obs->RemoveObserver(static_cast<nsIObserver *>(agent), kDataChanged);
        if (NS_FAILED(rv)) {
          LOGE("LgeImsAPI::Observe:: RemoveObserver kDataChanged failed");
    }

    return NS_OK;
  }
  return NS_OK;
}

void
LgeImsAPI::InitListenerService()
{
  nsCOMPtr<nsIObserverService> obs = services::GetObserverService();
  if (!obs) {
  LOGE("LgeImsAPI::InitListenerService:: obs == null");
  return;
  }
  LgeImsAPI* agent = LgeImsAPI::GetSingleton();
    if (!agent) {
    LOGE("LgeImsAPI::InitListenerService:: agent == null");
    return;
    }
  obs->AddObserver(static_cast<nsIObserver *>(agent), kXpcomStartup, false);
  LOGE("LgeImsAPI::InitListenerService::kXpcomShutdown AddObsever");
  obs->AddObserver(static_cast<nsIObserver *>(agent), kXpcomShutdown, false);
}

void
LgeImsAPI::Init()
{
#if 0
  lib_handle = dlopen("system/lib/libims.so", RTLD_LAZY);
  if (!lib_handle) {
    LOGE("Init() : lib_handle is null");
    return;
  }

  fn1 funcptr1;
  funcptr1 = (void (*)())dlsym(lib_handle, "XPCOM_OnLoad");
  if (!funcptr1) {
    LOGE("Init() : dlsym is error");
    return;
  }

  funcptr1();

    //
    //
    //
    //
#endif
  imsImpl = new ImsInterfaceImpl();
  LOGE("LgeImsAPI::init() create APN MNG");
  ApnMng* Apn = ApnMng::GetApnMng();
  android::sp<android::IServiceManager> sm = android::defaultServiceManager();
  sm->addService(android::String16("com.lge.ims.ucinterface"), imsImpl);

  LOGE("LgeImsAPI::NativeSystemInterface() is called");
  imsNativeWrapper = NativeWrapper::GetInstance();
  if(imsNativeWrapper == nullptr) {
    return;
  }
//
  LOGE("LgeImsAPI::NativeSystemInterface is completed");
  InitListenerService();
  {
    LOGE("LgeImsAPI::Init()::mImsWithoutSimCard");
    nsresult rv;
    mImsWithoutSimCard = do_CreateInstance("@mozilla.org/timer;1");
    if (!mImsWithoutSimCard) {
    LOGE("LgeImsAPI::Init()::mImsWithoutSimCard == null");
    return;
    }
    nsIEventTarget* target = static_cast<nsIEventTarget*>(imsXpcomThread);
    rv = mImsWithoutSimCard->SetTarget(target);
    if(NS_FAILED(rv)) {
    LOGE("LgeImsAPI::Init()::mImsWithoutSimCard::SetTarget failed");
    return;
    }
    LgeImsAPI* agent = LgeImsAPI::GetSingleton();
    rv = mImsWithoutSimCard->InitWithFuncCallback(ImsWithoutSIMCard,
                                    agent,
                                    30000,
                                    nsITimer::TYPE_ONE_SHOT);
    if(NS_FAILED(rv)) {
    LOGE("LgeImsAPI::Init()::mImsWithoutSimCard::InitWithFuncCallback failed") ;
    return;
    }
  }
}

const char*
LgeImsAPI::GetLocalAddress (int apnType, int ipVersion)
{
  ApnMng* Apn = ApnMng::GetApnMng();
  if(nullptr == Apn)
    return nullptr;
  return Apn->GetLocalAddress(apnType, ipVersion);
}

int
LgeImsAPI::GetWifiState()
{
  return mWifiState;
}

int
LgeImsAPI::GetWifiDetailedState()
{
  return mWifiDetailedState;
}

NS_IMETHODIMP
LgeImsAPI::NotifyVoiceChanged()
{
  return NS_OK;
}

NS_IMETHODIMP
LgeImsAPI::NotifyDataChanged()
{
  LOGE("LgeImsAPI::NotifyDataChanged()");

  bool dataServiceStateChanged = false;
  bool ratChanged = false;
  bool roamingChanged = false;

  int RAT_eHRPD = 1;
  int RAT_4G = 2;
  int RAT_3G = 3;
  int RAT_1xRTT = 4;
  int RAT_2G = 5;

  nsCOMPtr<nsIMobileConnectionService> service = do_GetService(NS_MOBILE_CONNECTION_SERVICE_CONTRACTID);
  if (!service) {
    LOGI("Cannot get MobileConnectionService");
    return NS_ERROR_FAILURE;
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
      nsString regState;
      bool bDataRoaming = false;
      nsString ratType;
      data->GetConnected(&isConnected);
      data->GetRoaming(&bDataRoaming);
      data->GetState(regState);
      data->GetType(ratType);
      NS_LossyConvertUTF16toASCII convertState(regState);
      if(convertState.Length() > 0) {
        LOGE("LgeImsAPI::NotifyDataChanged()::State - %s", convertState.get());
        strcpy(mDataServiceState, convertState.get());
      }
      NS_LossyConvertUTF16toASCII convertType(ratType);
      if(convertType.Length() > 0) {
        LOGE("LgeImsAPI::NotifyDataChanged()::type - %s", convertType.get());
        strcpy(mRAT, convertType.get());
      }
      if(true == bDataRoaming) {
        strcpy(mRoaming, "true");
      } else {
        strcpy(mRoaming, "false");
      }

      if (strcmp(mDataServiceState, "'registered'") != 0) {
        strcpy(mDataServiceState, "'registered'");
        dataServiceStateChanged = true;
      }
      if (strcmp(mRAT, "'lte'") != 0) {
        strcpy(mRAT, "'lte'");
        ratChanged = true;
      }
      if (strcmp(mRoaming, "false") != 0) {
        strcpy(mRoaming, "false");
        roamingChanged = true;
      }

      if (dataServiceStateChanged ) {
        android::Parcel parcel;
        if(strcmp(mDataServiceState, "'registered'") == 0) {
          Connect(APN_IMS);
          //
          parcel.writeInt32(STATE_IN_SERVICE);
        } else {
          //
          parcel.writeInt32(STATE_OUT_OF_SERVICE);
        }
     imsNativeWrapper->sendData2NativeImpl(NOTIFY_SERVICE_STATE_CHANGED, parcel);
      }
      if (ratChanged) {
        //
        android::Parcel parcel;
        if (strcmp(mRAT,"'lte'") == 0){
          Connect(APN_IMS);
          //
       parcel.writeInt32(RAT_4G);
        } else if(strcmp(mRAT, "'ehrpd'") == 0){
          Connect(APN_IMS);
          //
       parcel.writeInt32(RAT_eHRPD);
        } else if(is3G()){
          //
          parcel.writeInt32(RAT_3G);
        } else if(is1xRTT()){
          //
       parcel.writeInt32(RAT_1xRTT);
        } else if(is2G()){
          //
                 parcel.writeInt32(RAT_2G);
        }
     imsNativeWrapper->sendData2NativeImpl(NOTIFY_NETWORK_TYPE_CHANGED, parcel);
        if ((strcmp(mRAT,"'lte'") == 0) || (strcmp(mRAT, "'ehrpd'") == 0)) {
          nsCOMPtr<nsIRadioInterfaceLayer> gRIL(do_GetService("@mozilla.org/ril;1"));
          nsCOMPtr<nsIRadioInterface> radioInterface;
          if(gRIL) {
            gRIL->GetRadioInterface(0, getter_AddRefs(radioInterface));
          }
          radioInterface->QueryInfoForImsIF();
        }
        imsImpl->stopCloseImsPdnTimer(true);
      }

      if (roamingChanged) {
     android::Parcel parcel;
        if(strcmp(mRoaming, "true") == 0) {
          disconnectImsApn();
          //
        } else {
          Connect(APN_IMS);
          //

        }
     imsImpl->notifyEvent(IMS_EVENT_ROAMING_STATE, IMS_ROAMING_STATE_OFF, 0);
      }
    }
  }
  return NS_OK;
}

NS_IMETHODIMP
LgeImsAPI::NotifyDataError(const nsAString & message)
{
  return NS_OK;
}

NS_IMETHODIMP
LgeImsAPI::NotifyCFStateChanged(uint16_t action,
                                uint16_t reason,
                                const nsAString& number,
                                uint16_t timeSeconds,
                                uint16_t serviceClass)
{
  return NS_OK;
}

NS_IMETHODIMP
LgeImsAPI::NotifyEmergencyCbModeChanged(bool active, uint32_t timeoutMs)
{
  return NS_OK;
}

NS_IMETHODIMP
LgeImsAPI::NotifyOtaStatusChanged(const nsAString & status)
{
  return NS_OK;
}

NS_IMETHODIMP
LgeImsAPI::NotifyRadioStateChanged()
{
  LOGE("LgeImsAPI::NotifyRadioStateChanged()");
  return NS_OK;
}

NS_IMETHODIMP
LgeImsAPI::NotifyClirModeChanged(uint32_t aMode)
{
  return NS_OK;
}

NS_IMETHODIMP
LgeImsAPI::NotifyLastKnownNetworkChanged()
{
  return NS_OK;
}

NS_IMETHODIMP
LgeImsAPI::NotifyLastKnownHomeNetworkChanged()
{
  return NS_OK;
}

NS_IMETHODIMP
LgeImsAPI::NotifyNetworkSelectionModeChanged()
{
  LOGE("LgeImsAPI::NotifyNetworkSelectionModeChanged()");
  return NS_OK;
}

/*                     */
NS_IMPL_ISUPPORTS(nsNetworkInterface, nsINetworkInterface)

nsNetworkInterface::nsNetworkInterface()
{
  /*                                          */
}

nsNetworkInterface::~nsNetworkInterface()
{
  /*                 */
}

NS_IMETHODIMP
nsNetworkInterface::GetState(int32_t *aState)
{
  return NS_OK;
}

NS_IMETHODIMP
nsNetworkInterface::GetType(int32_t *aType)
{
  return NS_OK;
}

NS_IMETHODIMP
nsNetworkInterface::GetName(nsAString & aName)
{
  int32_t iApnType = -1;
  GetType(&iApnType);
  LOGE("nsNetworkInterface::GetName::ApnType - %d", iApnType);
  ApnMng* agent = ApnMng::GetApnMng();
  aName = agent->GetName(iApnType);
  return NS_OK;
}


NS_IMETHODIMP
nsNetworkInterface::GetAddresses(char16_t * **ips, uint32_t **prefixLengths, uint32_t *count)
{
  return NS_OK;
}

NS_IMETHODIMP
nsNetworkInterface::GetGateways(uint32_t *count, char16_t * **gateways)
{
  //
  int32_t iApnType = -1;
  GetType(&iApnType);
  LOGE("nsNetworkInterface::GetName::ApnType - %d", iApnType);
  ApnMng* agent = ApnMng::GetApnMng();
  *gateways = agent->GetGateways(iApnType);
  *count = agent->GetGatewayCount(iApnType);
  return NS_OK;
}

NS_IMETHODIMP
nsNetworkInterface::GetDnses(uint32_t *count, char16_t * **dnses)
{
  return NS_OK;
}

NS_IMETHODIMP
nsNetworkInterface::GetHttpProxyHost(nsAString & aHttpProxyHost)
{
  return NS_OK;
}

NS_IMETHODIMP
nsNetworkInterface::GetHttpProxyPort(int32_t *aHttpProxyPort)
{
  return NS_OK;
}
bool
LgeImsAPI::GetPsiInfo()
{
  LOGE("LgeImsAPI::GetPsiInfo()");
  if(false == mAlreadyPsiRead) {
    nsCOMPtr<nsIRadioInterfaceLayer> gRIL(do_GetService("@mozilla.org/ril;1"));
    nsCOMPtr<nsIRadioInterface> radioInterface;
    nsString PsiSMSC;
    if (gRIL) {
      gRIL->GetRadioInterface(0, getter_AddRefs(radioInterface));
    }
    LOGE("LgeImsAPI::GetPsiInfo()::GetPhoneInfoIF");
    radioInterface->GetPhoneInfoIF(3, PsiSMSC);
    if(!PsiSMSC.IsEmpty()) {
      LOGE("LgeImsAPI::GetPsiInfo() is not empty");
      NS_LossyConvertUTF16toASCII convertName(PsiSMSC);
      LOGE("LgeImsAPI::GetPsiInfo() - %s", convertName.get());
      pImsDbInterface->UpdatePSIValue(convertName.get());
      mAlreadyPsiRead = true;
      return true;
    } else {
      LOGE("LgeImsAPI::GetPsiInfo() is empty");
      mAlreadyPsiRead = false;
      return false;
    }
  }
  return false;
}

NS_IMETHODIMP
LgeImsAPI::GetImsUcManager(nsIImsUCManager* *ainterface)
{
  LOGE("LgeImsAPI::GetImsUcManager");
  pIImsUCManager = ImsManager::getInstance();
  return CallQueryInterface(pIImsUCManager, ainterface);
}

NS_IMETHODIMP
LgeImsAPI::NotifySIMLoaded()
{
  LOGE("LgeImsAPI::NotifySIMLoaded()");
  imsNativeWrapper->NativeSystemInterface(imsImpl);
  if(true == GetPsiInfo()){
    int CONFIG_EVENT_CAT_2 = (IMS_CONFIG_CAT_2 << 16);
    imsImpl->notifyEvent(IMS_EVENT_CONFIG_UPDATE, CONFIG_EVENT_CAT_2, 0);
  }
  LOGE("LgeImsAPI::NotifySIMLoaded()::IMS_EVENT_PHONE_NUMBER_AVAILABLE");
  imsImpl->notifyEvent(IMS_EVENT_PHONE_NUMBER_AVAILABLE, IMS_PHONE_NUMBER_AVAILABLE_INITIAL, IMS_PHONE_NUMBER_SIM_LOADED);

  return NS_OK;
}
NS_IMETHODIMP
LgeImsAPI::NotifyPcscfUpdate(nsTArray<nsString> &aPcfcsAddresses_ipv4, nsTArray<nsString> &aPcfcsAddresses_ipv6, int32_t iApnType)
{
  LOGE("LgeImsAPI::NotifyPcscfUpdate()");
  ApnMng* Apn = ApnMng::GetApnMng();
  Apn->SetPcscfAddress(aPcfcsAddresses_ipv4, aPcfcsAddresses_ipv6, iApnType);
  //
  bNotifyPcscfReceived = true;
  if(bPcscfOnConnection) {
      LOGE("LgeImsAPI::bPcscfOnConnection true");
/*


     */
    bPcscfOnConnection = false;
  } else if(DATA_STATE_CONNECTED == mImsDataState) {
    LOGE("LgeImsAPI::PcscfUpdateConnect()::DATA_STATE_DISCONNECTED");
    if(nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS == iApnType) {
      android::Parcel parcel;
      parcel.writeInt32(APN_IMS); //
      parcel.writeInt32(DATA_STATE_DISCONNECTED);
      imsNativeWrapper->sendData2NativeImpl(NOTIFY_DATA_CONNECTION_STATE_CHANGED, parcel);
    }
    else if(nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY == iApnType) {
      android::Parcel parcel;
      parcel.writeInt32(APN_EMERGENCY); //
      parcel.writeInt32(DATA_STATE_DISCONNECTED);
      imsNativeWrapper->sendData2NativeImpl(NOTIFY_DATA_CONNECTION_STATE_CHANGED, parcel);
    }
    //
    NS_DispatchToMainThread(NS_NewRunnableMethodWithArg<int32_t>(this, &LgeImsAPI::PcscfUpdateConnect, iApnType));
  }
  return NS_OK;
}

void
LgeImsAPI::PcscfUpdateConnect(int32_t iApnType)
{
  LOGE("LgeImsAPI::PcscfUpdateConnect()");
  LOGE("LgeImsAPI::PcscfUpdateConnect()::DATA_STATE_CONNECTED");
  if(nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS == iApnType) {
    android::Parcel parcel;
    parcel.writeInt32(APN_IMS); //
    parcel.writeInt32(DATA_STATE_CONNECTED); //
    imsNativeWrapper->sendData2NativeImpl(NOTIFY_DATA_CONNECTION_STATE_CHANGED, parcel);
  }
  else if(nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY == iApnType) {
    android::Parcel parcel;
    parcel.writeInt32(APN_EMERGENCY); //
    parcel.writeInt32(DATA_STATE_CONNECTED); //
    imsNativeWrapper->sendData2NativeImpl(NOTIFY_DATA_CONNECTION_STATE_CHANGED, parcel);
  }
#if 0
  ApnMng* Apn = ApnMng::GetApnMng();
  if(nullptr == Apn) {
    return false;
  }
  Apn->SetNotifyPcscfReceived(iApnType, false);
#endif
  bNotifyPcscfReceived = false;
}

bool
LgeImsAPI::disconnectImsApn()
{
  nsCOMPtr<nsIRadioInterfaceLayer> gRIL(do_GetService("@mozilla.org/ril;1"));
  nsCOMPtr<nsIRadioInterface> radioInterface;
  if (gRIL) {
    gRIL->GetRadioInterface(0, getter_AddRefs(radioInterface));
  }

  if(getDataCallStateByType(nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS) == nsINetworkInterface::NETWORK_STATE_CONNECTED ) {
    radioInterface->DeactivateDataCallByType(nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS);
    bAlreadyPdnReq = false;
    return true;
  }
  return true;
}

bool
LgeImsAPI::disconnectEmergencyApn()
{
  nsCOMPtr<nsIRadioInterfaceLayer> gRIL(do_GetService("@mozilla.org/ril;1"));
  nsCOMPtr<nsIRadioInterface> radioInterface;
  if (gRIL) {
    gRIL->GetRadioInterface(0, getter_AddRefs(radioInterface));
  }
  if(getDataCallStateByType(nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY) == nsINetworkInterface::NETWORK_STATE_CONNECTED ) {
    radioInterface->DeactivateDataCallByType(nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY);
    mEmergencyAttached = false;
    return true;
  }
  return true;
}

bool
LgeImsAPI::connectEmergencyApn()
{
  LOGE("LgeImsAPI::connectEmergencyApn");

  if(true == mEmergencyAttached) {
    LOGE("LgeImsAPI::connectEmergencyApn::Already attached");
    return false;
  }
  nsCOMPtr<nsIRadioInterfaceLayer> gRIL(do_GetService("@mozilla.org/ril;1"));
  nsCOMPtr<nsIRadioInterface> radioInterface;
  if (gRIL) {
    gRIL->GetRadioInterface(0, getter_AddRefs(radioInterface));
  }
  if(getDataCallStateByType(nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY) != nsINetworkInterface::NETWORK_STATE_CONNECTED ) {
      radioInterface->SetupDataCallByType(nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY);
      mEmergencyAttached = true;
      return true;
  }
  return false;
}

bool
LgeImsAPI::connectImsApn()
{
  LOGE("LgeImsAPI::connectImsApn");
  if(bAlreadyPdnReq == true) {
    LOGE("Data connection(mobile_ims) request is already sent");
    return true;
  }
  //
  //
  //
  //
  if(TEST_MASK_ROAMING_CONDITION & pImsDbInterface->GetTestMaskValue()) {
    LOGI("ImsInterfaceImpl::connectImsApn::POLICY_ROAMING_ALLOWED");
  }else if (checkRoaming()) {
    LOGE("Roaming is not allowed");
    return false;
  }
  //
  if(!checkRadioTechnology()) {
      LOGE("RAT is not valid");
    return false;
  }

  //
  if(mAirplaneModeEnabled)
  {
    LOGE("Airplane Mode is enabled");
    return false;
  }
#if 0
  if(mEmergencyAttached == true) {
    LOGE("Emergency PDN Attached");
    return false;
  }
#endif
  //
#if 0//
  { //
    if(mWifiState == WIFI_NET_STATE_CONNECTED) {
      LOGE("Data state :: WiFi is connected");
      if ((mImsDataState == DATA_STATE_DISCONNECTED)
            && (mDefaultDataState == DATA_STATE_DISCONNECTED)) {

      } else {
        LOGE("Data state :: mobile_ims= %d, mobile= %d", mImsDataState, mDefaultDataState);
        return false;
      }
    } else{
        LOGE("Data state :: WiFi is not connected");
        if (mDefaultDataState == DATA_STATE_DISCONNECTED) {
            LOGE("Data state :: mobile is not connected");
            return false;
        }
    }
  }
 #endif
  //
  //
  //
  if (mCloseImsTimeRun == true) {
    LOGE("Timer for stopping ims pdn is running");
    return false;
  }
  nsCOMPtr<nsIRadioInterfaceLayer> gRIL(do_GetService("@mozilla.org/ril;1"));
  nsCOMPtr<nsIRadioInterface> radioInterface;
  if (gRIL) {
    gRIL->GetRadioInterface(0, getter_AddRefs(radioInterface));
  }

  if(getDataCallStateByType(nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS) != nsINetworkInterface::NETWORK_STATE_CONNECTED ) {
      radioInterface->SetupDataCallByType(nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS);
    bAlreadyPdnReq = true;
      return true;
  }
  return false;
}

bool
LgeImsAPI::Connect(int apnType)
{
  LOGE("LgeImsAPI::Connect");
  //
  if(pImsDbInterface == NULL) {
    LOGE("LgeImsAPI::Connect:: pImsDbInterface == null");
    return false;
  }
  if(!pImsDbInterface->isImsOn()) {
    LOGE("LgeImsAPI::Connect::isImsOn::false");
    return false;
  }
  //
  if(!isImsServiceStarted){
    LOGE("LgeImsAPI::Connect::Ims service not started");
    return false;
  }
  if (apnType == APN_IMS) {
    LOGE("LgeImsAPI::Connect::APN_IMS");
    return connectImsApn();
  }    else if(apnType == APN_INTERNET) {
    //
    return false;
  } else if (apnType == APN_EMERGENCY) {
    LOGE("LgeImsAPI::Connect::APN_EMERGENCY");
    connectEmergencyApn();
    return false;
  }
  return false;
}

bool
LgeImsAPI::checkRoaming()
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

long
LgeImsAPI::getDataCallStateByType(long aAPNType)
{
  LOGE("LgeImsAPI::getDataCallStateByType() - %lu", aAPNType);
  nsCOMPtr<nsIRadioInterfaceLayer> gRIL(do_GetService("@mozilla.org/ril;1"));
  nsCOMPtr<nsIRadioInterface> radioInterface;
  if (gRIL) {
    gRIL->GetRadioInterface(0, getter_AddRefs(radioInterface));
  }
  int32_t state = -1;
  radioInterface->GetDataCallStateByType(aAPNType, &state);
  LOGE("LgeImsAPI::getDataCallStateByType():: %d", state);
  return state;
}

void
LgeImsAPI::notifyImsStatusToObserverService(int32_t iStatus)
{
  LOGI("LgeImsAPI::notifyImsStatusToObserverService()");
  nsCOMPtr<nsIObserverService> obs = services::GetObserverService();
  char *pcStatus = (char*)malloc(256);
  memset(pcStatus, 0x0, 256);
  if(IMS_STARTED == iStatus) {
    strcpy(pcStatus, "STARTED");
  } else {
    strcpy(pcStatus, "STOPED");
  }
  LOGI("LgeImsAPI::notifyImsStatusToObserverService() IMS Service %s",pcStatus);
  obs->NotifyObservers(NULL, "ims-service-status", NS_ConvertASCIItoUTF16(pcStatus).get());
}

void
LgeImsAPI::notifyImsServiceStarted()
{
  LOGE("LgeImsAPI::notifyImsServiceStarted()");
  notifyImsStatusToObserverService(IMS_STARTED);
  if(!isImsServiceStarted) {
    isImsServiceStarted = true;
    Connect(APN_IMS);
  }
}
void
LgeImsAPI::onImsBootCompleted()
{
  LOGE("LgeImsAPI::onImsBootCompleted()");
  NS_DispatchToMainThread(NS_NewRunnableMethod(this, &LgeImsAPI::notifyImsServiceStarted));

  LOGE("LgeImsAPI::IMS_EVENT_AOS_START event - %d",IMS_EVENT_AOS_START);
  imsImpl->notifyEvent(IMS_EVENT_AOS_START,0,0);
  LOGE("LgeImsAPI::after IMS_EVENT_AOS_START event - %d",IMS_EVENT_AOS_START);
}
bool
LgeImsAPI::checkRadioTechnology()
{
  if ((strcmp(mRAT,"'lte'") == 0) || (strcmp(mRAT,"'ehrpd'") == 0)) {
    return true;
  }
  return false;
}

void
LgeImsAPI::sendConnectedEventToNative(int32_t iApnType)
{
  LOGE("LgeImsAPI::sendConnectedEventToNative() - %d", iApnType);
  nsresult rv;
  iCurrentApnType = iApnType;
  if(bNotifyPcscfReceived){
    //
    android::Parcel parcel;
    if(iApnType == nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS) {
      parcel.writeInt32(APN_IMS); //
    }
    else if(iApnType == nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY){
      parcel.writeInt32(APN_EMERGENCY); //
    }
    parcel.writeInt32(DATA_STATE_CONNECTED); //
    imsNativeWrapper->sendData2NativeImpl(NOTIFY_DATA_CONNECTION_STATE_CHANGED, parcel);
    bNotifyPcscfReceived = false;
  }else {
    //
    LOGE("LgeImsAPI::sendConnectedEventToNative()::mRetryPcscfTimer");
    mRetryPcscfTimer = do_CreateInstance("@mozilla.org/timer;1");
    if (!mRetryPcscfTimer) {
    LOGE("LgeImsAPI::sendConnectedEventToNative::mRetryPcscfTimer == null");
    return;
    }
    nsIEventTarget* target = static_cast<nsIEventTarget*>(imsXpcomThread);
    rv = mRetryPcscfTimer->SetTarget(target);
    if(NS_FAILED(rv)) {
    LOGE("LgeImsAPI::sendConnectedEventToNative::SetTarget failed");
    return;
    }
    LgeImsAPI* agent = LgeImsAPI::GetSingleton();
    rv = mRetryPcscfTimer->InitWithFuncCallback(retryPcscfTimerCallback,
                                    agent,
                                    2000,
                                    nsITimer::TYPE_ONE_SHOT);
    if(NS_FAILED(rv)) {
    LOGE("LgeImsAPI::sendConnectedEventToNative::InitWithFuncCallback failed") ;
    return;
    }
  }

}
void
LgeImsAPI::retryPcscfTimerCallback(nsITimer* aTimer, void* aClosure)
{
  LOGE("LgeImsAPI::retryPcscfTimerCallback()");
  LgeImsAPI* self = static_cast<LgeImsAPI*>(aClosure);
  if (self) {
    if(aTimer) {
      self->HandleRetryPcscfTimerEvent();
      aTimer->Cancel();
      aTimer = nullptr;
  //
    }
    self = nullptr;
  } else {
    LOGE("LgeImsAPI::retryPcscfTimerCallback()::Creating Object failed");
  }
}
void
LgeImsAPI::HandleRetryPcscfTimerEvent()
{
  if(isEnabledPreviously == true)
  {
     //
    ApnMng* Apn = ApnMng::GetApnMng();
    if(nullptr == Apn)
      return;
    android::Parcel parcel;
    if(iCurrentApnType == nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS) {
      parcel.writeInt32(APN_IMS); //
    }
    else if(iCurrentApnType == nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY){
      parcel.writeInt32(APN_EMERGENCY); //
    }
    int32_t state = Apn->GetConnectionState(iCurrentApnType);
    if(state == nsINetworkInterface::NETWORK_STATE_CONNECTED) {
      parcel.writeInt32(DATA_STATE_CONNECTED);
    }else {
      parcel.writeInt32(DATA_STATE_DISCONNECTED);
    }
    imsNativeWrapper->sendData2NativeImpl(NOTIFY_DATA_CONNECTION_STATE_CHANGED, parcel);
  }
}

NS_IMETHODIMP
LgeImsAPI::NotifyOnAlarmTimerFired(int32_t alarmId)
{
  LOGE("LgeImsAPI: NotifyOnAlarmTimerFired :: %d", alarmId);

  std::map<int32_t, int32_t>::iterator it;
  for (it = mAlarmTimerId.begin(); it != mAlarmTimerId.end(); ++it )
  if (it->second == alarmId)
    break;
  if(it == mAlarmTimerId.end()) {
    LOGE("ImsInterfaceImpl::HandleTimerExpireEvent:: end of timer object array");
    return NS_OK;
  }
  //
  LOGE("LgeImsAPI: NotifyOnAlarmTimerFired alarmId :: %d, timer id - %d", alarmId, it->first);
  android::Parcel parcel;
  parcel.writeInt64(it->first);
  imsNativeWrapper->sendData2NativeImpl(NOTIFY_ALARM_EXPIRED, parcel);
  mAlarmTimerId.erase(it->first);
  return NS_OK;
}
NS_IMETHODIMP
LgeImsAPI::NotifyOnAlarmId(int32_t iTimerID, int32_t iAlarmId)
{
  LOGE("LgeImsAPI::NotifyOnAlarmId iTimerID %d, iAlarmId %d", iTimerID, iAlarmId);
  mAlarmTimerId.insert ( std::pair<int32_t,int32_t>(iTimerID,iAlarmId));
  return NS_OK;
}
void
LgeImsAPI::startHalTimer(stTimer getVal)
{
  LOGE("LgeImsAPI:: startHalTimer");
  nsCOMPtr<nsIImsAlarmService> gAlarmService(do_GetService("@mozilla.org/ImsAlarmservice;1"));
  if(gAlarmService){
    LOGE("=================>gAlarmService");
    LOGE("LgeImsAPI::startHalTimer - Duration - %d, TimerId %d", getVal.Duration, getVal.TimerID);
    gAlarmService->ImsSetAlarm(getVal.Duration, getVal.TimerID);
 //
  }else{
    LOGE("=================>No gAlarmService");
  }
}
void
LgeImsAPI::callHalTimer(int32_t iDelay, int32_t iTimerId)
{
  LOGE("LgeImsAPI:: callHalTimer- time - %d, Id - %d", iDelay, iTimerId);
  stTimer setval;
  setval.Duration = iDelay;
  setval.TimerID = iTimerId;

  NS_DispatchToMainThread(NS_NewRunnableMethodWithArg<stTimer>(this, &LgeImsAPI::startHalTimer, setval));
}
void
LgeImsAPI::callStopHalTimer(int32_t iTimerId)
{
  LOGE("LgeImsAPI:: callStopHalTimer");
  if (mAlarmTimerId.find(iTimerId) != mAlarmTimerId.end()) {
    LOGE("LgeImsAPI:: callStopHalTimer :: Timer Found");
    int32_t alarmId = mAlarmTimerId[iTimerId];
    LOGE("LgeImsAPI:: callStopHalTimer :: alarmId - %d", alarmId);
     nsCOMPtr<nsIImsAlarmService> gAlarmService(do_GetService("@mozilla.org/ImsAlarmservice;1"));
     if(gAlarmService){
      LOGE("=================>gAlarmService");
      gAlarmService->ImsStopAlarm(alarmId);
    }
    mAlarmTimerId.erase(iTimerId);
  }else{
    LOGE("LgeImsAPI:: callStopHalTimer :: Timer Not Found");
  }
}
void
LgeImsAPI::StopHalTimer(int32_t iTimerId)
{
  LOGE("LgeImsAPI:: StopHalTimer");
  NS_DispatchToMainThread(NS_NewRunnableMethodWithArg<int32_t>(this, &LgeImsAPI::callStopHalTimer, iTimerId));
}

void
LgeImsAPI::notifyToObserverService(nsString CallID)
{
  LOGE("LgeImsAPI::notifyToObserverService()");
  nsCOMPtr<nsIObserverService> obs = services::GetObserverService();
  /*


                                                                   */
  obs->NotifyObservers(NULL, "ims-incoming_call", CallID.get()/*                                    */);
}

void
LgeImsAPI::NotifyImsIncomingCall(nsString CallID)
{
  LOGE("LgeImsAPI::NotifyImsIncomingCall()");
  NS_DispatchToMainThread(NS_NewRunnableMethodWithArg<nsString>(this, &LgeImsAPI::notifyToObserverService, CallID));
}

void
LgeImsAPI::retryDataAddObservers(nsITimer* aTimer, void* aClosure)
{
  LOGI("LgeImsAPI::retryDataAddObservers()");
  LgeImsAPI* self = static_cast<LgeImsAPI*>(aClosure);
  if (self) {
    if(aTimer) {
      self->HandleRetryDataAddObservers();
      aTimer->Cancel();
      aTimer = nullptr;
  //
    }
    self = nullptr;
  } else {
    LOGE("LgeImsAPI::retryDataAddObservers()::Creating Object failed");
  }
}

int retryDataObserver = 0;
void
LgeImsAPI::HandleRetryDataAddObservers()
{
  LOGI("LgeImsAPI::HandleRetryDataAddObservers()");
  nsresult rv;
  nsCOMPtr<nsIMobileConnectionService> service =
  do_GetService(NS_MOBILE_CONNECTION_SERVICE_CONTRACTID);
  if(nullptr == service) {
    LOGE("LgeImsAPI::HandleRetryDataAddObservers:: service is null");
  }else{
    nsCOMPtr<nsIMobileConnection> connection;
    retryDataObserver++;
    service->GetItemByServiceId(mClientId, getter_AddRefs(connection));
    if(nullptr == connection) {
      LOGE("LgeImsAPI::HandleRetryDataAddObservers:: connection is still null - %d", retryDataObserver);
      mRetryDataAddObserver = do_CreateInstance("@mozilla.org/timer;1");
      if (!mRetryDataAddObserver) {
        LOGE("LgeImsAPI::Observe::mRetryDataAddObserver == null");
        return ;
      }
      nsIEventTarget* target = static_cast<nsIEventTarget*>(imsXpcomThread);
      rv = mRetryDataAddObserver->SetTarget(target);
      if(NS_FAILED(rv)) {
        LOGE("LgeImsAPI::Observe::SetTarget failed");
        return ;
      }
      LgeImsAPI* agent = LgeImsAPI::GetSingleton();
      rv = mRetryDataAddObserver->InitWithFuncCallback(retryDataAddObservers,
                                                  agent,
                                                  2000,
                                                  nsITimer::TYPE_ONE_SHOT);
      if(NS_FAILED(rv)) {
        LOGE("LgeImsAPI::Observe::InitWithFuncCallback failed") ;
        return ;
      }
    } else {
      rv = connection->RegisterListener(this);
      if (NS_FAILED(rv)) {
        LOGE("LgeImsAPI::Observe:: HandleRetryDataAddObservers to connection failed");
      }
    }
  }
}

void
LgeImsAPI::HandleImsWithoutSIMCard()
{
  nsCOMPtr<nsIRadioInterfaceLayer> gRIL(do_GetService("@mozilla.org/ril;1"));
  nsCOMPtr<nsIRadioInterface> radioInterface;
  if (gRIL) {
    gRIL->GetRadioInterface(0, getter_AddRefs(radioInterface));
  }
  if(radioInterface) {
    nsCOMPtr<nsIRilContext> rilCtx;
    radioInterface->GetRilContext(getter_AddRefs(rilCtx));
    if(rilCtx) {
      uint32_t aCardState = nsIIccProvider::CARD_STATE_UNDETECTED;
      rilCtx->GetCardState(&aCardState);
      if(aCardState == nsIIccProvider::CARD_STATE_UNDETECTED) {
        LOGE("LgeImsAPI::HandleImsWithoutSIMCard:: SIM is not present");
        imsNativeWrapper->NativeSystemInterface(imsImpl);
      } else {
        LOGE("LgeImsAPI::HandleImsWithoutSIMCard:: SIM is present");
      }
    }
  }
}
void
LgeImsAPI::ImsWithoutSIMCard(nsITimer* aTimer, void* aClosure)
{
  LOGI("LgeImsAPI::ImsWithoutSIMCard()");
  LgeImsAPI* self = static_cast<LgeImsAPI*>(aClosure);
  if (self) {
    if(aTimer) {
      self->HandleImsWithoutSIMCard();
      aTimer->Cancel();
      aTimer = nullptr;
    }
    self = nullptr;
  } else {
    LOGE("LgeImsAPI::ImsWithoutSIMCard()::Creating Object failed");
  }
}

NS_IMETHODIMP
LgeImsAPI::OnPhoneStateChanged(int32_t item, int32_t param1, int32_t param2, nsTArray<nsString> &paramEx)
{
  LOGI("LgeImsAPI::OnPhoneStateChanged()");
}

NS_IMETHODIMP
LgeImsAPI::NotifyLteConnectionStatus(nsTArray<int32_t> &param)
{
  LOGI("LgeImsAPI::NotifyLteConnectionStatus()");
}
