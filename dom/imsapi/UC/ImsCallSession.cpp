/*








                                                                   */
#include "ImsCallSession.h"
#include <binder/Parcel.h>
#include <binder/IServiceManager.h>
#undef LOG_TAG
#define LOG_TAG "LGE_IMS_API_UC_SESSION"
#include <utils/Log.h>
#include <dlfcn.h>
#include "nsString.h"
#include "nsILgeImsUCManager.h"
#include "../../../dom/system/gonk/android_audio/AudioSystem.h"
#include "nsIRunnable.h"
#include "ImsCall.h"
#include "ImsUtils.h"

#define LOGI(args...) \
     __android_log_print(ANDROID_LOG_INFO, LOG_TAG, ## args)

#define LOGE(args...) \
     __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, ## args)
#define MAX_CALL_COUNT 6

#ifdef __cplusplus
extern "C" {
typedef IXPCOMCoreInterface* (*fn3) ();
}
#endif
using namespace android ;

ImsCallSession::ImsCallSession(int64_t SessionID)
{
  LOGI("ImsCallSession::ImsCallSession Session Id - %lld", SessionID);

  pMediaProfile = nullptr;
  profile = nullptr;
  reasonInfo = nullptr;
  mstate = nullptr;
  pImsCall = nullptr;
  isSessionAlive = true;
  m_nSessionID = SessionID;
  if(pXPCOMCoreInterfaceObj == NULL){
    sp<IServiceManager> ism = defaultServiceManager();
    sp<IBinder> binder;
    binder = ism->getService(String16("com.lge.ims.coreinterface"));
    if (binder != 0)
    {
      pXPCOMCoreInterfaceObj = interface_cast<IXPCOMCoreInterface>(binder);
      pXPCOMCoreInterfaceObj->XpcomCore_setListener(m_nSessionID, reinterpret_cast<int>(this));
    }
  }
}

ImsCallSession::ImsCallSession()
{
  LOGI("ImsCallSession::ImsCallSession");
  pMediaProfile = nullptr;
  profile = nullptr;
  reasonInfo = nullptr;
  mstate = nullptr;
  pImsCall = nullptr;
  isSessionAlive = true;
}

ImsCallSession::~ImsCallSession()
{
  LOGI("ImsCallSession::~ImsCallSession");
  if(pMediaProfile)
    delete pMediaProfile;
  if(profile)
    delete profile;
  if(reasonInfo)
    delete reasonInfo;
  if(mstate)
    delete mstate;
  pImsCall = nullptr;
}

void ImsCallSession::close()
{
  LOGI("ImsCallSession::close()");
  android::Parcel ims_parcel;
  ims_parcel.writeInt32(IImsCallSession::close);
  if(pXPCOMCoreInterfaceObj != nullptr) {
    pXPCOMCoreInterfaceObj->XpcomCore_sendData(m_nSessionID, ims_parcel);
  } else {
    LOGE("ImsCallSession::close()::pXPCOMCoreInterfaceObj is null");
  }
  pImsCall = nullptr;
  delete this;
}


int32_t ImsCallSession::getCallID()
{
  //
  LOGI("ImsCallSession::getCallID()");
  android::Parcel ims_parcel;
  android::Parcel ims_out_parcel;
  ims_parcel.writeInt32(IImsCallSession::getCallId);
  if(pXPCOMCoreInterfaceObj != nullptr) {
    pXPCOMCoreInterfaceObj->XpcomCore_sendData(m_nSessionID, ims_parcel, ims_out_parcel);
  } else {
    LOGE("ImsCallSession::getCallID()::pXPCOMCoreInterfaceObj is null");
  }
  ims_out_parcel.setDataPosition(0);
  return ims_out_parcel.readInt32();
}

ImsCallProfile *ImsCallSession::getCallProfile()
{
  LOGI("ImsCallSession::getCallProfile()");
  ImsCallProfile *ret = new ImsCallProfile();
  if(nullptr == ret) {
    LOGI("ImsCallSession::getCallProfile()::ImsCallProfile is null");
    return nullptr;
  }
  android::Parcel ims_parcel;
  android::Parcel ims_out_parcel;
  ims_parcel.writeInt32(IImsCallSession::getCallProfile);
  if(pXPCOMCoreInterfaceObj != nullptr) {
    pXPCOMCoreInterfaceObj->XpcomCore_sendData(m_nSessionID, ims_parcel, ims_out_parcel);
  } else {
    LOGE("ImsCallSession::getCallProfile()::pXPCOMCoreInterfaceObj is null");
  }
  ims_out_parcel.setDataPosition(0);
  ret->readFromParcel(ims_out_parcel);
  return ret;
}

ImsCallProfile *ImsCallSession::getLocalCallProfile()
{
  LOGI("ImsCallSession::getLocalCallProfile()");
  ImsCallProfile *ret = new ImsCallProfile();
  if(nullptr != ret) {
    LOGI("ImsCallSession::getLocalCallProfile()::ImsCallProfile is null");
    return nullptr;
  }
  android::Parcel ims_parcel;
  android::Parcel ims_out_parcel;
  ims_parcel.writeInt32(IImsCallSession::getLocalCallProfile);
  if(pXPCOMCoreInterfaceObj != nullptr) {
    pXPCOMCoreInterfaceObj->XpcomCore_sendData(m_nSessionID, ims_parcel, ims_out_parcel);
  } else {
    LOGE("ImsCallSession::getLocalCallProfile()::pXPCOMCoreInterfaceObj is null");
  }
  ims_out_parcel.setDataPosition(0);
  ret->readFromParcel(ims_out_parcel);
  return ret;
}

ImsCallProfile *ImsCallSession::getRemoteCallProfile()
{
  LOGI("ImsCallSession::getRemoteCallProfile()");
  ImsCallProfile *ret = new ImsCallProfile();
  if(nullptr != ret) {
    LOGI("ImsCallSession::getRemoteCallProfile()::ImsCallProfile is null");
    return nullptr;
  }
  android::Parcel ims_parcel;
  android::Parcel ims_out_parcel;
  ims_parcel.writeInt32(IImsCallSession::getRemoteCallProfile);
  if(pXPCOMCoreInterfaceObj != nullptr) {
    pXPCOMCoreInterfaceObj->XpcomCore_sendData(m_nSessionID, ims_parcel, ims_out_parcel);
  } else {
    LOGE("ImsCallSession::getRemoteCallProfile()::pXPCOMCoreInterfaceObj is null");
  }
  ims_out_parcel.setDataPosition(0);
  ret->readFromParcel(ims_out_parcel);
  return ret;
}

#if 0
IImsVideoCallProvider *ImsCallSession::getVideoCallProvider()
{
  return nullptr;
}
#endif

int32_t ImsCallSession::getCallSubstate()
{
  LOGI("ImsCallSession::getCallSubstate()");
  android::Parcel ims_parcel;
  android::Parcel ims_out_parcel;
  ims_parcel.writeInt32(IImsCallSession::getCallSubstate);
  if(pXPCOMCoreInterfaceObj != nullptr) {
    pXPCOMCoreInterfaceObj->XpcomCore_sendData(m_nSessionID, ims_parcel, ims_out_parcel);
  } else {
    LOGE("ImsCallSession::getCallSubstate()::pXPCOMCoreInterfaceObj is null");
  }
  ims_out_parcel.setDataPosition(0);
  return ims_out_parcel.readInt32();
}

/*



*/
nsString ImsCallSession::getProperty(const nsString& name)
{
  nsString ret;
  android::Parcel ims_parcel;
  android::Parcel ims_out_parcel;
  NS_LossyConvertUTF16toASCII convertStr(name);
  LOGI("ImsCallSession::getProperty:: %s", convertStr.get());
  ims_parcel.writeInt32(IImsCallSession::getProperty);
  ims_parcel.writeString16(android::String16(convertStr.get()));
  if(pXPCOMCoreInterfaceObj != nullptr) {
    pXPCOMCoreInterfaceObj->XpcomCore_sendData(m_nSessionID, ims_parcel, ims_out_parcel);
  } else {
    LOGE("ImsCallSession::getProperty()::pXPCOMCoreInterfaceObj is null");
  }
  ims_out_parcel.setDataPosition(0);
  ret = ims_out_parcel.readString16();
  return ret;
}

int32_t ImsCallSession::getState()
{
  LOGI("ImsCallSession::getState()");
  android::Parcel ims_parcel;
  android::Parcel ims_out_parcel;
  ims_parcel.writeInt32(IImsCallSession::getState);
  if(pXPCOMCoreInterfaceObj != nullptr) {
    pXPCOMCoreInterfaceObj->XpcomCore_sendData(m_nSessionID, ims_parcel, ims_out_parcel);
  } else {
    LOGE("ImsCallSession::getState()::pXPCOMCoreInterfaceObj is null");
  }
  ims_out_parcel.setDataPosition(0);
  return ims_out_parcel.readInt32();
}

bool ImsCallSession::isAlive()
{
  LOGI("ImsCallSession::isAlive()");
  int32_t state = getState();
  switch (state) {
    case State::IDLE:
    case State::INITIATED:
    case State::NEGOTIATING:
    case State::ESTABLISHING:
    case State::ESTABLISHED:
    case State::RENEGOTIATING:
    case State::REESTABLISHING:
      return true;
    default:
      isSessionAlive = false;
      return false;
  }
}

#if 0
IImsCallSession *ImsCallSession::getSession()
{
  return miSession;
}
#endif

bool ImsCallSession::isInCall()
{
  LOGI("ImsCallSession::isInCall()");
  android::Parcel ims_parcel;
  android::Parcel ims_out_parcel;
  ims_parcel.writeInt32(IImsCallSession::isInCall);
  if(pXPCOMCoreInterfaceObj != nullptr) {
    pXPCOMCoreInterfaceObj->XpcomCore_sendData(m_nSessionID, ims_parcel, ims_out_parcel);
  } else {
    LOGE("ImsCallSession::isInCall()::pXPCOMCoreInterfaceObj is null");
  }
  ims_out_parcel.setDataPosition(0);
  if(ims_out_parcel.readInt32()) {
    return true;
  } else {
    return false;
  }
}

void ImsCallSession::setListener(ImsCall *listener)
{
    pImsCall = listener;
}

void ImsCallSession::setMute(bool muted)
{
  LOGI("ImsCallSession::setMute()");
  android::Parcel ims_parcel;
  ims_parcel.writeInt32(IImsCallSession::setMute);
  if(muted) {
    ims_parcel.writeInt32(1);
  } else {
    ims_parcel.writeInt32(0);
  }
  if(pXPCOMCoreInterfaceObj != nullptr) {
    pXPCOMCoreInterfaceObj->XpcomCore_sendData(m_nSessionID, ims_parcel);
  } else {
    LOGE("ImsCallSession::setMute()::pXPCOMCoreInterfaceObj is null");
  }
}

void ImsCallSession::start(const nsString& callee, ImsCallProfile *profile)
{
  LOGI("ImsCallSession::start() - %ld", m_nSessionID);
  android::Parcel ims_parcel;
  NS_LossyConvertUTF16toASCII convertStr(callee);
  LOGE("ImsCallSession::start :dial %s", convertStr.get());

  ims_parcel.writeInt32(IImsCallSession::start);
  ims_parcel.writeString16(android::String16(convertStr.get()));
  profile->writeToParcel(ims_parcel, 0);
  LOGE("ImsCallSession::start :media::mAudioQuality - %d, mAudioDirection - %d, mVideoQuality - %d, mVideoDirection - %d", profile->mMediaProfile->mAudioQuality,
                                    profile->mMediaProfile->mAudioDirection, profile->mMediaProfile->mVideoQuality, profile->mMediaProfile->mVideoDirection);
  if(pXPCOMCoreInterfaceObj != nullptr) {
    pXPCOMCoreInterfaceObj->XpcomCore_sendData(m_nSessionID, ims_parcel);
  } else {
    LOGE("ImsCallSession::start()::pXPCOMCoreInterfaceObj is null");
  }
}

void start(nsTArray<nsString> &participants, ImsCallProfile *profile)
{
  LOGI("ImsCallSession::start() for conference::To-Do");
}


void ImsCallSession::accept(int32_t callType, ImsStreamMediaProfile *profile)
{
  LOGI("ImsCallSession::accept()");
  android::Parcel ims_parcel;
  ims_parcel.writeInt32(IImsCallSession::accept);
  //
  ims_parcel.writeInt32(nsIImsCallProfile::CALL_TYPE_VOICE);
  profile->writeToParcel(ims_parcel, 0);
  if(pXPCOMCoreInterfaceObj != nullptr) {
    pXPCOMCoreInterfaceObj->XpcomCore_sendData(m_nSessionID, ims_parcel);
  } else {
    LOGE("ImsCallSession::accept()::pXPCOMCoreInterfaceObj is null");
  }
}

void ImsCallSession::deflect(const nsString& number)
{
  LOGI("ImsCallSession::deflect()");
  NS_LossyConvertUTF16toASCII convertStr(number);
  LOGE("ImsCallSession::start :dial %s", convertStr.get());
  android::Parcel ims_parcel;
  ims_parcel.writeInt32(IImsCallSession::deflect);
  ims_parcel.writeString16(android::String16(convertStr.get()));
  if(pXPCOMCoreInterfaceObj != nullptr) {
    pXPCOMCoreInterfaceObj->XpcomCore_sendData(m_nSessionID, ims_parcel);
  } else {
    LOGE("ImsCallSession::deflect()::pXPCOMCoreInterfaceObj is null");
  }
}

void ImsCallSession::reject(int32_t reason)
{
  LOGI("ImsCallSession::reject()");
  android::Parcel ims_parcel;
  ims_parcel.writeInt32(IImsCallSession::reject);
  ims_parcel.writeInt32(reason);
  if(pXPCOMCoreInterfaceObj != nullptr) {
    pXPCOMCoreInterfaceObj->XpcomCore_sendData(m_nSessionID, ims_parcel);
  } else {
    LOGE("ImsCallSession::reject()::pXPCOMCoreInterfaceObj is null");
  }
}

void ImsCallSession::terminate(int32_t reason)
{
  LOGI("ImsCallSession::terminate() session - %d, m_nSessionID - %ld", this, m_nSessionID);
  android::Parcel ims_parcel;
  ims_parcel.writeInt32(IImsCallSession::terminate);
  ims_parcel.writeInt32(reason);
  if(pXPCOMCoreInterfaceObj != nullptr) {
    pXPCOMCoreInterfaceObj->XpcomCore_sendData(m_nSessionID, ims_parcel);
  } else {
    LOGE("ImsCallSession::terminate()::pXPCOMCoreInterfaceObj is null");
  }
}

void ImsCallSession::hold(ImsStreamMediaProfile *profile)
{
  LOGI("ImsCallSession::hold() session - %d, m_nSessionID - %ld", this, m_nSessionID);
  android::Parcel ims_parcel;
  ims_parcel.writeInt32(IImsCallSession::hold);
  profile->writeToParcel(ims_parcel, 0);
  if(pXPCOMCoreInterfaceObj != nullptr) {
    pXPCOMCoreInterfaceObj->XpcomCore_sendData(m_nSessionID, ims_parcel);
  } else {
    LOGE("ImsCallSession::hold()::pXPCOMCoreInterfaceObj is null");
  }
}

void ImsCallSession::resume(ImsStreamMediaProfile *profile)
{
  LOGI("ImsCallSession::resume():: session - %d, m_nSessionID - %ld", this, m_nSessionID);
  android::Parcel ims_parcel;
  ims_parcel.writeInt32(IImsCallSession::resume);
  profile->writeToParcel(ims_parcel, 0);
  if(pXPCOMCoreInterfaceObj != nullptr) {
    pXPCOMCoreInterfaceObj->XpcomCore_sendData(m_nSessionID, ims_parcel);
  } else {
    LOGE("ImsCallSession::resume()::pXPCOMCoreInterfaceObj is null");
  }
}

void ImsCallSession::merge()
{
  LOGI("ImsCallSession::merge():: session - %d, m_nSessionID - %ld", this, m_nSessionID);
  android::Parcel ims_parcel;
  ims_parcel.writeInt32(IImsCallSession::merge);
  if(pXPCOMCoreInterfaceObj != nullptr) {
    pXPCOMCoreInterfaceObj->XpcomCore_sendData(m_nSessionID, ims_parcel);
  } else {
    LOGE("ImsCallSession::merge()::pXPCOMCoreInterfaceObj is null");
  }
}

void ImsCallSession::update(int32_t callType, ImsStreamMediaProfile *profile)
{
  LOGI("ImsCallSession::update()");
  android::Parcel ims_parcel;
  ims_parcel.writeInt32(IImsCallSession::update);
  ims_parcel.writeInt32(callType);
  profile->writeToParcel(ims_parcel, 0);
  if(pXPCOMCoreInterfaceObj != nullptr) {
    pXPCOMCoreInterfaceObj->XpcomCore_sendData(m_nSessionID, ims_parcel);
  } else {
    LOGE("ImsCallSession::update()::pXPCOMCoreInterfaceObj is null");
  }
}

void ImsCallSession::extendToConference(nsTArray<nsString> &participants)
{
  LOGI("ImsCallSession::extendToConference()::To-Do");
}

void ImsCallSession::inviteParticipants(nsTArray<nsString> &participants)
{
  LOGI("ImsCallSession::inviteParticipants()::To-Do");
}

void ImsCallSession::removeParticipants(nsTArray<nsString> &participants)
{
  LOGI("ImsCallSession::removeParticipants()::session - %d, m_nSessionID - %ld", this, m_nSessionID);
  android::Parcel ims_parcel;
  ims_parcel.writeInt32(IImsCallSession::removeParticipants);
  int32_t count = participants.Length();
  LOGI("ImsCallSession::removeParticipants()::count - %d", count);
  ims_parcel.writeInt32(count);
  for(int index = 0; index < count; index++){
    nsString temp = participants[index];
    NS_LossyConvertUTF16toASCII convertTemp(temp);
    LOGI("ImsCallSession::removeParticipants()::temp - %s", convertTemp.get());
    ims_parcel.writeString16(android::String16(convertTemp.get()));
  }
  if(pXPCOMCoreInterfaceObj != nullptr) {
    pXPCOMCoreInterfaceObj->XpcomCore_sendData(m_nSessionID, ims_parcel);
  } else {
    LOGE("ImsCallSession::update()::pXPCOMCoreInterfaceObj is null");
  }
}

/*





 */
void ImsCallSession::SendDtmf(const nsAString & c)
{
  LOGI("ImsCallSession::SendDtmf()");
  nsString tempStr;
  tempStr.Assign(c);
  NS_LossyConvertUTF16toASCII convertStr(tempStr);
  android::Parcel ims_parcel;
  ims_parcel.writeInt32(IImsCallSession::sendDtmf);
  ims_parcel.writeString16(android::String16(convertStr.get()));
  ims_parcel.writeInt32(20);
  if(pXPCOMCoreInterfaceObj != nullptr) {
    pXPCOMCoreInterfaceObj->XpcomCore_sendData(m_nSessionID, ims_parcel);
  } else {
    LOGE("ImsCallSession::SendDtmf()::pXPCOMCoreInterfaceObj is null");
  }
}

/*





 */
void ImsCallSession::startDtmf(char *c)
{
  LOGI("ImsCallSession::startDtmf()::To-Do");
#if 0
  nsString tempStr;
  tempStr.Assign(c);
  NS_LossyConvertUTF16toASCII convertStr(tempStr);
  android::Parcel ims_parcel;
  ims_parcel.writeInt32(IImsCallSession::startDtmf);
  ims_parcel.writeString16(android::String16(convertStr.get()));
  ims_parcel.writeInt32(20);
  if(pXPCOMCoreInterfaceObj != nullptr) {
    pXPCOMCoreInterfaceObj->XpcomCore_sendData(m_nSessionID, ims_parcel);
  } else {
    LOGE("ImsCallSession::startDtmf()::pXPCOMCoreInterfaceObj is null");
  }
#endif
}

void ImsCallSession::stopDtmf()
{
  LOGI("ImsCallSession::stopDtmf()");
  android::Parcel ims_parcel;
  ims_parcel.writeInt32(IImsCallSession::stopDtmf);
  if(pXPCOMCoreInterfaceObj != nullptr) {
    pXPCOMCoreInterfaceObj->XpcomCore_sendData(m_nSessionID, ims_parcel);
  } else {
    LOGE("ImsCallSession::stopDtmf()::pXPCOMCoreInterfaceObj is null");
  }
}

void ImsCallSession::sendUssd(const nsAString& ussdMessage)
{
  LOGI("ImsCallSession::sendUssd()");
  NS_LossyConvertUTF16toASCII convertStr(ussdMessage);
  android::Parcel ims_parcel;
  ims_parcel.writeInt32(IImsCallSession::sendUssd);
  ims_parcel.writeString16(android::String16(convertStr.get()));
  if(pXPCOMCoreInterfaceObj != nullptr) {
    pXPCOMCoreInterfaceObj->XpcomCore_sendData(m_nSessionID, ims_parcel);
  } else {
    LOGE("ImsCallSession::sendUssd()::pXPCOMCoreInterfaceObj is null");
  }
}

bool ImsCallSession::isMultiparty()
{
  LOGI("ImsCallSession::isMultiparty()::session - %d, m_nSessionID - %ld", this, m_nSessionID);
  android::Parcel ims_parcel;
  android::Parcel ims_out_parcel;
  ims_parcel.writeInt32(IImsCallSession::isMultiparty);
  if(pXPCOMCoreInterfaceObj != nullptr) {
    pXPCOMCoreInterfaceObj->XpcomCore_sendData(m_nSessionID, ims_parcel, ims_out_parcel);
  } else {
    LOGE("ImsCallSession::isMultiparty()::pXPCOMCoreInterfaceObj is null");
  }
  ims_out_parcel.setDataPosition(0);
  bool ret = ims_out_parcel.readInt32();
  LOGI("ImsCallSession::isMultiparty() - %d", ret);
  return ret;
}

//
/*

 */
void ImsCallSession::explicitCallTransfer()
{
  LOGI("ImsCallSession::explicitCallTransfer()::To-Do");
}

#if 0
IImsCallSessionEx *ImsCallSession::getCallSessionEx()
{

}
#endif

class IMSToB2gRunnable: public nsRunnable {
public:
  IMSToB2gRunnable(ImsCallSession* session, const android::Parcel& parcel) :
    mSession(session) {
    mParcel.appendFrom(&parcel, (size_t)0, parcel.dataSize());
  }

  NS_IMETHOD Run() {
    if (mSession)
      mSession->onMessageInternal(mParcel);
    return NS_OK;
  }
private:
  ImsCallSession* mSession;
  android::Parcel mParcel;
};

/*




 */

/*                                                                                    */
void ImsCallSession::onMessage(const android::Parcel& pParcel)
{
  LOGE("ImsCallSession::onMessage()");
  nsCOMPtr<nsIRunnable> proc = new IMSToB2gRunnable(this, pParcel);
  if (NS_FAILED(NS_DispatchToMainThread(proc))) {
    LOGE("ImsCallSession::onMessage error");
    return;
  }
}

void ImsCallSession::onMessageEx(const android::Parcel &pParcel, Parcel *pOutParcel)
{
    return;
}

void ImsCallSession::registrationConnected()
{
    return;
}

void ImsCallSession::registrationDisconnected(const android::Parcel &pParcel)
{
    return;
}

void ImsCallSession::notifyIncomingCall(int phoneID, int serviceID, const char* callId)
{
    return;
}

void ImsCallSession::enteredECBM()
{
    return;
}

void ImsCallSession::exitedECBM()
{
    return;
}

void ImsCallSession::set911State(int32_t state, int32_t ecbm)
{
    return;
}

void ImsCallSession::onMessageInternal(const android::Parcel& pParcel)
{
  LOGE("ImsCallSession::onMessageInternal()");

  pParcel.setDataPosition(0);
  int msg = pParcel.readInt32();
  LOGE("ImsCallSession::onMessageInternal()::msg - %d", msg);
  switch (msg){
    case IImsCallSessionListener ::callSessionProgressing:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionProgressing");
      if(pMediaProfile)
        delete pMediaProfile;
      pMediaProfile = new ImsStreamMediaProfile(pParcel);
      if(NULL != pImsCall){
        pImsCall->callSessionProgressing(this, pMediaProfile);
      }
    }
    break;
    case IImsCallSessionListener::callSessionStarted:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionStarted");
      if(profile)
        delete profile;
      profile = new ImsCallProfile(pParcel);
      if(NULL != pImsCall){
        pImsCall->callSessionStarted(this, profile);
      }
    }
    break;
    case IImsCallSessionListener::callSessionStartFailed:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionStartFailed");
      if(reasonInfo)
        delete reasonInfo;
      reasonInfo = new ImsReasonInfo(pParcel);
      if(NULL != pImsCall){
        pImsCall->callSessionStartFailed(this, reasonInfo);
      }
    }
    break;
    case IImsCallSessionListener::callSessionTerminated:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionTerminated");
      if(reasonInfo)
        delete reasonInfo;
      reasonInfo = new ImsReasonInfo(pParcel);
      if(NULL != pImsCall){
        pImsCall->callSessionTerminated(this, reasonInfo);
      }
    }
    break;
    case IImsCallSessionListener::callSessionHeld:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionHeld");
      if(profile)
        delete profile;
      profile = new ImsCallProfile(pParcel);
      if(NULL != pImsCall){
        pImsCall->callSessionHeld(this, profile);
      }
    }
    break;
    case IImsCallSessionListener::callSessionHoldFailed:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionHoldFailed");
      if(reasonInfo)
        delete reasonInfo;
      reasonInfo = new ImsReasonInfo(pParcel);
      if(NULL != pImsCall){
        pImsCall->callSessionHoldFailed(this, reasonInfo);
      }
    }
    break;
    case IImsCallSessionListener::callSessionHoldReceived:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionHoldReceived");
      if(profile)
        delete profile;
      profile = new ImsCallProfile(pParcel);
      if(NULL != pImsCall){
        pImsCall->callSessionHoldReceived(this, profile);
      }
    }
    break;
    case IImsCallSessionListener::callSessionResumed:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionResumed");
      if(profile)
        delete profile;
      profile = new ImsCallProfile(pParcel);
      if(NULL != pImsCall){
        pImsCall->callSessionResumed(this, profile);
      }
    }
    break;
    case IImsCallSessionListener::callSessionResumeFailed:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionResumeFailed");
      if(reasonInfo)
        delete reasonInfo;
      reasonInfo = new ImsReasonInfo(pParcel);
      if(NULL != pImsCall){
        pImsCall->callSessionResumeFailed(this, reasonInfo);
      }
    }
    break;
    case IImsCallSessionListener::callSessionResumeReceived:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionResumeReceived");
      if(profile)
        delete profile;
      profile = new ImsCallProfile(pParcel);
      if(NULL != pImsCall){
        pImsCall->callSessionResumeReceived(this, profile);
      }
    }
    break;
    case IImsCallSessionListener::callSessionMergeStarted:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionMergeStarted");
      int64_t sessionId = pParcel.readInt64();
      if(profile)
        delete profile;
      profile = new ImsCallProfile(pParcel);
      ImsCallSession *newSession = new ImsCallSession(sessionId);
      if(NULL != pImsCall){
        pImsCall->callSessionMergeStarted(this, newSession, profile);
        newSession = nullptr;
      }
    }
    break;
    case IImsCallSessionListener::callSessionMergeComplete:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionMergeComplete");
      if(NULL != pImsCall){
        pImsCall->callSessionMergeComplete(this);
      }
      LOGI("ImsCallSession::onMessageInternal::callSessionMergeComplete::end");
    }
    break;
    case IImsCallSessionListener::callSessionMergeFailed:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionMergeFailed");
      if(reasonInfo)
        delete reasonInfo;
      reasonInfo = new ImsReasonInfo(pParcel);
      if(NULL != pImsCall){
        pImsCall->callSessionMergeFailed(this, reasonInfo);
      }
    }
    break;
    case IImsCallSessionListener::callSessionUpdated:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionUpdated");
      if(profile)
        delete profile;
      profile = new ImsCallProfile(pParcel);
      if(NULL != pImsCall){
        pImsCall->callSessionUpdated(this, profile);
      }
    }
    break;
    case IImsCallSessionListener::callSessionUpdateFailed:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionUpdateFailed");
      if(reasonInfo)
        delete reasonInfo;
      reasonInfo = new ImsReasonInfo(pParcel);
      if(NULL != pImsCall){
        pImsCall->callSessionUpdateFailed(this, reasonInfo);
      }
    }
    break;
    case IImsCallSessionListener::callSessionUpdateReceived:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionUpdateReceived");
      if(profile)
        delete profile;
      profile = new ImsCallProfile(pParcel);
      if(NULL != pImsCall){
        pImsCall->callSessionUpdateReceived(this, profile);
      }
    }
    break;
    case IImsCallSessionListener::callSessionConferenceExtended:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionConferenceExtended::To-Do");
    }
    break;
    case IImsCallSessionListener::callSessionConferenceExtendFailed:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionConferenceExtendFailed::To-Do");
    }
    break;
    case IImsCallSessionListener::callSessionConferenceExtendReceived:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionConferenceExtendReceived::To-Do");
    }
    break;
    case IImsCallSessionListener::callSessionInviteParticipantsRequestDelivered:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionInviteParticipantsRequestDelivered");
      if(NULL != pImsCall){
        pImsCall->callSessionInviteParticipantsRequestDelivered(this);
      }
    }
    break;
    case IImsCallSessionListener::callSessionInviteParticipantsRequestFailed:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionInviteParticipantsRequestFailed");
      if(reasonInfo)
        delete reasonInfo;
      reasonInfo = new ImsReasonInfo(pParcel);
      if(NULL != pImsCall){
        pImsCall->callSessionInviteParticipantsRequestFailed(this, reasonInfo);
      }
    }
    break;
    case IImsCallSessionListener::callSessionRemoveParticipantsRequestDelivered:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionRemoveParticipantsRequestDelivered");
      if(NULL != pImsCall){
        pImsCall->callSessionRemoveParticipantsRequestDelivered(this);
      }
    }
    break;
    case IImsCallSessionListener::callSessionRemoveParticipantsRequestFailed:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionRemoveParticipantsRequestFailed");
      if(reasonInfo)
        delete reasonInfo;
      reasonInfo = new ImsReasonInfo(pParcel);
      if(NULL != pImsCall){
        pImsCall->callSessionRemoveParticipantsRequestFailed(this, reasonInfo);
      }
    }
    break;
    case IImsCallSessionListener::callSessionConferenceStateUpdated:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionConferenceStateUpdated");
      mstate = new ImsConferenceState(pParcel);
      if(NULL != pImsCall){
        pImsCall->callSessionConferenceStateUpdated(this, mstate);
      }
    }
    break;
    case IImsCallSessionListener::callSessionUssdMessageReceived:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionUssdMessageReceived::To-Do");
    }
    break;
    case IImsCallSessionListener::callSessionHandoverFailed:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionHandoverFailed::To-Do");
    }
    break;
    case IImsCallSessionListener::callSessionTtyModeReceived:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionTtyModeReceived::To-Do");
    }
    break;
    case IImsCallSessionListener::callSessionDeflected:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionDeflected::To-Do");
    }
    break;
    case IImsCallSessionListener::callSessionDeflectFailed:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionDeflectFailed::To-Do");
    }
    break;
    case IImsCallSessionListener::callSessionSuppServiceReceived:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionSuppServiceReceived::To-Do");
    }
    break;
    case IImsCallSessionListener::callSessionRetryErrorReceived:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionRetryErrorReceived::To-Do");
    }
    break;
    case IImsCallSessionListener::callSessionExplicitCallTransferred:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionExplicitCallTransferred::To-Do");
    }
    break;
    case IImsCallSessionListener::callSessionExplicitCallTransferFailed:
    {
      LOGI("ImsCallSession::onMessageInternal::callSessionExplicitCallTransferFailed::To-Do");
    }
    break;
    default:
    {
      LOGE("ImsCallSession::onMessage::default");
    }
  }

}

void ImsCallSession::binderDied(const wp<IBinder>&) {
    LOGE("binder death detected");
}
