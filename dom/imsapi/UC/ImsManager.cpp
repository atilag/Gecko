/*




*/

#include "ImsManager.h"
#include "IImsService.h"
#include <binder/Parcel.h>
#include <binder/IServiceManager.h>
#include "../../../dom/system/gonk/android_audio/AudioSystem.h"
#include "ImsCall.h"
#include "LgeImsAPI.h"
#include "nsIRadioInterfaceLayer.h"
#include "nsServiceManagerUtils.h"

#undef LOG_TAG
#define LOG_TAG "LGE_IMS_API_IMSMANAGER"
#include <utils/Log.h>
#include <dlfcn.h>
using namespace android ;

#define LOGI(args...) \
     __android_log_print(ANDROID_LOG_INFO, LOG_TAG, ## args)

#define LOGE(args...) \
     __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, ## args)

NS_IMPL_ISUPPORTS(ImsManager, nsIImsUCManager)
ImsManager* ImsManager::m_pIUCIManagerObj = NULL;

ImsManager::ImsManager()
{
  mEcbm = NULL;
}

ImsManager::~ImsManager()
{
}

void ImsManager::deleteInstance()
{
  delete m_pIUCIManagerObj;
  m_pIUCIManagerObj = 0;
}

ImsManager* ImsManager::getInstance()
{
  LOGE("ImsManager::getInstance()");
  if (NULL == m_pIUCIManagerObj) {
    LOGE("ImsManager::getInstance()::m_pIUCIManagerObj::NULL");
    m_pIUCIManagerObj = new ImsManager();
    m_pIUCIManagerObj->createImsService();
  }
  return m_pIUCIManagerObj;
}

//
NS_IMETHODIMP
ImsManager::Open(int32_t serviceClass, nsIImsConnectionStateListener *listener, int32_t *_retval)
{
  LOGE("ImsManager::open - %d", serviceClass);
  m_pImsManagerListener = listener;
  m_nServiceID = mImsService->open(0/*       */, serviceClass, reinterpret_cast<int>(this));
  LOGE("ImsManager::open m_nServiceID - %d", m_nServiceID);
  *_retval = m_nServiceID;
  return NS_OK;
}

NS_IMETHODIMP
ImsManager::Close(int32_t serviceId)
{
  if(mImsService != NULL)
    mImsService->close(m_nServiceID);
  //
  mEcbm = NULL;
  return NS_OK;
}

NS_IMETHODIMP
ImsManager::MakeCall(int32_t serviceId,  int32_t serviceType, int32_t callType, nsIImsStreamMediaProfile *mediaProfile, nsTArray<nsString> &callees, nsIImsCallListener *listener, nsIImsCall* *iReturn)
{
  LOGI("ImsManager::makeCall:: serviceType - %d, callType - %d, serviceId - %d, m_nServiceID - %d", serviceType, callType, serviceId, m_nServiceID);
 //
 //

  ImsCallProfile *profile = new ImsCallProfile(serviceType, callType);

  ImsCall *call = new ImsCall(profile);
  call->setListener(listener);
  ImsCallSession *session = createCallSession(m_nServiceID, profile);

  NS_LossyConvertUTF16toASCII convertAddr(callees[0]);
  LOGE("ImsManager::MakeCall::dial %s", convertAddr.get());
#if TBD

  bool isConferenceUri = profile.getCallExtraBoolean(
                TelephonyProperties.EXTRAS_IS_CONFERENCE_URI, false);
        if (!isConferenceUri && (callees != NULL) && (callees.length == 1)) {
            call.start(session, callees[0]);
        } else
#endif
  {
    call->start(session, callees[0]);
  }
  LOGE("ImsManager::MakeCall::set media param::vsid=281026560;call_state=2");
  android::String8 str8 = android::String8("vsid=281026560;call_state=2");
  android::AudioSystem::setParameters(0, str8);
  *iReturn = static_cast<nsIImsCall *>(call);
  return NS_OK;
}

NS_IMETHODIMP
ImsManager::TakeCall(int32_t serviceId, nsTArray<nsString> &callId, nsIImsCallListener *listener, nsIImsCall* *iReturn)
{
  LOGE("ImsManager::takeCall serviceId = %d", serviceId);
  NS_LossyConvertUTF16toASCII convertCallId(callId[0]);
  LOGE("ImsManager::takeCall::callId - %s", convertCallId.get());
  int64_t IImsCallSessionID;
  ImsCallSession *callsession;
  ImsCall *call;

  if(mImsService != NULL)
  {
    IImsCallSessionID = mImsService->getPendingCallSession(serviceId, (char*)convertCallId.get());
    callsession = new ImsCallSession(IImsCallSessionID);
    call = new ImsCall(callsession->getCallProfile());
    ImsCallProfile *pProfile = callsession->getCallProfile();
    if(pProfile != nullptr){
      nsString caller = pProfile->getCallExtra(pProfile->EXTRA_OI);
      NS_LossyConvertUTF16toASCII convertCallerName(caller);
      LOGE("ImsManager::takeCall::callerName - %s", convertCallerName.get());
      call->SetCallerPartyNum(caller);
      call->sCallerPartyNum = caller;
    }
  }
  else
      return NS_OK;
  call->attachSession(callsession);
  call->setListener(listener);
  LOGE("ImsManager::takeCall::set media param::vsid=281026560;call_state=2");
  android::String8 str8 = android::String8("vsid=281026560;call_state=2");
  android::AudioSystem::setParameters(0, str8);
  *iReturn = static_cast<nsIImsCall *>(call);

  return NS_OK;
}

bool
ImsManager::createImsService()
{
  sp<IServiceManager> ism = defaultServiceManager();
  sp<IBinder> binder;

  binder = ism->checkService(String16("ims"));
  if (binder == 0) {
      return 0;
  }
  binder = ism->getService(String16("ims"));
  if (binder != 0) {
    mImsService = interface_cast<IImsService>(binder);
    //
    LOGE("ImsManager::createImsService is done");
    return 1;
  }
  else
    return 0;
}


ImsCallSession*
ImsManager::    createCallSession(int serviceId,ImsCallProfile *profile)
{
  LOGI("ImsManager::createCallSession::serviceId - %d", serviceId);
  android::Parcel ims_parcel;
  if(profile)
  {
    profile->writeToParcel(ims_parcel, 1);
  }
  profile = nullptr;
  return new ImsCallSession(mImsService->createCallSession(serviceId, ims_parcel));
}

void
ImsManager::checkAndThrowExceptionIfServiceUnavailable()
{
  if (mImsService == NULL) {
    createImsService();

    if (mImsService == NULL) {
      LOGE("ImsManager::Service is not available");
      return;
    }
    LOGE("ImsManager::Service is available");
  }
  else
  {
    sp<IServiceManager> ism = defaultServiceManager();
    if (ism->checkService(String16("ims")) == 0) {
      LOGE("ImsManager::Service is not available");
      return;
    }
    LOGE("ImsManager::Service is available");
  }
}

NS_IMETHODIMP
ImsManager::GetEcbmInterface(int serviceId, nsIImsEcbm* *iReturn)
{
  if (mEcbm == NULL) {
    checkAndThrowExceptionIfServiceUnavailable();

    int64_t iEcbm = mImsService->getEcbmInterface(serviceId);

    if (iEcbm == 0) {
      LOGE("ImsManager::ECBM is not supported ");
    }
    mEcbm = new ImsEcbm(iEcbm);
  }
  *iReturn = static_cast<nsIImsEcbm *>(mEcbm);

  return NS_OK;
}

void
ImsManager::binderDied(const wp<IBinder>& who)
{
  LOGE("ImsManager::binderDied::LibIms died");
  mImsService = NULL;
  mEcbm = NULL;
}

void
ImsManager::registrationConnected()
{
  LOGE("ImsManager::registrationConnected");

  if(m_pImsManagerListener)
  {
    LOGE("ImsManager::registrationConnected sending to Telephony");
    m_pImsManagerListener->OnImsConnected();
  }
}

void
ImsManager::registrationDisconnected(const android::Parcel &pParcel)
{
  LOGE("ImsManager::registrationDisconnected");
  if(m_pImsManagerListener)
    m_pImsManagerListener->OnImsDisconnected();
}

void
ImsManager::notifyIncomingCall(int phoneID, int serviceID, const char* callId)
{
  LOGE("ImsManager::notifyIncomingCall");
  LgeImsAPI* agent = LgeImsAPI::GetSingleton();
  nsString nscallID;
  nscallID.AssignASCII(callId);
  agent->NotifyImsIncomingCall(nscallID);
}

void ImsManager::enteredECBM()
{
    return;
}

void ImsManager::exitedECBM()
{
    return;
}

void ImsManager::set911State(int32_t state, int32_t ecbm)
{
  LOGE("ImsManager::set911State::state - %d, ecbm - %d", state, ecbm);
  nsCOMPtr<nsIRadioInterfaceLayer> gRIL = (do_GetService("@mozilla.org/ril;1"));
  nsCOMPtr<nsIRadioInterface> radioInterface;
  if(gRIL) {
    gRIL->GetRadioInterface(0, getter_AddRefs(radioInterface));
  }else {
    LOGI("ImsManager::set911State::gRIL is Null");
  }
  if(radioInterface) {
    radioInterface->SetE911StateIF(state, ecbm);
  }else {
    LOGI("ImsManager::set911State::radioInterface is Null");
  }
}

void ImsManager::onMessageEx(const android::Parcel &pParcel, Parcel *pOutParcel)
{
    return;
}

void ImsManager::onMessage(const android::Parcel& pParcel)
{
  return;
}
