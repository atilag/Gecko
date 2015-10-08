#include <android/log.h>
#include "nsINetworkService.h"
#include "ImsEventConstantMap.h"
#include "nsIMobileNetworkInfo.h"
#include "nsIMobileConnectionInfo.h"
#include "nsNetUtil.h"
#include "ApnMang.h"
#include "NativeWrapper.h"
#undef LOG_TAG
#define LOG_TAG "IMS_APN_MANG"
#include <utils/Log.h>
#define LOGI(args...) \
     __android_log_print(ANDROID_LOG_INFO, LOG_TAG, ## args)

#define LOGE(args...) \
     __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, ## args)
ImsApn* ImsApn::sImsApn = NULL;
EmergencyApn* EmergencyApn::sEmergencyApn = NULL;
ApnMng* ApnMng::sApnMng = NULL;

NS_IMPL_ISUPPORTS(ImsApn::ImsApnCB, nsIGetNetIdCallback)
ImsApn::ImsApn()
{
  LOGI("ImsApn::ImsApn");
  mLocalAddress = nullptr;
  mGateway = nullptr;
  GatewayCnt = 0;
  NS_GetCurrentThread(getter_AddRefs(imsApnThread));
  if(imsApnThread == nullptr)
  {
    LOGE("ImsApn::ImsApn::imsApnThread == null");
  }
  CBObj = new ImsApnCB();
}
ImsApn::~ImsApn()
{
  LOGI("ImsApn::~ImsApn");
  mPcfcsAddresses_ipv4.Clear();
  mPcfcsAddresses_ipv6.Clear();
  if(mLocalAddress != nullptr)
    free((char *)mLocalAddress);
  imsApnThread = nullptr;
  delete CBObj;
}

ImsApn*
ImsApn::GetImsApn()
{
  if(nullptr == sImsApn)
  {
    sImsApn = new ImsApn();
  }
  return sImsApn;
}

void
ImsApn::sendConnectedEventToNative(int32_t iState)
{
  LOGI("ImsApn::sendConnectedEventToNative() - %d", iState);
  nsresult rv;
  if(bNotifyPcscfReceived){
    android::Parcel parcel;
    parcel.writeInt32(APN_IMS); //
    if(iState == nsINetworkInterface::NETWORK_STATE_CONNECTED) {
      parcel.writeInt32(DATA_STATE_CONNECTED);
    }else {
      parcel.writeInt32(DATA_STATE_DISCONNECTED);
    }
    NativeWrapper *imsNativeWrapper = NativeWrapper::GetInstance();
    if(nullptr == imsNativeWrapper) {
      LOGI("ImsApn::sendConnectedEventToNative()::imsNativeWrapper is null");
      return;
    }
    imsNativeWrapper->sendData2NativeImpl(NOTIFY_DATA_CONNECTION_STATE_CHANGED, parcel);
    bNotifyPcscfReceived = false;
  }else {
    //
    LOGE("ImsApn::sendConnectedEventToNative()::mRetryPcscfTimer");
    mRetryPcscfTimer = do_CreateInstance("@mozilla.org/timer;1");
    if (!mRetryPcscfTimer) {
    LOGE("ImsApn::sendConnectedEventToNative::mRetryPcscfTimer == null");
    return;
    }
    nsIEventTarget* target = static_cast<nsIEventTarget*>(imsApnThread);
    rv = mRetryPcscfTimer->SetTarget(target);
    if(NS_FAILED(rv)) {
    LOGE("ImsApn::sendConnectedEventToNative::SetTarget failed");
    return;
    }
    ImsApn* agent = ImsApn::GetImsApn();
    rv = mRetryPcscfTimer->InitWithFuncCallback(retryPcscfTimerCallback,
                                    agent,
                                    2000,
                                    nsITimer::TYPE_ONE_SHOT);
    if(NS_FAILED(rv)) {
    LOGE("ImsApn::sendConnectedEventToNative::InitWithFuncCallback failed") ;
    return;
    }
  }
}

void
ImsApn::retryPcscfTimerCallback(nsITimer* aTimer, void* aClosure)
{
  LOGE("ImsApn::retryPcscfTimerCallback()");
  ImsApn* self = static_cast<ImsApn*>(aClosure);
  if (self) {
    if(aTimer) {
      self->HandleRetryPcscfTimerEvent();
      aTimer->Cancel();
      aTimer = nullptr;
  //
    }
    self = nullptr;
  } else {
    LOGE("ImsApn::retryPcscfTimerCallback()::Creating Object failed");
  }
}
void
ImsApn::HandleRetryPcscfTimerEvent()
{
  LOGE("ImsApn::HandleRetryPcscfTimerEvent - %d, state - %d", isEnabledPreviously, state);
  if(isEnabledPreviously == true) {
    android::Parcel parcel;
    parcel.writeInt32(APN_IMS); //
    if(state == nsINetworkInterface::NETWORK_STATE_CONNECTED) {
      parcel.writeInt32(DATA_STATE_CONNECTED);
    }else {
      parcel.writeInt32(DATA_STATE_DISCONNECTED);
    }
    NativeWrapper *imsNativeWrapper = NativeWrapper::GetInstance();
    if(nullptr == imsNativeWrapper) {
      LOGI("ImsApn::HandleRetryPcscfTimerEvent()::imsNativeWrapper is null");
      return;
    }
    imsNativeWrapper->sendData2NativeImpl(NOTIFY_DATA_CONNECTION_STATE_CHANGED, parcel);
  }
}

ImsApn::ImsApnCB::ImsApnCB()
{
}
ImsApn::ImsApnCB::~ImsApnCB()
{
  LOGI("ImsApnCB::~ImsApnCB");
}

NS_IMETHODIMP
ImsApn::ImsApnCB::OnGetNetId(int32_t id)
{
  //
  //
  LOGI("ImsApnCB::OnGetNetId id - %d", id);
  (ImsApn::GetImsApn())->nIfaceId = id;
}


NS_IMPL_ISUPPORTS(EmergencyApn::EmergencyApnCB, nsIGetNetIdCallback)
EmergencyApn::EmergencyApn()
{
  LOGI("EmergencyApn::EmergencyApn");
  mLocalAddress = nullptr;
  mGateway = nullptr;
  GatewayCnt = 0;
  NS_GetCurrentThread(getter_AddRefs(emergencyApnThread));
  if(emergencyApnThread == nullptr)
  {
    LOGE("ImsApn::ImsApn::emergencyApnThread == null");
  }
  CBObj = new EmergencyApnCB();
}
EmergencyApn::~EmergencyApn()
{
  LOGI("EmergencyApn::~EmergencyApn");
  mPcfcsAddresses_ipv4.Clear();
  mPcfcsAddresses_ipv6.Clear();
  if(mLocalAddress != nullptr)
    free((char *)mLocalAddress);
  emergencyApnThread = nullptr;
  delete CBObj;
}

EmergencyApn*
EmergencyApn::GetEmergencyApn()
{
  if(nullptr == sEmergencyApn)
  {
    sEmergencyApn = new EmergencyApn();
  }
  return sEmergencyApn;
}

void
EmergencyApn::sendConnectedEventToNative(int32_t iState)
{
  LOGE("EmergencyApn::sendConnectedEventToNative()");
  nsresult rv;
  if(bNotifyPcscfReceived){
    android::Parcel parcel;
    parcel.writeInt32(APN_EMERGENCY); //
    if(state == nsINetworkInterface::NETWORK_STATE_CONNECTED) {
      parcel.writeInt32(DATA_STATE_CONNECTED);
    }else {
      parcel.writeInt32(DATA_STATE_DISCONNECTED);
    }
    NativeWrapper *imsNativeWrapper = NativeWrapper::GetInstance();
    if(nullptr == imsNativeWrapper) {
      LOGI("EmergencyApn::sendConnectedEventToNative()::imsNativeWrapper is null");
      return;
    }
    imsNativeWrapper->sendData2NativeImpl(NOTIFY_DATA_CONNECTION_STATE_CHANGED, parcel);
    bNotifyPcscfReceived = false;
  }else {
    //
    LOGE("EmergencyApn::sendConnectedEventToNative()::mRetryPcscfTimer");
    mRetryPcscfTimer = do_CreateInstance("@mozilla.org/timer;1");
    if (!mRetryPcscfTimer) {
    LOGE("EmergencyApn::sendConnectedEventToNative::mRetryPcscfTimer == null");
    return;
    }
    nsIEventTarget* target = static_cast<nsIEventTarget*>(emergencyApnThread);
    rv = mRetryPcscfTimer->SetTarget(target);
    if(NS_FAILED(rv)) {
    LOGE("EmergencyApn::sendConnectedEventToNative::SetTarget failed");
    return;
    }
    EmergencyApn* agent = EmergencyApn::GetEmergencyApn();
    rv = mRetryPcscfTimer->InitWithFuncCallback(retryPcscfTimerCallback,
                                    agent,
                                    2000,
                                    nsITimer::TYPE_ONE_SHOT);
    if(NS_FAILED(rv)) {
    LOGE("EmergencyApn::sendConnectedEventToNative::InitWithFuncCallback failed") ;
    return;
    }
  }
}

void
EmergencyApn::retryPcscfTimerCallback(nsITimer* aTimer, void* aClosure)
{
  LOGE("EmergencyApn::retryPcscfTimerCallback()");
  EmergencyApn* self = static_cast<EmergencyApn*>(aClosure);
  if (self) {
    if(aTimer) {
      self->HandleRetryPcscfTimerEvent();
      aTimer->Cancel();
      aTimer = nullptr;
  //
    }
    self = nullptr;
  } else {
    LOGE("ImsApn::retryPcscfTimerCallback()::Creating Object failed");
  }
}
void
EmergencyApn::HandleRetryPcscfTimerEvent()
{
  if(isEnabledPreviously == true) {
    android::Parcel parcel;
    parcel.writeInt32(APN_EMERGENCY); //
    if(state == nsINetworkInterface::NETWORK_STATE_CONNECTED) {
      parcel.writeInt32(DATA_STATE_CONNECTED);
    }else {
      parcel.writeInt32(DATA_STATE_DISCONNECTED);
    }
    NativeWrapper *imsNativeWrapper = NativeWrapper::GetInstance();
    if(nullptr == imsNativeWrapper) {
      LOGI("EmergencyApn::HandleRetryPcscfTimerEvent()::imsNativeWrapper is null");
      return;
    }
    imsNativeWrapper->sendData2NativeImpl(NOTIFY_DATA_CONNECTION_STATE_CHANGED, parcel);
  }
}

EmergencyApn::EmergencyApnCB::EmergencyApnCB()
{
}
EmergencyApn::EmergencyApnCB::~EmergencyApnCB()
{
  LOGI("EmergencyApnCB::~EmergencyApnCB");
}

NS_IMETHODIMP
EmergencyApn::EmergencyApnCB::OnGetNetId(int32_t id)
{
  //
  //
  LOGI("EmergencyApn::OnGetNetId id - %d", id);
  (EmergencyApn::GetEmergencyApn())->nIfaceId = id;
}

ApnMng::ApnMng()
{
  LOGI("ApnMng::ApnMng");
  sApnMng = nullptr;
}
ApnMng::~ApnMng()
{
  LOGI("ApnMng::~ApnMng");
  delete sApnMng;
  delete ImsApn::GetImsApn();
}

ApnMng*
ApnMng::GetApnMng()
{
  LOGI("ApnMng::GetApnMng");
  if(nullptr == sApnMng)
  {
    sApnMng = new ApnMng();
    ImsApn* pImsApn = ImsApn::GetImsApn();
    if(pImsApn == nullptr) {
      LOGI("ApnMng::GetApnMng::ImsApn is null");
    }
    EmergencyApn* pEmerApn = EmergencyApn::GetEmergencyApn();
    if(pEmerApn == nullptr) {
      LOGI("ApnMng::GetApnMng::EmergencyApn is null");
    }
  }
  return sApnMng;
}

void
ApnMng::SetApnType(int32_t apnType)
{
  LOGI("ApnMng::SetApnType:: %d", apnType);
  if(nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS == apnType){
    ImsApn* Apn = ImsApn::GetImsApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::SetApnType::ImsApn is Null");
      return;
    }
    Apn->ApnType = apnType;
  }
  else if(nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY == apnType) {
    EmergencyApn* Apn = EmergencyApn::GetEmergencyApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::SetApnType::EmergencyApn is Null");
      return;
    }
    Apn->ApnType = apnType;
  }
  else {
    LOGE("ApnMng::SetApnType::Not handle");
  }
}

void
ApnMng::SetLocalAddress(const char * mLocalAddress, int32_t iApnType)
{
  LOGI("ApnMng::SetLocalAddress:: Address - %s, ApnType - %d", mLocalAddress, iApnType);
  if(nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS == iApnType){
    ImsApn* Apn = ImsApn::GetImsApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::SetLocalAddress::ImsApn is Null");
      return;
    }
    if(Apn->mLocalAddress != nullptr)
      free((char *)Apn->mLocalAddress);
    Apn->mLocalAddress = (char*)malloc(sizeof(char*) * strlen(mLocalAddress) + 1);
    strcpy((char*)Apn->mLocalAddress , mLocalAddress);
  }
  else if(nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY == iApnType) {
    EmergencyApn* Apn = EmergencyApn::GetEmergencyApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::SetLocalAddress::EmergencyApn is Null");
      return;
    }
    if(Apn->mLocalAddress != nullptr)
      free((char *)Apn->mLocalAddress);
    Apn->mLocalAddress = (char*)malloc(sizeof(char*) * strlen(mLocalAddress) + 1);
    strcpy((char*)Apn->mLocalAddress , mLocalAddress);
  }
  else {
    LOGE("ApnMng::SetLocalAddress::Not handle");
  }
}

char*
ApnMng::GetLocalAddress(int32_t ApnType, int32_t IPVersion)
{
  LOGI("ApnMng::GetLocalAddress:: ApnType - %d, IPVersion - %d", ApnType, IPVersion);
  if(ApnType == APN_IMS) {
    ImsApn* Apn = ImsApn::GetImsApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::SetLocalAddress::ImsApn is Null");
      return nullptr;
    }
    LOGI("ApnMng::GetLocalAddress:: LocalAddress - %s", Apn->mLocalAddress);
    return Apn->mLocalAddress;
  }
  else if(ApnType == APN_EMERGENCY){
    EmergencyApn* Apn = EmergencyApn::GetEmergencyApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::SetLocalAddress::ImsApn is Null");
      return nullptr;
    }
    LOGI("ApnMng::GetLocalAddress:: LocalAddress - %s", Apn->mLocalAddress);
    return Apn->mLocalAddress;
  }
  else {
    LOGE("ApnMng::GetLocalAddress::Not handle");
  }
  return nullptr;
}

void
ApnMng::SetPcscfAddress(nsTArray<nsString> &aPcfcsAddresses_ipv4, nsTArray<nsString> &aPcfcsAddresses_ipv6, int32_t iApnType)
{
  LOGI("ApnMng::SetPcscfAddress:: ApnType - %d", iApnType);
  if(iApnType == nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS) {
    ImsApn* Apn = ImsApn::GetImsApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::SetLocalAddress::ImsApn is Null");
      return ;
    }
    Apn->mPcfcsAddresses_ipv4.Clear();
    Apn->mPcfcsAddresses_ipv6.Clear();
    Apn->mPcfcsAddresses_ipv4 = aPcfcsAddresses_ipv4;
    Apn->mPcfcsAddresses_ipv6 = aPcfcsAddresses_ipv6;
  }
  else if(iApnType == nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY){
    EmergencyApn* Apn = EmergencyApn::GetEmergencyApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::SetLocalAddress::ImsApn is Null");
      return ;
    }
    Apn->mPcfcsAddresses_ipv4.Clear();
    Apn->mPcfcsAddresses_ipv6.Clear();
    Apn->mPcfcsAddresses_ipv4 = aPcfcsAddresses_ipv4;
    Apn->mPcfcsAddresses_ipv6 = aPcfcsAddresses_ipv6;
  }
  else {
    LOGE("ApnMng::SetPcscfAddress::Not handle");
  }
}

const char**
ApnMng::GetPcscfAddress(int32_t ApnType, int32_t IPVersion)
{
  LOGI("ApnMng::GetPcscfAddress:: ApnType - %d, IPVersion - %d", ApnType, IPVersion);
  nsTArray<nsString> mPcfcsAddresses;
  const char** mPcscfAddress = nullptr;
  if(ApnType == APN_IMS) {
    ImsApn* Apn = ImsApn::GetImsApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::SetLocalAddress::ImsApn is Null");
      return nullptr;
    }
    if(4 == IPVersion) {
      mPcfcsAddresses = Apn->mPcfcsAddresses_ipv4;
    }else if(6 == IPVersion) {
      mPcfcsAddresses = Apn->mPcfcsAddresses_ipv6;
    }else {
      return nullptr;
    }
  }
  else if(ApnType == APN_EMERGENCY){
    EmergencyApn* Apn = EmergencyApn::GetEmergencyApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::SetLocalAddress::ImsApn is Null");
      return nullptr;
    }
    if(4 == IPVersion) {
      mPcfcsAddresses = Apn->mPcfcsAddresses_ipv4;
    }else if(6 == IPVersion) {
      mPcfcsAddresses = Apn->mPcfcsAddresses_ipv6;
    }else {
      return nullptr;
    }
  }
  else {
    LOGE("ApnMng::GetPcscfAddress::Not handle");
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
  return nullptr;
}

void
ApnMng::SetConnectionState(int32_t iApnType, int32_t iState)
{
  LOGI("ApnMng::SetConnectionState:: Apn - %d, State- %d", iApnType, iState);
  if(nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS == iApnType){
    ImsApn* Apn = ImsApn::GetImsApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::SetConnectionState::ImsApn is Null");
      return;
    }
    Apn->state = iState;
  }
  else if(nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY == iApnType) {
    EmergencyApn* Apn = EmergencyApn::GetEmergencyApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::SetConnectionState::EmergencyApn is Null");
      return;
    }
    Apn->state = iState;
  }
  else {
    LOGE("ApnMng::SetConnectionState::Not handle");
  }
}

int32_t
ApnMng::GetConnectionState(int32_t ApnType)
{
  LOGI("ApnMng::GetConnectionState:: ApnType - %d", ApnType);
  if(ApnType == nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS) {
    ImsApn* Apn = ImsApn::GetImsApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::GetConnectionState::ImsApn is Null");
      return -1;
    }
    LOGI("ApnMng::GetConnectionState:: State - %d", Apn->state);
    return Apn->state;
  }
  else if(ApnType == nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY){
    EmergencyApn* Apn = EmergencyApn::GetEmergencyApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::GetConnectionState::ImsApn is Null");
      return -1;
    }
    LOGI("ApnMng::GetConnectionState:: State - %d", Apn->state);
    return Apn->state;
  }
  else {
    LOGE("ApnMng::GetConnectionState::Not handle");
  }
  return -1;
}

void
ApnMng::SetName(int32_t iApnType, nsString mName)
{
  NS_LossyConvertUTF16toASCII convertName(mName);
  LOGI("ApnMng::SetName:: Apn - %d, Name- %s", iApnType, convertName.get());
  if(nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS == iApnType){
    ImsApn* Apn = ImsApn::GetImsApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::SetName::ImsApn is Null");
      return;
    }
    Apn->mName = mName;
  }
  else if(nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY == iApnType) {
    EmergencyApn* Apn = EmergencyApn::GetEmergencyApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::SetName::EmergencyApn is Null");
      return;
    }
    Apn->mName = mName;
  }
  else {
    LOGE("ApnMng::SetName::Not handle");
  }
}

nsString
ApnMng::GetName(int32_t ApnType)
{
  LOGI("ApnMng::GetName:: ApnType - %d", ApnType);
  nsString name;
  if(ApnType == nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS) {
    ImsApn* Apn = ImsApn::GetImsApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::GetName::ImsApn is Null");
    }
    NS_LossyConvertUTF16toASCII convertName(Apn->mName);
    LOGI("ApnMng::GetName:: Name - %s", convertName.get());
    name = Apn->mName;
  }
  else if(ApnType == nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY){
    EmergencyApn* Apn = EmergencyApn::GetEmergencyApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::GetName::ImsApn is Null");
    }
    NS_LossyConvertUTF16toASCII convertName(Apn->mName);
    LOGI("ApnMng::GetName:: Name - %s", convertName.get());
    name = Apn->mName;
  }
  else {
    LOGE("ApnMng::GetName::Not handle");
  }
  return name;
}

void
ApnMng::SetGatewayCount(int32_t iApnType, int32_t iCount)
{
  LOGI("ApnMng::SetGatewayCount:: Apn - %d, iCount- %d", iApnType, iCount);
  if(nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS == iApnType){
    ImsApn* Apn = ImsApn::GetImsApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::SetGatewayCount::ImsApn is Null");
      return;
    }
    Apn->GatewayCnt = iCount;
  }
  else if(nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY == iApnType) {
    EmergencyApn* Apn = EmergencyApn::GetEmergencyApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::SetGatewayCount::EmergencyApn is Null");
      return;
    }
    Apn->GatewayCnt = iCount;
  }
  else {
    LOGE("ApnMng::SetGatewayCount::Not handle");
  }
}

int32_t
ApnMng::GetGatewayCount(int32_t ApnType)
{
  LOGI("ApnMng::GetGatewayCount:: ApnType - %d", ApnType);
  if(ApnType == nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS) {
    ImsApn* Apn = ImsApn::GetImsApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::GetGatewayCount::ImsApn is Null");
      return 0;
    }
    LOGI("ApnMng::GetGatewayCount:: Count - %d", Apn->GatewayCnt);
    return Apn->GatewayCnt;
  }
  else if(ApnType == nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY){
    EmergencyApn* Apn = EmergencyApn::GetEmergencyApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::GetGatewayCount::ImsApn is Null");
      return 0;
    }
    LOGI("ApnMng::GetGatewayCount:: Count - %d", Apn->GatewayCnt);
    return Apn->GatewayCnt;
  }
  else {
    LOGE("ApnMng::GetGatewayCount::Not handle");
  }
  return 0;
}

void
ApnMng::SetGateways(int32_t iApnType, char16_t **pGateWay, uint32_t count)
{
  LOGI("ApnMng::SetGateways:: Apn - %d", iApnType);
  if(nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS == iApnType){
    ImsApn* Apn = ImsApn::GetImsApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::SetGateways::ImsApn is Null");
      return;
    }
    LOGI("ApnMng::SetGateways::Free mGateway[iCount]");
    for(uint32_t iCount = 0; iCount < Apn->GatewayCnt; iCount++){
      if(Apn->mGateway[iCount] != nullptr)
        NS_Free(Apn->mGateway[iCount]);
    }
    LOGI("ApnMng::SetGateways::Free mGateway");
    if(nullptr != Apn->mGateway)
      free(Apn->mGateway);
    Apn->mGateway = (char16_t**)malloc(sizeof(char16_t*) * count + 1);
    for(uint32_t iCount = 0; iCount < count; iCount++){
      Apn->mGateway[iCount] = NS_strdup(pGateWay[iCount]);
      NS_LossyConvertUTF16toASCII converGateway(Apn->mGateway[iCount]);
      LOGI("ApnMng::SetGateways::IMS Network Gateway is %s", converGateway.get());
    }
  }
  else if(nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY == iApnType) {
    EmergencyApn* Apn = EmergencyApn::GetEmergencyApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::SetGateways::EmergencyApn is Null");
      return;
    }
    LOGI("ApnMng::SetGateways::Free mGateway[iCount]");
    for(uint32_t iCount = 0; iCount < Apn->GatewayCnt; iCount++){
    if(Apn->mGateway[iCount] != nullptr)
      NS_Free(Apn->mGateway[iCount]);
    }
    LOGI("ApnMng::SetGateways::Free mGateway");
    if(nullptr != Apn->mGateway)
      free(Apn->mGateway);
    Apn->mGateway = (char16_t**)malloc(sizeof(char16_t*) * count + 1);
    for(uint32_t iCount = 0; iCount < count; iCount++){
      Apn->mGateway[iCount] = NS_strdup(pGateWay[iCount]);
      NS_LossyConvertUTF16toASCII converGateway(Apn->mGateway[iCount]);
      LOGI("ApnMng::SetGateways::IMS Network Gateway is %s", converGateway.get());
    }
  }
  else {
    LOGE("ApnMng::SetGateways::Not handle");
  }
}

char16_t **
ApnMng::GetGateways(int32_t ApnType)
{
  LOGI("ApnMng::GetGateways:: ApnType - %d", ApnType);
  if(ApnType == nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS) {
    ImsApn* Apn = ImsApn::GetImsApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::GetGateways::ImsApn is Null");
      return nullptr;
    }
    for(uint32_t iCount = 0; iCount < Apn->GatewayCnt; iCount++){
      NS_LossyConvertUTF16toASCII converGateway(Apn->mGateway[iCount]);
      LOGI("ApnMng::GetGateways::IMS Network Gateway is %s", converGateway.get());
    }
    return Apn->mGateway;
  }
  else if(ApnType == nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY){
    EmergencyApn* Apn = EmergencyApn::GetEmergencyApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::GetGateways::ImsApn is Null");
      return nullptr;
    }
    for(uint32_t iCount = 0; iCount < Apn->GatewayCnt; iCount++){
      NS_LossyConvertUTF16toASCII converGateway(Apn->mGateway[iCount]);
      LOGI("ApnMng::GetGateways::IMS Network Gateway is %s", converGateway.get());
    }
    return Apn->mGateway;;
  }
  else {
    LOGE("ApnMng::GetGateways::Not handle");
  }
  return nullptr;
}

void
ApnMng::SetNotifyPcscfReceived(int32_t iApnType, bool bReceived)
{
  LOGI("ApnMng::SetNotifyPcscfReceived:: Apn - %d, bReceived- %d", iApnType, bReceived);
  if(nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS == iApnType){
    ImsApn* Apn = ImsApn::GetImsApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::SetNotifyPcscfReceived::ImsApn is Null");
      return;
    }
    Apn->bNotifyPcscfReceived = bReceived;
  }
  else if(nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY == iApnType) {
    EmergencyApn* Apn = EmergencyApn::GetEmergencyApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::SetNotifyPcscfReceived::EmergencyApn is Null");
      return;
    }
    Apn->bNotifyPcscfReceived = bReceived;
  }
  else {
    LOGE("ApnMng::SetNotifyPcscfReceived::Not handle");
  }
}

void
ApnMng::sendConnectedEventToApnMang(int32_t iApnType, int32_t iState)
{
  LOGI("ApnMng::sendConnectedEventToApnMang:: Apn - %d, iState- %d", iApnType, iState);
  if(nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS == iApnType){
    ImsApn* Apn = ImsApn::GetImsApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::sendConnectedEventToApnMang::ImsApn is Null");
      return;
    }
    Apn->sendConnectedEventToNative(iState);
  }
  else if(nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY == iApnType) {
    EmergencyApn* Apn = EmergencyApn::GetEmergencyApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::sendConnectedEventToApnMang::EmergencyApn is Null");
      return;
    }
    Apn->sendConnectedEventToNative(iState);
  }
  else {
    LOGE("ApnMng::SetNotifyPcscfReceived::Not handle");
  }
}

void
ApnMng::SetIsEnabledPreviously(int32_t iApnType, bool bEnabled)
{
  LOGI("ApnMng::SetIsEnabledPreviously:: Apn - %d, bEnabled- %d", iApnType, bEnabled);
  if(nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS == iApnType){
    ImsApn* Apn = ImsApn::GetImsApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::SetIsEnabledPreviously::ImsApn is Null");
      return;
    }
    Apn->isEnabledPreviously = bEnabled;
  }
  else if(nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY == iApnType) {
    EmergencyApn* Apn = EmergencyApn::GetEmergencyApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::SetIsEnabledPreviously::EmergencyApn is Null");
      return;
    }
    Apn->isEnabledPreviously = bEnabled;
  }
  else {
    LOGE("ApnMng::SetIsEnabledPreviously::Not handle");
  }
}

void
ApnMng::SetInterfaceID(int32_t iApnType)
{
  LOGI("ApnMng::SetInterfaceID:: Apn - %d", iApnType);
  if(nsINetworkInterface::NETWORK_TYPE_MOBILE_IMS == iApnType){
    ImsApn* Apn = ImsApn::GetImsApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::SetInterfaceID::ImsApn is Null");
      return;
    }
    nsCOMPtr<nsINetworkService> gNetworkService(do_GetService("@mozilla.org/network/service;1"));
    if(gNetworkService == nullptr) {
      LOGE("ApnMng::SetInterfaceID::gNetworkService is null");
      return;
    }
    gNetworkService->GetNetIdWithCallback(Apn->mName, Apn->CBObj);
  }
  else if(nsINetworkInterface::NETWORK_TYPE_MOBILE_EMERGENCY == iApnType) {
    EmergencyApn* Apn = EmergencyApn::GetEmergencyApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::SetInterfaceID::EmergencyApn is Null");
      return;
    }
    nsCOMPtr<nsINetworkService> gNetworkService(do_GetService("@mozilla.org/network/service;1"));
    if(gNetworkService == nullptr) {
      LOGE("ApnMng::SetInterfaceID::gNetworkService is null");
      return;
    }
    gNetworkService->GetNetIdWithCallback(Apn->mName, Apn->CBObj);
  }
  else {
    LOGE("ApnMng::SetInterfaceID::Not handle");
  }
}

int32_t
ApnMng::GetInterfaceID(int32_t ApnType)
{
  LOGI("ApnMng::GetInterfaceID:: ApnType - %d", ApnType);
  if(ApnType == APN_IMS) {
    ImsApn* Apn = ImsApn::GetImsApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::GetInterfaceID::ImsApn is Null");
      return -1;
    }
    LOGI("ApnMng::GetInterfaceID::for IMS PDN::nIfaceId - %d", Apn->nIfaceId);
    return Apn->nIfaceId;
  }
  else if(ApnType == APN_EMERGENCY){
    EmergencyApn* Apn = EmergencyApn::GetEmergencyApn();
    if(Apn == nullptr) {
      LOGE("ApnMng::GetInterfaceID::ImsApn is Null");
      return -1;
    }
    LOGI("ApnMng::GetInterfaceID::for EMERGENCY PDN::nIfaceId - %d", Apn->nIfaceId);
    return Apn->nIfaceId;
  }
  else {
    LOGE("ApnMng::GetInterfaceID::Not handle");
  }
  return -1;
}
