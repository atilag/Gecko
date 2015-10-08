#undef LOG_TAG
#define LOG_TAG "LGE_IMS_CALL"
#include <utils/Log.h>
#include "nsIRunnable.h"
#include "ImsCall.h"
#include "nsString.h"
#include "nsILgeImsUCManager.h"

#define LOGI(args...) \
     __android_log_print(ANDROID_LOG_INFO, LOG_TAG, ## args)

#define LOGE(args...) \
     __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, ## args)

NS_IMPL_ISUPPORTS(ImsCall, nsIImsCall)
using namespace android ;


class IMSCallToB2gRunnable: public nsRunnable {
public:
  IMSCallToB2gRunnable(ImsCallSession* session){
    if (session != nullptr) {
      callSession = session;
    }
    bWhichcase = 2;
  }
  ~IMSCallToB2gRunnable(){}
  IMSCallToB2gRunnable(ImsCall* session, ImsReasonInfo* reasonInfo) :
    call(session) {
    mreasonInfo = reasonInfo;
    bWhichcase = 1;
  }

  NS_IMETHOD Run() {
      switch(bWhichcase){
      case 1:
        if (call)
          call->SendOnCallTermate(mreasonInfo);
        return NS_OK;
      case 2:
        if (callSession != nullptr) {
          callSession->close();
        }
        return NS_OK ;
      default:
        break;
    }
  }
private:
  ImsCall* call;
  ImsCallSession* callSession;
  ImsReasonInfo* mreasonInfo;
  int bWhichcase;
};

ImsCall::ImsCall(ImsCallProfile *profile)
{
  LOGI("ImsCall::ImsCall");
  mSession = new ImsCallSession();
  if(mSession != nullptr) {
    mSession->setListener(this);
  }
  LOGI("ImsCall::ImsCall::mSession - %d", mSession);
  m_pIAUCSessionListener = nullptr;
  if(profile != nullptr) {
    if(mCallProfile)
      delete mCallProfile;
    mCallProfile = new ImsCallProfile();
    mCallProfile->copyFrom(profile);
  }
  mImsCall = this;
  ConferenceHost = nullptr;
  mTransientConferenceSession = nullptr;
  mMergeHost = nullptr;
}

ImsCall::~ImsCall()
{
  LOGI("ImsCall::~ImsCall");

  if(mCallProfile) {
    LOGI("ImsCall::~ImsCall::mCallProfile");
    delete mCallProfile;
    mCallProfile = nullptr;
  }
  if(mProposedCallProfile) {
    LOGI("ImsCall::~ImsCall::mProposedCallProfile");
    delete mProposedCallProfile;
    mProposedCallProfile = nullptr;
  }
  if(mLastReasonInfo) {
    LOGI("ImsCall::~ImsCall::mLastReasonInfo");
    delete mLastReasonInfo;
    mLastReasonInfo = nullptr;
  }
  if(mTransientConferenceSession) {
    LOGI("ImsCall::~ImsCall::mTransientConferenceSession");
    delete mTransientConferenceSession;
    mTransientConferenceSession = nullptr;
  }
  if(mSessionEndDuringMergeReasonInfo) {
    LOGI("ImsCall::~ImsCall::mSessionEndDuringMergeReasonInfo");
    //
    mSessionEndDuringMergeReasonInfo = nullptr;
  }
  ConferenceHost = nullptr;
  mMergePeer = nullptr;
  mMergeHost = nullptr;
  mImsCall = nullptr;
  mSession = nullptr;
}

void ImsCall::updateHoldValues()
{
  mHold = true;
}

/*

 */
NS_IMETHODIMP
ImsCall::Close()
{
  LOGI("ImsCall::Close");
  mSession->close();
  mSession = nullptr;
  mImsCall = nullptr;
  delete this;
  return NS_OK;
}

void ImsCall::close()
{
  LOGI("ImsCall::close");
  mSession->close();
  mSession = nullptr;
  delete this;
}
/*




*/
bool ImsCall::checkIfRemoteUserIsSame(const nsString& userId)
{
  LOGI("ImsCall::checkIfRemoteUserIsSame");
  nsString empty;
  return userId.Equals(mCallProfile->getCallExtra(ImsCallProfile::EXTRA_REMOTE_URI, empty));
}

/*




 */
bool ImsCall::equalsTo(ICall *call)
{
  LOGI("ImsCall::equalsTo");

  if (call == nullptr) {
    return false;
  }

  return (call == this);
}

bool ImsCall::isSessionAlive(ImsCallSession *session)
{
  if(session == nullptr){
    LOGI("ImsCall::isSessionAlive - session is null");
  }
  if(session->isSessionAlive == false){
    return false;
  }
  bool ret = session != nullptr && session->isAlive();
  LOGI("ImsCall::isSessionAlive - %d", ret);
  return ret;
}

/*



*/
ImsCallProfile *ImsCall::getCallProfile()
{
  return mCallProfile;
}

/*



*/
ImsCallProfile *ImsCall::getLocalCallProfile()
{
  if (mSession == nullptr) {
    LOGE("ImsCall::getLocalCallProfile::mSession is null");
  }
  return mSession->getLocalCallProfile();
}

/*



 */
ImsCallProfile *ImsCall::getRemoteCallProfile()
{
  if (mSession == nullptr) {
    LOGE("ImsCall::getRemoteCallProfile::mSession is null");
  }
  return mSession->getRemoteCallProfile();
}

/*



 */
ImsCallProfile *ImsCall::getProposedCallProfile()
{
  if (!isInCall()) {
    return nullptr;
  }
  return mProposedCallProfile;
}

/*




*/

nsTArray<ConferenceParticipant *> &ImsCall::getConferenceParticipants()
{
  LOGI("ImsCall::getConferenceParticipants");
  return mConferenceParticipants;
}

/*




*/
NS_IMETHODIMP
ImsCall::GetState(int32_t *aState)
{
  *aState = mSession->getState();
  return NS_OK;
}

/*




*/
ImsCallSession *ImsCall::getCallSession()
{
  return mSession;
}

/*





*/
#if 0
ImsStreamMediaSession *ImsCall::getMediaSession()
{
  return mMediaSession;
}
#endif
//
/*





*/
ImsConferenceState *ImsCall::getConferenceState()
{
  return mConfState;
}

//

/*




*/
nsString ImsCall::getCallExtra(const nsString& name)
{
  nsString empty;
  //
  if (mSession == nullptr) {
    LOGE("ImsCall::setListener::getCallExtra is null");
    return empty;
  }
  return mSession->getProperty(name);
}

/*



*/
int32_t ImsCall::getCallSubstate()
{
  if (mSession == nullptr) {
    LOGE("ImsCall::getCallSubstate::getCallExtra is null");
    return -1;
  }
  return mSession->getCallSubstate();
}

/*



*/
ImsReasonInfo *ImsCall::getLastReasonInfo()
{
  return mLastReasonInfo;
}

/*



*/
bool ImsCall::hasPendingUpdate()
{
  return (mUpdateRequest != UPDATE_NONE);
}

/*



*/
bool ImsCall::isInCall()
{
  return mInCall;
}

NS_IMETHODIMP
ImsCall::IsInCall(bool *aInCall)
{
  LOGE("ImsCall::isInCall - %d", mInCall);
  *aInCall = this->mInCall;
  return NS_OK;
}

/*



*/
bool ImsCall::isMuted()
{
  LOGI("ImsCall::isMuted - %d", mMute);
  return mMute;
}

/*



*/
bool ImsCall::isOnHold()
{
  LOGI("ImsCall::isOnHold - %d", mHold);
  return mHold;
}

/*



*/
bool ImsCall::isMultiparty()
{
  LOGI("ImsCall::isMultiparty::session - %d, ImsCallSession - %d", this, mSession);
  if (mSession == nullptr) {
    LOGE("ImsCall::isMultiparty::mSession is null");
    return false;
  }
  return mSession->isMultiparty();
}

/*




*/
void ImsCall::setIsMerged(bool isMerged)
{
  LOGI("ImsCall::isMerged - %d", isMerged);
  mIsMerged = isMerged;
}

/*

*/
bool ImsCall::isMerged()
{
  LOGI("ImsCall::isMerged - %d", mIsMerged);
  return mIsMerged;
}

void ImsCall::setListener(nsIImsCallListener *pIAUCSessionListener)
{
  LOGI("ImsCall::setListener");
  if(pIAUCSessionListener != nullptr) {
    m_pIAUCSessionListener = pIAUCSessionListener;
  } else {
    LOGE("ImsCall::setListener::pIAUCSessionListener is null");
  }
}

/*



*/
void ImsCall::setMute(bool muted)
{
  LOGI("ImsCall::setMute");
  if (mMute != muted) {
    mMute = muted;
    mSession->setMute(muted);
  }
}

/*




*/
void ImsCall::attachSession(ImsCallSession *session)
{
  LOGI("ImsCall::attachSession");
  mSession = session;
  mSession->setListener(createCallSessionListener());
}

/*






*/
void ImsCall::start(ImsCallSession *session, const nsString& callee)
{
  LOGI("ImsCall::start");
  mSession = session;
  session->setListener(createCallSessionListener());
  session->start(callee, mCallProfile);
  sCalleePartyNum = callee;
}

/*






*/
void ImsCall::start(ImsCallSession *session, nsTArray<nsString> &participants)
{
  LOGI("ImsCall::start:: for conference");
  mSession = session;
  session->setListener(createCallSessionListener());
  session->start(participants, mCallProfile);
}

/*






 */
NS_IMETHODIMP
ImsCall::Accept(int32_t callType)
{
  LOGI("ImsCall::accept");
  accept(callType, new ImsStreamMediaProfile());
  return NS_OK;
}

/*






*/
void ImsCall::accept(int32_t callType, ImsStreamMediaProfile *profile)
{
  if (mSession == nullptr) {
    LOGE("ImsCall::accept::mSession is null");
  }
  mSession->accept(callType, profile);

  if (mInCall && (mProposedCallProfile != nullptr)) {
    LOGI("ImsCall::accept::call profile will be updated");
    if(mCallProfile)
      delete mCallProfile;
    mCallProfile = new ImsCallProfile();
    mCallProfile->copyFrom(mProposedCallProfile);
  }

  //
  if (mInCall && (mUpdateRequest == UPDATE_UNSPECIFIED)) {
    mUpdateRequest = UPDATE_NONE;
  }
}

/*




*/
void ImsCall::deflect(const nsString& number)
{
  LOGI("ImsCall::deflect");
  if (mSession == nullptr) {
    LOGE("ImsCall::deflect::mSession is null");
  }
  mSession->deflect(number);
}

/*





 */
NS_IMETHODIMP
ImsCall::Reject(int32_t reason)
{
  LOGI("ImsCall::reject reason - %d", reason);
  if (mSession != nullptr) {
    mSession->reject(reason);
  }
  if (mInCall && (mProposedCallProfile != nullptr)) {
    mProposedCallProfile = nullptr;
  }

  //
  if (mInCall && (mUpdateRequest == UPDATE_UNSPECIFIED)) {
    mUpdateRequest = UPDATE_NONE;
  }
  return NS_OK;
}

/*




*/
NS_IMETHODIMP
ImsCall::Terminate(int32_t reason)
{
  LOGI("ImsCall::terminate::session - %d, ImsCallSession - %d, reason - %d", this, mSession, reason);
  mHold = false;
  mInCall = false;
  if (mSession != nullptr) {
    mSession->terminate(reason);
  }
  return NS_OK;
}

/*




*/
NS_IMETHODIMP
ImsCall::Hold()
{
  LOGI("ImsCall::hold");
  if (isOnHold()) {
    LOGI("ImsCall::Hold::call is already on hold");
    return NS_OK;
  }
  if (mUpdateRequest != UPDATE_NONE) {
    LOGI("ImsCall::Hold::update is in progress request = %s",updateRequestToString(mUpdateRequest));
  }
  if (mSession != nullptr) {
    mSession->hold(createHoldMediaProfile());
    mHold = true;
    mUpdateRequest = UPDATE_HOLD;
  }
  return NS_OK;
}

/*




*/
NS_IMETHODIMP
ImsCall::Resume()
{
  LOGI("ImsCall::resume");
  if (!isOnHold()) {
    LOGI("ImsCall::resume::call is in conversation");
    return NS_OK;
  }
  if (mUpdateRequest != UPDATE_NONE) {
    LOGI("ImsCall::Resume::update is in progress; request %s",updateRequestToString(mUpdateRequest));
  }
  if (mSession != nullptr) {
    mUpdateRequest = UPDATE_RESUME;
    //
    //
    mSession->resume(createResumeMediaProfile());
  }
  return NS_OK;
}

/*




*/
void ImsCall::merge()
{
  LOGI("ImsCall::merge");
  if (mUpdateRequest != UPDATE_NONE) {
    LOGE("ImsCall::merge::update is in progress; request %s", updateRequestToString(mUpdateRequest));
  }
  if (mSession == nullptr) {
    LOGE("ImsCall::merge::no session");
    return ;
  }
  bool skipHoldBeforeMerge = true;
  //
  //
  if (mHold || skipHoldBeforeMerge) {
    if (mMergePeer != nullptr && !mMergePeer->isMultiparty() && !isMultiparty()) {
      //
      //
      //
      //
      //
      //
      //
      mUpdateRequest = UPDATE_MERGE;
      mMergePeer->mUpdateRequest = UPDATE_MERGE;
    }
    if (mSession != nullptr) {
      mSession->merge();
    }
  } else {
    //
    //
    //
    mSession->hold(createHoldMediaProfile());
    mHold = true;
    mUpdateRequest = UPDATE_HOLD_MERGE;
  }

}

/*





*/
NS_IMETHODIMP
ImsCall::Merge(nsIImsCall *pbgCall)
{
  LOGI("ImsCall::merge(bgCall)::session - %d, pbgCall - %d, ImsCallSession - %d", this, pbgCall, mSession);
  if(pbgCall == nullptr) {
    LOGE("ImsCall::merge(bgCall)::bgCall is null");
    return NS_OK;
  }
  ImsCall *bgCall = (ImsCall*)pbgCall;
  if(!this->ConferenceHost && bgCall->ConferenceHost)
    bgCall = bgCall->ConferenceHost;
  else if(this->ConferenceHost  && !bgCall->ConferenceHost)
    bgCall = this->ConferenceHost;

  //
  setCallSessionMergePending(true);
  bgCall->setCallSessionMergePending(true);
  if ((!isMultiparty() && !bgCall->isMultiparty()) || isMultiparty()) {
    //
    //
    //
    //
    //
    setMergePeer(bgCall);
  } else {
    //
    setMergeHost(bgCall);
  }
  updateConferenceParticipantsList(bgCall);
  merge();
  return NS_OK;
}

/*

*/
NS_IMETHODIMP
ImsCall::Update(int32_t callType, nsIImsStreamMediaProfile *mediaProfile)
{
  LOGI("ImsCall::update");
  if (isOnHold()) {
    LOGI("ImsCall::update::call is on hold");
    return NS_OK;
  }
  if (mUpdateRequest != UPDATE_NONE) {
    LOGI("ImsCall::update::update is in progress request =%s", updateRequestToString(mUpdateRequest));
  }
  if(mSession != nullptr) {
    mSession->update(callType, (ImsStreamMediaProfile *)mediaProfile);
  }
  mUpdateRequest = UPDATE_UNSPECIFIED;
  return NS_OK;
}

/*



*/
NS_IMETHODIMP
ImsCall::ExtendToConference(nsTArray<nsString> &participants)
{
  LOGI("ImsCall::extendToConference");
  if (isOnHold()) {
    LOGE("ImsCall::extendToConference::call is on hold");
  }
  if (mUpdateRequest != UPDATE_NONE) {
    LOGE("ImsCall::ExtendToConference::update is in progressrequest =%s", updateRequestToString(mUpdateRequest));
  }
  if (mSession == nullptr) {
    LOGE("ImsCall::ExtendToConference::no session");
    return NS_OK;
  }
  mSession->extendToConference(participants);
  mUpdateRequest = UPDATE_EXTEND_TO_CONFERENCE;
  return NS_OK;
}

/*


*/
NS_IMETHODIMP
ImsCall::InviteParticipants(nsTArray<nsString> &participants)
{
  LOGI("ImsCall::inviteParticipants");
  if (mSession == nullptr) {
    LOGE("ImsCall::InviteParticipants::no session");
    return NS_OK;
  }
  mSession->inviteParticipants(participants);
  return NS_OK;
}

/*


*/
NS_IMETHODIMP
ImsCall::RemoveParticipants(nsTArray<nsString> &participants)
{
  LOGI("ImsCall::removeParticipants::To-Do");
#if 0
  if (mSession == nullptr) {
    LOGE("ImsCall::RemoveParticipants::no session");
    return ;
  }
  mSession->removeParticipants(participants);
  if (!mIsCEPPresent && participants != nullptr) {
    for(int index = 0; index < participants.Length(); index++) {
      for(int count = 0; count < mConferenceParticipants.Length(); count++) {
        //
      }
    }
  }//
#endif
  return NS_OK;
}

/*





*/
NS_IMETHODIMP
ImsCall::SendDtmf(const nsAString & c)
{
  LOGI("ImsCall::sendDtmf");
  mSession->SendDtmf(c);
  return NS_OK;
}

/*





*/
void ImsCall::startDtmf(char *c)
{
  LOGI("ImsCall::startDtmf");
  mSession->startDtmf(c);
}

/*

*/
void ImsCall::stopDtmf()
{
  LOGI("ImsCall::stopDtmf");
  mSession->stopDtmf();
}

/*



*/
NS_IMETHODIMP
ImsCall::SendUssd(const nsAString& aString)
{
  LOGI("ImsCall::SendUssd:To-Do");
  mSession->sendUssd(aString);
  return NS_OK;
}

//
/*

*/
void ImsCall::explicitCallTransfer()
{
  LOGI("ImsCall::explicitCallTransfer");
  mSession->explicitCallTransfer();
}

//

void ImsCall::clear(ImsReasonInfo *lastReasonInfo)
{
  mInCall = false;
  mHold = false;
  mUpdateRequest = UPDATE_NONE;
  if(mLastReasonInfo)
    delete mLastReasonInfo;
  mLastReasonInfo = new ImsReasonInfo();
  mLastReasonInfo->copyFrom(lastReasonInfo);
}

/*

*/
ImsCall *ImsCall::createCallSessionListener()
{
  return mImsCall;
}

ImsCall *ImsCall::createNewCall(ImsCallSession *session, ImsCallProfile *profile)
{
  ImsCall *call = new ImsCall(profile);
  call->attachSession(session);
  return call;
}

ImsStreamMediaProfile *ImsCall::createHoldMediaProfile()
{
  ImsStreamMediaProfile *mediaProfile = new ImsStreamMediaProfile();
  if (mCallProfile == nullptr) {
    return mediaProfile;
  }
  mediaProfile->mAudioQuality = mCallProfile->mMediaProfile->mAudioQuality;
  mediaProfile->mVideoQuality = mCallProfile->mMediaProfile->mVideoQuality;
  mediaProfile->mAudioDirection = ImsStreamMediaProfile::DIRECTION_SEND;

  if (mediaProfile->mVideoQuality != ImsStreamMediaProfile::VIDEO_QUALITY_NONE) {
    mediaProfile->mVideoDirection = ImsStreamMediaProfile::DIRECTION_SEND;
  }
  return mediaProfile;
}

ImsStreamMediaProfile *ImsCall::createResumeMediaProfile()
{
  ImsStreamMediaProfile *mediaProfile = new ImsStreamMediaProfile();

  if (mCallProfile == nullptr) {
    return mediaProfile;
  }

  mediaProfile->mAudioQuality = mCallProfile->mMediaProfile->mAudioQuality;
  mediaProfile->mVideoQuality = mCallProfile->mMediaProfile->mVideoQuality;
  mediaProfile->mAudioDirection = ImsStreamMediaProfile::DIRECTION_SEND_RECEIVE;

  if (mediaProfile->mVideoQuality != ImsStreamMediaProfile::VIDEO_QUALITY_NONE) {
    mediaProfile->mVideoDirection = ImsStreamMediaProfile::DIRECTION_SEND_RECEIVE;
  }

  return mediaProfile;
}

void ImsCall::enforceConversationMode()
{
  if (mInCall) {
    mHold = false;
    mUpdateRequest = UPDATE_NONE;
  }
}

void ImsCall::mergeInternal()
{
  LOGI("ImsCall::mergeInternal");
  mSession->merge();
  mUpdateRequest = UPDATE_MERGE;
}

void ImsCall::notifyConferenceSessionTerminated(ImsReasonInfo *reasonInfo)
{
  LOGI("ImsCall::notifyConferenceSessionTerminated");
  clear(reasonInfo);
#if 0
  if(m_pIAUCSessionListener != nullptr) {
    m_pIAUCSessionListener->OnCallTerminated(static_cast<nsIImsCall *>(this), static_cast<nsIImsReasonInfo *>(reasonInfo));
  } else {
    LOGE("ImsCall::callSessionTerminated::m_pIAUCSessionListener is null");
  }
#endif
  CallOnCallTermate(reasonInfo);
}


void ImsCall::notifyConferenceStateUpdated(ImsConferenceState *state)
{
  LOGI("ImsCall::notifyConferenceStateUpdated");
  NS_LossyConvertUTF16toASCII convertCallee(sCalleePartyNum);
  if(convertCallee.Length() > 0) {
    LOGI("ImsCall::notifyConferenceStateUpdated::callee - %s", convertCallee.get());
  } else {
    return;
  }
  LOGI("ImsCall::notifyConferenceStateUpdated::mParticipants size - %d", state->mParticipants.size());
  if (state->mParticipants.find(sCalleePartyNum) != state->mParticipants.end()) {
    LOGI("ImsCall::notifyConferenceStateUpdated::match found");
    Bundle *pBundle = state->mParticipants[sCalleePartyNum];
    if(pBundle == nullptr){
      LOGI("ImsCall::notifyConferenceStateUpdated::pBundle is null");
      return ;
    }
    nsString empty;
    nsString nsConfState = pBundle->getString(state->STATUS, empty);
    int32_t mConfState = state->getConnectionStateForStatus(nsConfState);
    LOGI("ImsCall::notifyConferenceStateUpdated::callee - %s, mState - %d", convertCallee.get(), mConfState);
    std::map<nsString, int32_t> Confstate;
    Confstate.insert ( std::pair<nsString, int32_t>(sCalleePartyNum,mState));
    state->SetConfState(Confstate);
  } else {
    LOGI("ImsCall::notifyConferenceStateUpdated::match not found");
  }
}


/*







*/
void ImsCall::processCallTerminated(ImsReasonInfo *reasonInfo)
{
  LOGI("ImsCall::processCallTerminated");

  //
  //
  //
  if (isCallSessionMergePending()) {
    //
    //
    //
    //
    //
    //
    LOGI("processCallTerminated :: burying termination during ongoing merge.");

    mSessionEndDuringMerge = true;
    mSessionEndDuringMergeReasonInfo = reasonInfo;

    //
    //
    //
    //
    if (mTransientConferenceSession != nullptr) {
      processMergeComplete();
      return;
    }
  }

  //
  if (isMultiparty()) {
      notifyConferenceSessionTerminated(reasonInfo);
      return;
  } else {
      clear(reasonInfo);
  }
  mIsCEPPresent = false;
#if 0
  if(m_pIAUCSessionListener != nullptr) {
    m_pIAUCSessionListener->OnCallTerminated(static_cast<nsIImsCall *>(this), static_cast<nsIImsReasonInfo *>(reasonInfo));
  } else {
    LOGE("ImsCall::callSessionTerminated::m_pIAUCSessionListener is null");
  }
#endif
  CallOnCallTermate(reasonInfo);
}

/*









*/
bool ImsCall::isTransientConferenceSession(ImsCallSession *session)
{
  LOGI("ImsCall::isTransientConferenceSession");
  if(mSession == nullptr){
    LOGI("ImsCall::isTransientConferenceSession::mSession is null");
  }
  if(session == nullptr){
    LOGI("ImsCall::isTransientConferenceSession::session is null");
  }
  if(mTransientConferenceSession == nullptr){
    LOGI("ImsCall::isTransientConferenceSession::mTransientConferenceSession is null");
    return false;
  }
  if (session != nullptr && session != mSession && session == mTransientConferenceSession) {
    LOGI("ImsCall::isTransientConferenceSession::true");
    return true;
  }
  LOGI("ImsCall::isTransientConferenceSession::false");
  return false;
}

void
ImsCall::setTransientSessionAsPrimary(ImsCallSession *transientSession)
{
  LOGI("ImsCall::setTransientSessionAsPrimary::ToDo");
  //
  ImsCallSession* session = mSession;
  //

  mSession->setListener(nullptr);
  mSession = transientSession;
  mSession->setListener(createCallSessionListener());
#if 0//
  //
  if (LGImsProfile.isLGImsStackSupported()) {
    if (session != nullptr) {
      session.close();
    }
  }
  //
#else
  /*

   */
  //
  //
  nsCOMPtr<nsIRunnable> proc = new IMSCallToB2gRunnable(session);
  if (NS_FAILED(NS_DispatchToMainThread(proc))) {
    LOGE("ImsCall::setTransientSessionAsPrimary close error");
    return;
  }
 #endif
}

/*


*/
void ImsCall::tryProcessConferenceResult()
{
  LOGI("ImsCall::tryProcessConferenceResult");
  if (shouldProcessConferenceResult()) {
    if (isMergeHost()) {
      processMergeComplete();
    } else if (mMergeHost != nullptr) {
      mMergeHost->processMergeComplete();
    } else {
      //
       LOGI("tryProcessConferenceResult :: No merge host for this conference!");
    }
  }
  LOGI("ImsCall::tryProcessConferenceResult::end");
}

/*




*/
void ImsCall::processMergeComplete()
{
  LOGI("ImsCall::processMergeComplete");
  if (!isMergeHost()) {
    LOGI("processMergeComplete :: We are not the merge host!");
    return;
  }
  nsIImsCallListener* listener;
  bool swapRequired = false;
  ImsCall* finalHostCall = this;
  ImsCall* finalPeerCall = mMergePeer;

  if (isMultiparty()) {
    //
    //
    //
    setIsMerged(false);
    if (!isSessionAlive(mMergePeer->mSession)) {
      //
      //
      mMergePeer->setIsMerged(true);
    } else {
      mMergePeer->setIsMerged(false);
    }
  } else {
    //
     //
     //
     //
     //
     if (mTransientConferenceSession == nullptr) {
       LOGI("processMergeComplete :: No transient session!");
       return;
     }
     if (mMergePeer == nullptr) {
       LOGI("processMergeComplete :: No merge peer!");
       return;
     }

     //
     //
     ImsCallSession* transientConferenceSession = mTransientConferenceSession;
     mTransientConferenceSession = nullptr;

    //
    //
    transientConferenceSession->setListener(nullptr);

    //
    //
    //
    //
    //
    //

    if (isSessionAlive(mSession) && !isSessionAlive(mMergePeer->getCallSession())) {
        //
        //
       //
       //
       //
       //
       //
       //
       finalHostCall = mMergePeer;
       finalPeerCall = this;
       swapRequired = true;
       setIsMerged(false);
       mMergePeer->setIsMerged(false);
       LOGI("processMergeComplete :: transient will transfer to merge peer");
    } else if (!isSessionAlive(mSession) && isSessionAlive(mMergePeer->getCallSession())) {
        //
        //
        //
        finalHostCall = this;
        finalPeerCall = mMergePeer;
        swapRequired = false;
        setIsMerged(false);
        mMergePeer->setIsMerged(false); //
        LOGI("processMergeComplete :: transient will stay with the merge host");
     } else {
        //
        //
        //
        finalHostCall = this;
        finalPeerCall = mMergePeer;
        swapRequired = false;
        setIsMerged(false);
        mMergePeer->setIsMerged(true);
        LOGI("processMergeComplete :: transient will stay with us (I'm the host).");
     }
     LOGI("processMergeComplete :: call=%d  is the final host", finalHostCall);

     //
     //
     finalHostCall->setTransientSessionAsPrimary(transientConferenceSession);
   }

  listener = finalHostCall->m_pIAUCSessionListener;

   //
   clearMergeInfo();

   //
   //
   finalPeerCall->notifySessionTerminatedDuringMerge();
   //
   //
   finalHostCall->clearSessionTerminationFlags();
#if 0 //
   //
   if (LGImsProfile.isLGImsStackSupported()) {
       if (finalHostCall.isOnHold() && finalHostCall.isInCall()) {
           log("Host call is on hold");
           finalHostCall.enforceConversationMode();
       }

       if (finalPeerCall.isOnHold() && finalPeerCall.isInCall()) {
           log("Peer call is on hold");
           finalPeerCall.enforceConversationMode();
       }
   }
   //
#endif
   if (listener != nullptr) {
     //
     LOGI("ImsCall::processMergeComplete::session - %d", this);
     listener->OnCallMerged(this, nullptr);

     this->ConferenceHost = finalHostCall;
     finalPeerCall->ConferenceHost  = finalHostCall;
#if 0//
     if (!mConferenceParticipants.IsEmpty()
                    && listener != nullptr) {
       listener->OnConferenceParticipantsStateChanged(this, mConferenceParticipants);
     }
#endif
  }
  return;
}

/*


*/
void ImsCall::notifySessionTerminatedDuringMerge()
{
  LOGI("ImsCall::notifySessionTerminatedDuringMerge");

  nsIImsCallListener* listener;
  bool notifyFailure = false;
  ImsReasonInfo* notifyFailureReasonInfo = nullptr;

  listener = m_pIAUCSessionListener;
  if (mSessionEndDuringMerge) {
    //
    //
    //
    LOGI("notifySessionTerminatedDuringMerge ::reporting terminate during merge");
    notifyFailure = true;
    notifyFailureReasonInfo = mSessionEndDuringMergeReasonInfo;
  }
  clearSessionTerminationFlags();
  if (listener != nullptr && notifyFailure) {
    LOGI("notifySessionTerminatedDuringMerge ::processCallTerminated");
    processCallTerminated(notifyFailureReasonInfo);
  }
  LOGI("notifySessionTerminatedDuringMerge ::End");
}

void ImsCall::clearSessionTerminationFlags()
{
  LOGI("ImsCall::clearSessionTerminationFlags");

  mSessionEndDuringMerge = false;
  mSessionEndDuringMergeReasonInfo = nullptr;
}

/*







*/
void ImsCall::processMergeFailed(ImsReasonInfo *reasonInfo)
{
  LOGI("ImsCall::processMergeFailed::To-Do");

  nsIImsCallListener* listener;

  //
  //
  if (!isMergeHost()) {
    LOGI("processMergeFailed :: We are not the merge host!");
    return;
  }

  if (mMergePeer == nullptr) {
    LOGI("processMergeFailed :: No merge peer!");
    return;
  }

  if (!isMultiparty()) {
#if 0
    //
    if (LGImsProfile.isLGImsStackSupported()) {
      if (mTransientConferenceSession == nullptr) {
        LOGE("processMergeFailed :: No transient session!");
        LOGI("LGIMS :: merge failed before the creation of conference session");
      } else {
        //
        mTransientConferenceSession->setListener(nullptr);
        mTransientConferenceSession = nullptr;
      }
    } else
#endif
    {
      if (mTransientConferenceSession == nullptr) {
        LOGE("processMergeFailed :: No transient session!");
        return;
      }

      //
      mTransientConferenceSession->setListener(nullptr);
      mTransientConferenceSession = nullptr;
    }
         //
  }
   //
   setIsMerged(false);
   mMergePeer->setIsMerged(false);

   listener = m_pIAUCSessionListener;

  //
  notifySessionTerminatedDuringMerge();
  mMergePeer->notifySessionTerminatedDuringMerge();

  //
  clearMergeInfo();

  if (listener != nullptr) {
    listener->OnCallMergeFailed(this, reasonInfo);
  }
  return;
}

void ImsCall::notifyError(int32_t reason, int32_t statusCode, const nsString& message)
{

}

void ImsCall::updateConferenceParticipantsList(ImsCall *bgCall)
{
  LOGI("ImsCall::updateConferenceParticipantsList");
  if (bgCall == nullptr) return;
    ImsCall* confCall = this;
    ImsCall* childCall = bgCall;
    if (bgCall->isMultiparty()) {
      //
      LOGI("updateConferenceParticipantsList: BG call is conference");
      confCall = bgCall;
      childCall = this;
    } else if (!isMultiparty()) {
      //
      //
      LOGI("updateConferenceParticipantsList: Make this call as conference and add child");
      addToConferenceParticipantList(this);
  }
  confCall->addToConferenceParticipantList(childCall);
}

void ImsCall::addToConferenceParticipantList(ImsCall *childCall)
{
  LOGI("ImsCall::addToConferenceParticipantList");
  if (childCall == nullptr) return;

  nsString empty;

  ImsCallProfile* profile = childCall->getCallProfile();
  if (profile == nullptr) {
      LOGI("addToConferenceParticipantList: null profile for childcall");
      return;
  }
  nsString handle = profile->getCallExtra(ImsCallProfile::EXTRA_OI, empty);
  nsString name = profile->getCallExtra(ImsCallProfile::EXTRA_CNA, empty);
  if (handle == nullptr) {
      LOGI("addToConferenceParticipantList: Invalid number for childcall");
      return;
  }
  //
  nsString userUri;
  ConferenceParticipant* participant = new ConferenceParticipant(userUri,
          name, userUri, 0/*                             */);
      LOGI("Adding participant: %d" , participant);
      mConferenceParticipants.AppendElement(participant);
      if (isMultiparty() && !mIsCEPPresent && !mConferenceParticipants.IsEmpty()
          && m_pIAUCSessionListener != nullptr) {
              //
              //
     }
}
void ImsCall::callSessionProgressing(ImsCallSession *session, ImsStreamMediaProfile *profile)
{
  LOGI("ImsCall::callSessionProgressing");

  if (isTransientConferenceSession(session)) {
      LOGI("callSessionProgressing :: not supported for transient conference session");
      return;
  }
  mCallProfile->mMediaProfile->copyFrom(profile);
  SetHDIcon(mCallProfile->getCallExtraBoolean(ImsCallProfile::EXTRA_HD_VOICE));
  if(m_pIAUCSessionListener != nullptr) {
    m_pIAUCSessionListener->OnCallProgressing(static_cast<nsIImsCall *>(this));
  } else {
    LOGE("ImsCall::callSessionProgressing::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionStarted(ImsCallSession *session, ImsCallProfile *profile)
{
  LOGI("ImsCall::callSessionStarted::session - %d, ImsCallSession - %d", this, session);

  if (!isTransientConferenceSession(session)) {
    //
    //
    //
    setCallSessionMergePending(false);
  } else {
    LOGI("callSessionStarted :: on transient session=%d" , session);
    return;
  }

   //
   //
   tryProcessConferenceResult();

   if (isTransientConferenceSession(session)) {
     //
     LOGI("callSessionStarted :: transient conference session resumed session=");
     return;
   }
  if(mCallProfile)
    delete mCallProfile;
  mCallProfile = new ImsCallProfile();
  mCallProfile->copyFrom(profile);
  nsString empty;
  nsString handle = profile->getCallExtra(ImsCallProfile::EXTRA_OI, empty);
  nsString name = profile->getCallExtra(ImsCallProfile::EXTRA_CNA, empty);
  NS_LossyConvertUTF16toASCII convertName(name);
  NS_LossyConvertUTF16toASCII convertHandle(handle);
  if(convertHandle.Length() > 0) {
    LOGI("ImsCall::callSessionStarted::handle - %s", convertHandle.get());
  }
  if(convertName.Length() > 0) {
    LOGI("ImsCall::callSessionStarted::name - %s", convertName.get());
  }
  mInCall = true;
  SetHDIcon(mCallProfile->getCallExtraBoolean(ImsCallProfile::EXTRA_HD_VOICE));
  if(m_pIAUCSessionListener != nullptr) {
    m_pIAUCSessionListener->OnCallStarted(static_cast<nsIImsCall *>(this));
  } else {
    LOGE("ImsCall::callSessionStarted::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionStartFailed(ImsCallSession *session, ImsReasonInfo *reasonInfo)
{
  LOGI("ImsCall::callSessionStartFailed");

  if (isTransientConferenceSession(session)) {
     //
     LOGI("callSessionStartFailed :: transient conference session resumed session=");
     return;
  }
  if(mLastReasonInfo)
    delete mLastReasonInfo;
  mLastReasonInfo = new ImsReasonInfo();
  mLastReasonInfo->copyFrom(static_cast<ImsReasonInfo *>(reasonInfo));
  if(m_pIAUCSessionListener != nullptr) {
    m_pIAUCSessionListener->OnCallStartFailed(static_cast<nsIImsCall *>(this), static_cast<nsIImsReasonInfo *>(reasonInfo));
  } else {
    LOGE("ImsCall::callSessionStartFailed::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionTerminated(ImsCallSession *session, ImsReasonInfo *reasonInfo)
{
  LOGI("ImsCall::callSessionTerminated");

  if (mSession != session) {
    if (isTransientConferenceSession(session)) {
      processMergeFailed(reasonInfo);
      LOGI("callSessionTerminated :: for transient session");
    }
    return;
  }


  //
  //
  //
  processCallTerminated(reasonInfo);

  //
  setCallSessionMergePending(false);

  //
  //
  tryProcessConferenceResult();
}

void ImsCall::callSessionHeld(ImsCallSession *session, ImsCallProfile *profile)
{
  LOGI("ImsCall::callSessionHeld");

  if (isTransientConferenceSession(session)) {
     //
     LOGI("callSessionHeld :: transient conference session resumed session=");
     return;
  }
  setCallSessionMergePending(false);
  if(mCallProfile)
    delete mCallProfile;
  mCallProfile = new ImsCallProfile();
  mCallProfile->copyFrom(profile);
    if (mUpdateRequest == UPDATE_HOLD_MERGE) {
    //
    mergeInternal();
    return;
  }

  //
  //
  //
  tryProcessConferenceResult();
  mUpdateRequest = UPDATE_NONE;

  SetHDIcon(mCallProfile->getCallExtraBoolean(ImsCallProfile::EXTRA_HD_VOICE));
  if(m_pIAUCSessionListener != nullptr) {
    m_pIAUCSessionListener->OnCallHeld(static_cast<nsIImsCall *>(this));
  } else {
    LOGE("ImsCall::callSessionHeld::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionHoldFailed(ImsCallSession *session, ImsReasonInfo *reasonInfo)
{
  LOGI("ImsCall::callSessionHoldFailed");
  if (isTransientConferenceSession(session)) {
     //
     LOGI("callSessionHoldFailed :: transient conference session resumed session=");
     return;
  }
  mHold = false;
  mUpdateRequest = UPDATE_NONE;
  if(m_pIAUCSessionListener != nullptr) {
    m_pIAUCSessionListener->OnCallHoldFailed(static_cast<nsIImsCall *>(this), static_cast<nsIImsReasonInfo *>(reasonInfo));
  } else {
    LOGE("ImsCall::callSessionHoldFailed::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionHoldReceived(ImsCallSession *session, ImsCallProfile *profile)
{
  LOGI("ImsCall::callSessionHoldReceived");
  if (isTransientConferenceSession(session)) {
     //
     LOGI("callSessionHoldReceived :: transient conference session resumed session=");
     return;
  }
  if(mCallProfile)
    delete mCallProfile;
  mCallProfile = new ImsCallProfile();
  mCallProfile->copyFrom(profile);
  SetHDIcon(mCallProfile->getCallExtraBoolean(ImsCallProfile::EXTRA_HD_VOICE));
  if(m_pIAUCSessionListener != nullptr) {
    m_pIAUCSessionListener->OnCallHoldReceived(static_cast<nsIImsCall *>(this));
  } else {
    LOGE("ImsCall::callSessionHoldReceived::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionResumed(ImsCallSession *session, ImsCallProfile *profile)
{
  LOGI("ImsCall::callSessionResumed");
  if (isTransientConferenceSession(session)) {
     //
     LOGI("callSessionResumed :: transient conference session resumed session=");
     return;
  }
  //
  //
  setCallSessionMergePending(false);

  //
  //
  tryProcessConferenceResult();

  if(mCallProfile)
    delete mCallProfile;
  mCallProfile = new ImsCallProfile();
  mCallProfile->copyFrom(profile);
  mUpdateRequest = UPDATE_NONE;
  mHold = false;
  SetHDIcon(mCallProfile->getCallExtraBoolean(ImsCallProfile::EXTRA_HD_VOICE));
  if(m_pIAUCSessionListener != nullptr) {
    m_pIAUCSessionListener->OnCallResumed(static_cast<nsIImsCall *>(this));
  } else {
    LOGE("ImsCall::callSessionResumed::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionResumeFailed(ImsCallSession *session, ImsReasonInfo *reasonInfo)
{
  LOGI("ImsCall::callSessionResumeFailed");
  if (isTransientConferenceSession(session)) {
     //
     LOGI("callSessionResumeFailed :: transient conference session resumed session=");
     return;
  }
  mHold = true;
  mUpdateRequest = UPDATE_NONE;
  if(m_pIAUCSessionListener != nullptr) {
    m_pIAUCSessionListener->OnCallResumeFailed(static_cast<nsIImsCall *>(this), static_cast<nsIImsReasonInfo *>(reasonInfo));
  } else {
    LOGE("ImsCall::callSessionResumeFailed::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionResumeReceived(ImsCallSession *session, ImsCallProfile *profile)
{
  LOGI("ImsCall::callSessionResumeReceived");
  if (isTransientConferenceSession(session)) {
     //
     LOGI("callSessionResumeReceived :: transient conference session resumed session=");
     return;
  }
  if(mCallProfile)
    delete mCallProfile;
  mCallProfile = new ImsCallProfile();
  mCallProfile->copyFrom(profile);
  if(m_pIAUCSessionListener != nullptr) {
    m_pIAUCSessionListener->OnCallResumeReceived(static_cast<nsIImsCall *>(this));
  } else {
    LOGE("ImsCall::callSessionResumeReceived::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionMergeStarted(ImsCallSession *session, ImsCallSession *newSession, ImsCallProfile *profile)
{
  LOGI("ImsCall::callSessionMergeStarted::session - %d, newSession - %d", this, newSession);

  if (!isCallSessionMergePending()) {
    //
    LOGI("callSessionMergeStarted :: no merge in progress.");
    return;
  }

  //
  //
  //
  //
  if (session == nullptr) {
    //
    //
    //
    LOGI("callSessionMergeStarted :: merging into existing ImsCallSession");
    return;
  }
  LOGI("callSessionMergeStarted ::  setting our transient ImsCallSession");

  //
  //
  //

  //
  //
  mTransientConferenceSession = newSession;
  newSession = nullptr;
  mTransientConferenceSession->setListener(createCallSessionListener());
  return;
}

void ImsCall::callSessionMergeComplete(ImsCallSession *session)
{
  LOGI("ImsCall::callSessionMergeComplete::session - %d", this);
  setCallSessionMergePending(false);

  //
  //
  tryProcessConferenceResult();
  LOGI("ImsCall::callSessionMergeComplete::end");
}

void ImsCall::callSessionMergeFailed(ImsCallSession *session, ImsReasonInfo *reasonInfo)
{
  if (!isCallSessionMergePending()) {
  //
    LOGI("callSessionMergeFailed :: no merge in progress.");
    return;
  }
  //
  //
  //
  //
  if (isMergeHost()) {
    processMergeFailed(reasonInfo);
  } else if (mMergeHost != nullptr) {
    mMergeHost->processMergeFailed(reasonInfo);
  } else {
    LOGI("callSessionMergeFailed :: No merge host for this conference!");
  }
}

void ImsCall::callSessionUpdated(ImsCallSession *session, ImsCallProfile *profile)
{
  LOGI("ImsCall::callSessionUpdated");
  if (isTransientConferenceSession(session)) {
     //
     LOGI("callSessionUpdated :: transient conference session resumed session=");
     return;
  }
  if(mCallProfile)
    delete mCallProfile;
  mCallProfile = new ImsCallProfile();
  mCallProfile->copyFrom(profile);
  mUpdateRequest = UPDATE_NONE;
  if(m_pIAUCSessionListener != nullptr) {
    m_pIAUCSessionListener->OnCallUpdated(static_cast<nsIImsCall *>(this));
  } else {
    LOGE("ImsCall::callSessionUpdated::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionUpdateFailed(ImsCallSession *session, ImsReasonInfo *reasonInfo)
{
  LOGI("ImsCall::callSessionUpdateFailed");
  if (isTransientConferenceSession(session)) {
     //
     LOGI("callSessionUpdateFailed :: transient conference session resumed session=");
     return;
  }
  mUpdateRequest = UPDATE_NONE;
  if(m_pIAUCSessionListener != nullptr) {
    m_pIAUCSessionListener->OnCallUpdateFailed(static_cast<nsIImsCall *>(this), static_cast<nsIImsReasonInfo*>(reasonInfo));
  } else {
    LOGE("ImsCall::callSessionUpdateFailed::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionUpdateReceived(ImsCallSession *session, ImsCallProfile *profile)
{
  LOGI("ImsCall::callSessionUpdateReceived");
  if (isTransientConferenceSession(session)) {
     //
     LOGI("callSessionUpdateFailed :: transient conference session resumed session=");
     return;
  }
  mProposedCallProfile = profile;
  mUpdateRequest = UPDATE_UNSPECIFIED;
  if(m_pIAUCSessionListener != nullptr) {
    m_pIAUCSessionListener->OnCallUpdateReceived(static_cast<nsIImsCall *>(this));
  } else {
    LOGE("ImsCall::callSessionUpdateReceived::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionConferenceExtended(ImsCallSession *session, ImsCallSession *newSession, ImsCallProfile *profile)
{
  LOGI("ImsCall::callSessionConferenceExtended");
  if (isTransientConferenceSession(session)) {
     //
     LOGI("callSessionUpdateFailed :: transient conference session resumed session=");
     return;
  }
  ImsCall* newCall = nullptr;
#if 0//
  if (!LGImsProfile.isLGImsStackSupported()) {
      /*       */ newCall = createNewCall(newSession, profile);

      if (newCall == nullptr) {
          callSessionConferenceExtendFailed(session, new ImsReasonInfo());
          return;
      }
  } else
#endif
  {
      if (newSession != nullptr) {
          newCall = createNewCall(newSession, profile);
          if (newCall == nullptr) {
              callSessionConferenceExtendFailed(session, new ImsReasonInfo());
              return;
          }
      }
  }
  mUpdateRequest = UPDATE_NONE;
#if 0
  //
  if (LGImsProfile.isLGImsStackSupported()) {
    if (newCall == nullptr) {
      if(mCallProfile)
        delete mCallProfile;
      mCallProfile = new ImsCallProfile();
      mCallProfile->copyFrom(profile);
    }
  }
  //
 #endif
  if(m_pIAUCSessionListener != nullptr) {
    m_pIAUCSessionListener->OnCallConferenceExtendReceived(static_cast<nsIImsCall *>(this), static_cast<nsIImsCall *>(newCall));
  } else {
    LOGE("ImsCall::callSessionConferenceExtended::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionConferenceExtendFailed(ImsCallSession *session, ImsReasonInfo *reasonInfo)
{
  LOGI("ImsCall::callSessionConferenceExtendFailed");
  if (isTransientConferenceSession(session)) {
     //
     LOGI("callSessionUpdateFailed :: transient conference session resumed session=");
     return;
  }
  mUpdateRequest = UPDATE_NONE;
  if(m_pIAUCSessionListener != nullptr) {
    m_pIAUCSessionListener->OnCallConferenceExtendFailed(static_cast<nsIImsCall *>(this), static_cast<nsIImsReasonInfo *>(reasonInfo));
  } else {
    LOGE("ImsCall::callSessionConferenceExtendFailed::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionConferenceExtendReceived(ImsCallSession *session, ImsCallSession *newSession, ImsCallProfile *profile)
{
  LOGI("ImsCall::callSessionConferenceExtendReceived");
  if (isTransientConferenceSession(session)) {
     //
     LOGI("callSessionConferenceExtendReceived :: transient conference session resumed session=");
     return;
  }
  //
  ImsCall* newCall = nullptr;
#if 0//
  if (!LGImsProfile.isLGImsStackSupported()) {
      /*       */ newCall = createNewCall(newSession, profile);
      if (newCall == null) {
          //
          return;
      }
  } else
#endif
  {
      if (newSession != nullptr) {
          newCall = createNewCall(newSession, profile);
          if (newCall == nullptr) {
              //
              return;
          }
      }
  }
  //
#if 0  //
  //
  if (LGImsProfile.isLGImsStackSupported()) {
      if (newCall == null) {
          if(mCallProfile)
            delete mCallProfile;
          mCallProfile = new ImsCallProfile();
          mCallProfile->copyFrom(profile);
      }
  }
  //
#endif
  if(m_pIAUCSessionListener != nullptr) {
    m_pIAUCSessionListener->OnCallConferenceExtendReceived(static_cast<nsIImsCall *>(this), static_cast<nsIImsCall *>(newCall));
  } else {
    LOGE("ImsCall::callSessionConferenceExtendReceived::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionInviteParticipantsRequestDelivered(ImsCallSession *session)
{
  LOGI("ImsCall::callSessionInviteParticipantsRequestDelivered");
  if (isTransientConferenceSession(session)) {
     //
     LOGI("callSessionInviteParticipantsRequestDelivered :: transient conference session resumed session=");
     return;
  }
  if(m_pIAUCSessionListener != nullptr) {
    m_pIAUCSessionListener->OnCallInviteParticipantsRequestDelivered(static_cast<nsIImsCall *>(this));
  } else {
    LOGE("ImsCall::callSessionInviteParticipantsRequestDelivered::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionInviteParticipantsRequestFailed(ImsCallSession *session, ImsReasonInfo *reasonInfo)
{
  LOGI("ImsCall::callSessionInviteParticipantsRequestFailed");
  if (isTransientConferenceSession(session)) {
     //
     LOGI("callSessionInviteParticipantsRequestDelivered :: transient conference session resumed session=");
     return;
  }
  if(m_pIAUCSessionListener != nullptr) {
    m_pIAUCSessionListener->OnCallInviteParticipantsRequestFailed(static_cast<nsIImsCall *>(this), static_cast<nsIImsReasonInfo *>(reasonInfo));
  } else {
    LOGE("ImsCall::callSessionInviteParticipantsRequestFailed::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionRemoveParticipantsRequestDelivered(ImsCallSession *session)
{
  LOGI("ImsCall::callSessionRemoveParticipantsRequestDelivered");
  if (isTransientConferenceSession(session)) {
     //
     LOGI("callSessionInviteParticipantsRequestDelivered :: transient conference session resumed session=");
     return;
  }
  if(m_pIAUCSessionListener != nullptr) {
    m_pIAUCSessionListener->OnCallRemoveParticipantsRequestDelivered(static_cast<nsIImsCall *>(this));
  } else {
    LOGE("ImsCall::callSessionRemoveParticipantsRequestDelivered::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionRemoveParticipantsRequestFailed(ImsCallSession *session, ImsReasonInfo *reasonInfo)
{
  LOGI("ImsCall::callSessionRemoveParticipantsRequestFailed");
  if (isTransientConferenceSession(session)) {
     //
     LOGI("callSessionInviteParticipantsRequestDelivered :: transient conference session resumed session=");
     return;
  }
  if(m_pIAUCSessionListener != nullptr) {
    m_pIAUCSessionListener->OnCallRemoveParticipantsRequestFailed(static_cast<nsIImsCall *>(this), nullptr);
  } else {
    LOGE("ImsCall::callSessionRemoveParticipantsRequestFailed::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionConferenceStateUpdated(ImsCallSession *session, ImsConferenceState *state)
{
  LOGI("ImsCall::callSessionConferenceStateUpdated");
  if (isTransientConferenceSession(session)) {
     //
     LOGI("callSessionInviteParticipantsRequestDelivered :: transient conference session resumed session");
     return;
  }
#if 0//
  //
  if (LGImsProfile.isLGImsStackSupported()) {
      mConfState = state;
  }
  //
#endif
  conferenceStateUpdated(state);
}

void ImsCall::callSessionUssdMessageReceived(ImsCallSession *session, int32_t mode,nsString ussdMessage)
{
  LOGI("ImsCall::callSessionUssdMessageReceived::To-Do");
  if (isTransientConferenceSession(session)) {
     //
     LOGI("callSessionInviteParticipantsRequestDelivered :: transient conference session resumed session=");
     return;
  }
  if(m_pIAUCSessionListener != nullptr) {
    LOGE("ImsCall::callSessionUssdMessageReceived::To-Do");
  } else {
    LOGE("ImsCall::callSessionUssdMessageReceived::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionTtyModeReceived(ImsCallSession *session, int32_t mode)
{
  LOGI("ImsCall::callSessionTtyModeReceived");
  if(m_pIAUCSessionListener != nullptr) {
    LOGE("ImsCall::callSessionTtyModeReceived::To-Do");
  } else {
    LOGE("ImsCall::callSessionTtyModeReceived::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionHandover(ImsCallSession *session, int32_t srcAccessTech, int32_t targetAccessTech, ImsReasonInfo *reasonInfo)
{
  LOGI("ImsCall::callSessionHandover");
  if(m_pIAUCSessionListener != nullptr) {
    LOGE("ImsCall::callSessionHandover::To-Do");
  } else {
    LOGE("ImsCall::callSessionHandover::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionHandoverFailed(ImsCallSession *session, int32_t srcAccessTech, int32_t targetAccessTech, ImsReasonInfo *reasonInfo)
{
  LOGI("ImsCall::callSessionHandoverFailed");
  if(m_pIAUCSessionListener != nullptr) {
    LOGE("ImsCall::callSessionHandoverFailed::To-Do");
  } else {
    LOGE("ImsCall::callSessionHandoverFailed::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionSuppServiceReceived(ImsCallSession *session)
{
  LOGI("ImsCall::callSessionSuppServiceReceived");
  if(m_pIAUCSessionListener != nullptr) {
    LOGE("ImsCall::callSessionSuppServiceReceived::To-Do");
  } else {
    LOGE("ImsCall::callSessionSuppServiceReceived::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionRetryErrorReceived(ImsCallSession *session, ImsReasonInfo *reasonInfo)
{
  LOGI("ImsCall::callSessionRetryErrorReceived");
  if(m_pIAUCSessionListener != nullptr) {
    LOGE("ImsCall::callSessionRetryErrorReceived::To-Do");
  } else {
    LOGE("ImsCall::callSessionRetryErrorReceived::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionExplicitCallTransferred(ImsCallSession *session)
{
  LOGI("ImsCall::callSessionExplicitCallTransferred");
  if(m_pIAUCSessionListener != nullptr) {
    LOGE("ImsCall::callSessionExplicitCallTransferred::To-Do");
  } else {
    LOGE("ImsCall::callSessionExplicitCallTransferred::m_pIAUCSessionListener is null");
  }
}

void ImsCall::callSessionExplicitCallTransferFailed(ImsCallSession *session, ImsReasonInfo *reasonInfo)
{
  LOGI("ImsCall::callSessionExplicitCallTransferFailed");
  if(m_pIAUCSessionListener != nullptr) {
    LOGE("ImsCall::callSessionExplicitCallTransferFailed::To-Do");
  } else {
    LOGE("ImsCall::callSessionExplicitCallTransferFailed::m_pIAUCSessionListener is null");
  }
}

void ImsCall::conferenceStateUpdated(ImsConferenceState* state)
{
  LOGI("ImsCall::conferenceStateUpdated");
  notifyConferenceStateUpdated(state);
   if(m_pIAUCSessionListener != nullptr) {
    m_pIAUCSessionListener->OnCallConferenceStateUpdated(static_cast<nsIImsCall *>(this),
        static_cast<nsIImsConferenceState*>(state));
  } else {
    LOGE("ImsCall::callSessionConferenceStateUpdated::m_pIAUCSessionListener is null");
  }
}

/*




 */
char*
ImsCall::updateRequestToString(int updateRequest) {
    switch (updateRequest) {
        case UPDATE_NONE:
            return "NONE";
        case UPDATE_HOLD:
            return "HOLD";
        case UPDATE_HOLD_MERGE:
            return "HOLD_MERGE";
        case UPDATE_RESUME:
            return "RESUME";
        case UPDATE_MERGE:
            return "MERGE";
        case UPDATE_EXTEND_TO_CONFERENCE:
            return "EXTEND_TO_CONFERENCE";
        case UPDATE_UNSPECIFIED:
            return "UNSPECIFIED";
        default:
            return "UNKNOWN";
    }
}


/*




 */
void
ImsCall::setMergePeer(ImsCall* mergePeer) {
    mMergePeer = mergePeer;
    mMergeHost = nullptr;

    mergePeer->mMergeHost = this;
    mergePeer->mMergePeer = nullptr;
}

/*




 */
void
ImsCall::setMergeHost(ImsCall* mergeHost) {
    mMergeHost = mergeHost;
    mMergePeer = nullptr;

    mergeHost->mMergeHost = nullptr;
    mergeHost->mMergePeer = this;
}

/*



 */
bool
ImsCall::isMerging() {
    return mMergePeer != nullptr || mMergeHost != nullptr;
}

/*



 */
bool
ImsCall::isMergeHost() {
    return mMergePeer != nullptr && mMergeHost == nullptr;
}

/*



 */
bool
ImsCall::isMergePeer() {
    return mMergePeer == nullptr && mMergeHost != nullptr;
}

/*



 */
bool
ImsCall::isCallSessionMergePending() {
    LOGI("ImsCall :: isCallSessionMergePending - %d", mCallSessionMergePending);
    return mCallSessionMergePending;
}

/*




 */
void
ImsCall::setCallSessionMergePending(bool callSessionMergePending) {
    LOGI("ImsCall :: setCallSessionMergePending - %d", callSessionMergePending);
    mCallSessionMergePending = callSessionMergePending;
}

/*












 */
bool
ImsCall::shouldProcessConferenceResult()
{
  bool areMergeTriggersDone = false;

  //
  //
  //
  if (!isMergeHost() && !isMergePeer()) {
    LOGI("shouldProcessConferenceResult :: no merge in progress");
    return false;
  }
  //
  //
  //
  if (isMergeHost()) {
    LOGI("shouldProcessConferenceResult::isMergeHost:: We are a merge host=%d", this);
    LOGI("shouldProcessConferenceResult::isMergeHost:: Here is the merge peer=%d" , mMergePeer);
    areMergeTriggersDone = !isCallSessionMergePending() &&!mMergePeer->isCallSessionMergePending();
    LOGI("shouldProcessConferenceResult :: isMergeHost:: areMergeTriggersDone - %d" , areMergeTriggersDone);
    if (!isMultiparty()) {
      LOGI("shouldProcessConferenceResult::isMergeHost:: !isMultiparty()");
      //
      areMergeTriggersDone &= isSessionAlive(mTransientConferenceSession);
    }
  } else if (isMergePeer()) {
    LOGI("shouldProcessConferenceResult::isMergePeer:: We are a merge peer=%d" , this);
    LOGI("shouldProcessConferenceResult::isMergePeer:: Here is the merge host= %d" , mMergeHost);

    areMergeTriggersDone = !isCallSessionMergePending() && !mMergeHost->isCallSessionMergePending();
    if (!mMergeHost->isMultiparty()) {
      LOGI("shouldProcessConferenceResult::isMergePeer:: !isMultiparty()::mMergeHost->mTransientConferenceSession  - %d", mMergeHost->mTransientConferenceSession);
      //
     areMergeTriggersDone &= isSessionAlive(mMergeHost->mTransientConferenceSession);
    } else {
       //
       //
       //
       //
       //
       //
       //
       //
       //
       //
       LOGI("shouldProcessConferenceResult::isMergePeer:: isMultiparty()");
       areMergeTriggersDone = !isCallSessionMergePending();
    }
  } else {
  //
  LOGI("shouldProcessConferenceResult : merge in progress but call is neither host nor peer.");
  }
  LOGI("shouldProcessConferenceResult :: returning: %s",(areMergeTriggersDone ? "true" : "false"));
  return areMergeTriggersDone;
}

#if 0
ConfInfo *ImsCall::createConfInfoFromParticipants(nsTArray<nsString> &participants)
{
  ConfInfo *confInfo = nullptr;
  confInfo = new ConfInfo();
  for(int i = 0; participants.Length(); i++) {
    //
  }
  return confInfo;
}
#endif
/*


*/
void ImsCall::clearMergeInfo()
{
  //
  if (mMergeHost != nullptr) {
    mMergeHost->mMergePeer = nullptr;
    mMergeHost->mUpdateRequest = UPDATE_NONE;
    mMergeHost->mCallSessionMergePending = false;
  }
  if (mMergePeer != nullptr) {
    mMergePeer->mMergeHost = nullptr;
    mMergePeer->mUpdateRequest = UPDATE_NONE;
    mMergePeer->mCallSessionMergePending = false;
  }
  mMergeHost = nullptr;
  mMergePeer = nullptr;
  mUpdateRequest = UPDATE_NONE;
  mCallSessionMergePending = false;

}


void ImsCall::CallOnCallTermate(ImsReasonInfo* reasonInfo)
{
  LOGE("ImsCall::CallOnCallTermate");
  nsCOMPtr<nsIRunnable> proc = new IMSCallToB2gRunnable(this, reasonInfo);
  if (NS_FAILED(NS_DispatchToMainThread(proc))) {
    LOGE("ImsCallSession::onMessage error");
    return;
  }
}

void ImsCall::SendOnCallTermate(ImsReasonInfo* reasonInfo)
{
  LOGE("ImsCall::SendOnCallTermate");
  if(m_pIAUCSessionListener != nullptr) {
    m_pIAUCSessionListener->OnCallTerminated(static_cast<nsIImsCall *>(this), static_cast<nsIImsReasonInfo *>(reasonInfo));
  } else {
    LOGE("ImsCall::callSessionTerminated::m_pIAUCSessionListener is null");
  }
}


//



NS_IMETHODIMP
ImsCall::StartConference(int32_t callType, nsTArray<nsString> &participants)
{
  LOGE("ImsCall::StartConference::To-Do");
  return NS_OK;
}

NS_IMETHODIMP
ImsCall::TerminateConnection(int32_t reason)
{
  LOGE("ImsCall::TerminateConnection::reason - %d", reason);
  android::Parcel ims_parcel;
  ims_parcel.writeInt32(reason);
  nsTArray<nsString> participants;
  NS_LossyConvertUTF16toASCII convertCallee(sCalleePartyNum);
  NS_LossyConvertUTF16toASCII convertCaller(sCallerPartyNum);
  if(convertCallee.Length() > 0) {
    LOGI("ImsCall::TerminateConnection::callee - %s", convertCallee.get());
    participants.AppendElement(sCalleePartyNum);
  } else if(convertCaller.Length() > 0){
    LOGI("ImsCall::TerminateConnection::caller - %s", convertCaller.get());
    participants.AppendElement(sCallerPartyNum);
  }
  if(mSession){
    mSession->removeParticipants(participants);
  } else {
    LOGE("ImsCall::TerminateConnection::mSession is null");
  }
  return NS_OK;
}


NS_IMETHODIMP
ImsCall::GetMediaProfile(nsIImsStreamMediaProfile* *iMediaProfile)
{
  LOGE("ImsCall::GetMediaProfile");
  *iMediaProfile = static_cast<nsIImsStreamMediaProfile *>(mCallProfile->mMediaProfile);
  return NS_OK;
}

NS_IMETHODIMP
ImsCall::GetCallerPartyNum(nsAString & aCallerNum)
{
  NS_LossyConvertUTF16toASCII convertCallerPartyNum(this->sCallerPartyNum);
  LOGE("ImsCall::GetCallerPartyNum - %s", convertCallerPartyNum.get());
  aCallerNum.Assign(this->sCallerPartyNum);
  return NS_OK;
}

NS_IMETHODIMP
ImsCall::SetCallerPartyNum(const nsAString & aCallerNum)
{
  NS_LossyConvertUTF16toASCII convertCallerPartyNum(aCallerNum);
  LOGE("ImsCall::SetCallerPartyNum - %s", convertCallerPartyNum.get());
  sCallerPartyNum.Assign(aCallerNum);
  return NS_OK;
}

NS_IMETHODIMP
ImsCall::GetServiceType(int32_t *aServiceType)
{
  LOGE("ImsCall::GetServiceType - %d", serviceType);
  *aServiceType = serviceType;
  return NS_OK;
}

NS_IMETHODIMP
ImsCall::SetServiceType(int32_t aServiceType)
{
  LOGE("ImsCall::SetServiceType - %d", aServiceType);
  serviceType = aServiceType;
  return NS_OK;
}

NS_IMETHODIMP
ImsCall::GetSessionType(int32_t *aSessionType)
{
  LOGE("ImsCall::GetSessionType - %d", sessionType);
  *aSessionType = sessionType;
  return NS_OK;
}

NS_IMETHODIMP
ImsCall::SetSessionType(int32_t aSessionType)
{
  LOGE("ImsCall::SetSessionType - %d", sessionType);
  sessionType = aSessionType;
  return NS_OK;
}

NS_IMETHODIMP
ImsCall::GetIsConf(bool *aIsConf)
{
  LOGE("ImsCall::GetIsConf - %d", isConf);
  *aIsConf = isConf;
  return NS_OK;
}

NS_IMETHODIMP
ImsCall::SetIsConf(bool aIsConf)
{
  LOGE("ImsCall::SetIsConf - %d", aIsConf);
  isConf = aIsConf;
  return NS_OK;
}

NS_IMETHODIMP
ImsCall::GetOIPType(int32_t *aOIPType)
{
  LOGE("ImsCall::GetOIPType - %d", OIPType);
  *aOIPType = OIPType;
  return NS_OK;
}

NS_IMETHODIMP
ImsCall::SetOIPType(int32_t aOIPType)
{
  LOGE("ImsCall::SetOIPType - %d", aOIPType);
  OIPType = aOIPType;
  return NS_OK;
}

NS_IMETHODIMP
ImsCall::GetHDIcon(int32_t *aHDIcon)
{
  LOGE("ImsCall::GetHDIcon - %d", m_HDIcon);
  *aHDIcon = m_HDIcon;
  return NS_OK;
}

NS_IMETHODIMP
ImsCall::SetHDIcon(int32_t aHDIcon)
{
  LOGE("ImsCall::SetHDIcon - %d", aHDIcon);
  m_HDIcon = aHDIcon;
  return NS_OK;
}

NS_IMETHODIMP
ImsCall::GetSessionKey(int64_t *aSessionKey)
{
  LOGE("ImsCall::GetServiceType - %lld", m_nSessionID);
  *aSessionKey = m_nSessionID;
  return NS_OK;
}

NS_IMETHODIMP
ImsCall::SetSessionKey(int64_t aSessionKey)
{
  LOGE("ImsCall::SetServiceType - %lld", aSessionKey);
  m_nSessionID = aSessionKey;
  return NS_OK;
}

ConferenceParticipant::ConferenceParticipant(const nsString& handle, const nsString& displayName, const nsString& endpoint, int state)
{
  LOGI("ConferenceParticipant::ConferenceParticipant");
  NS_LossyConvertUTF16toASCII convertName(displayName);
  NS_LossyConvertUTF16toASCII convertHandle(handle);
  if(convertHandle.Length() > 0) {
    LOGI("ConferenceParticipant::ConferenceParticipant::handle - %s", convertHandle.get());
  }
  if(convertName.Length() > 0) {
    LOGI("ConferenceParticipant::ConferenceParticipant::name - %s", convertName.get());
  }
  mHandle = handle;
  mDisplayName = displayName;
  mEndpoint = endpoint;
  mState = state;
}

ConferenceParticipant::~ConferenceParticipant()
{
  LOGI("ConferenceParticipant::~ConferenceParticipant");
}

nsString ConferenceParticipant::getHandle() {
  return mHandle;
}

nsString ConferenceParticipant::getDisplayName() {
  return mDisplayName;
}

nsString ConferenceParticipant::getEndpoint() {
  return mEndpoint;
}

int ConferenceParticipant::getState() {
  return mState;
}
