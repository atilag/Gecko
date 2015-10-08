#ifndef __IXPCOM_UC_IMS_CALL_H_
#define __IXPCOM_UC_IMS_CALL_H_
#pragma once
#include <binder/IInterface.h>
#include "IXPCOMCoreInterface.h"
#include "UCInterface.h"
#include "nsString.h"
#include "nsILgeImsUCManager.h"
#include "nsThreadUtils.h"
#include "nsComponentManagerUtils.h"
#include "nsITimer.h"
#include "ImsCallSession.h"
#include "ImsStreamMediaProfile.h"
#include "ImsCallProfile.h"
#include "ICall.h"
#include "Bundle.h"

using namespace android ;
class ConferenceParticipant
{
  public:
    /*

    */
    nsString mHandle;
    /*

    */
    nsString mDisplayName;
    /*


    */
    nsString mEndpoint;
    /*



    */
    int mState = 0;
    ConferenceParticipant(const nsString& handle, const nsString& displayName, const nsString& endpoint, int state);
    virtual ~ConferenceParticipant();
    nsString getHandle();
    nsString getDisplayName();
    nsString getEndpoint();
    int getState();
};

class ImsCall: public nsIImsCall, public ICall
{
  public:
    NS_DECL_ISUPPORTS
    NS_DECL_NSIIMSCALL
    ImsCall(ImsCallProfile *profile);
    virtual ~ImsCall();

    nsIImsCallListener* m_pIAUCSessionListener;
    void setListener(nsIImsCallListener *pIAUCSessionListener);
    ImsCall *mImsCall = nullptr;
    ImsCall *ConferenceHost = nullptr;

    void callSessionProgressing(ImsCallSession *session, ImsStreamMediaProfile *profile);
    void callSessionStarted(ImsCallSession *session, ImsCallProfile *profile);
    void callSessionStartFailed(ImsCallSession *session, ImsReasonInfo *reasonInfo);
    void callSessionTerminated(ImsCallSession *session, ImsReasonInfo *reasonInfo);
    void callSessionHeld(ImsCallSession *session, ImsCallProfile *profile);
    void callSessionHoldFailed(ImsCallSession *session, ImsReasonInfo *reasonInfo);
    void callSessionHoldReceived(ImsCallSession *session, ImsCallProfile *profile);
    void callSessionResumed(ImsCallSession *session, ImsCallProfile *profile);
    void callSessionResumeFailed(ImsCallSession *session, ImsReasonInfo *reasonInfo);
    void callSessionResumeReceived(ImsCallSession *session, ImsCallProfile *profile);
    void callSessionMergeStarted(ImsCallSession *session, ImsCallSession *newSession, ImsCallProfile *profile);
    void callSessionMergeComplete(ImsCallSession *session);
    void callSessionMergeFailed(ImsCallSession *session, ImsReasonInfo *reasonInfo);
    void callSessionUpdated(ImsCallSession *session, ImsCallProfile *profile);
    void callSessionUpdateFailed(ImsCallSession *session, ImsReasonInfo *reasonInfo);
    void callSessionUpdateReceived(ImsCallSession *session, ImsCallProfile *profile);
    void callSessionConferenceExtended(ImsCallSession *session, ImsCallSession *newSession, ImsCallProfile *profile);
    void callSessionConferenceExtendFailed(ImsCallSession *session, ImsReasonInfo *reasonInfo);
    void callSessionConferenceExtendReceived(ImsCallSession *session, ImsCallSession *newSession, ImsCallProfile *profile);
    void callSessionInviteParticipantsRequestDelivered(ImsCallSession *session);
    void callSessionInviteParticipantsRequestFailed(ImsCallSession *session, ImsReasonInfo *reasonInfo);
    void callSessionRemoveParticipantsRequestDelivered(ImsCallSession *session);
    void callSessionRemoveParticipantsRequestFailed(ImsCallSession *session, ImsReasonInfo *reasonInfo);
    void callSessionConferenceStateUpdated(ImsCallSession *session, ImsConferenceState *state);
    void callSessionUssdMessageReceived(ImsCallSession *session, int32_t mode,nsString ussdMessage);
    void callSessionTtyModeReceived(ImsCallSession *session, int32_t mode);
    void callSessionHandover(ImsCallSession *session, int32_t srcAccessTech, int32_t targetAccessTech, ImsReasonInfo *reasonInfo);
    void callSessionHandoverFailed(ImsCallSession *session, int32_t srcAccessTech, int32_t targetAccessTech, ImsReasonInfo *reasonInfo);
    void callSessionSuppServiceReceived(ImsCallSession *session);
    void callSessionRetryErrorReceived(ImsCallSession *session, ImsReasonInfo *reasonInfo);
    void callSessionExplicitCallTransferred(ImsCallSession *session);
    void callSessionExplicitCallTransferFailed(ImsCallSession *session, ImsReasonInfo *reasonInfo);

    virtual void close();
    void updateHoldValues();
    virtual bool checkIfRemoteUserIsSame(const nsString& userId);
    virtual bool equalsTo(ICall *call);
    bool isSessionAlive(ImsCallSession *session);
    ImsCallProfile *getCallProfile();
    ImsCallProfile *getLocalCallProfile();
    ImsCallProfile *getRemoteCallProfile();
    ImsCallProfile *getProposedCallProfile();
    nsTArray<ConferenceParticipant *> &getConferenceParticipants();
    ImsCallSession *getCallSession();
    ImsConferenceState *getConferenceState();
    //
    nsString getCallExtra(const nsString& name);
    int32_t getCallSubstate();
    ImsReasonInfo *getLastReasonInfo();
    bool hasPendingUpdate();
    bool isMuted();
    bool isOnHold();
    bool isMultiparty();
    void setIsMerged(bool isMerged);
    bool isMerged();
    void attachSession(ImsCallSession *session);
    void start(ImsCallSession *session, const nsString& callee);
    void start(ImsCallSession *session, nsTArray<nsString> &participants);
    void accept(int32_t callType, ImsStreamMediaProfile *profile);
    void deflect(const nsString& number);
    void merge();
    void startDtmf(char *c);
    void stopDtmf();
    void explicitCallTransfer();
    void clear(ImsReasonInfo *lastReasonInfo);
    ImsCall *createCallSessionListener();
    ImsCall *createNewCall(ImsCallSession *session, ImsCallProfile *profile);
    ImsStreamMediaProfile *createHoldMediaProfile();
    ImsStreamMediaProfile *createResumeMediaProfile();
    void enforceConversationMode();
    void mergeInternal();
    void notifyConferenceSessionTerminated(ImsReasonInfo *reasonInfo);
    void processCallTerminated(ImsReasonInfo *reasonInfo);
    bool isTransientConferenceSession(ImsCallSession *session);
    void setTransientSessionAsPrimary(ImsCallSession *transientSession);
    void tryProcessConferenceResult();
    void processMergeComplete();
    void notifySessionTerminatedDuringMerge();
    void clearSessionTerminationFlags();
    void processMergeFailed(ImsReasonInfo *reasonInfo);
    void notifyError(int32_t reason, int32_t statusCode, const nsString& message);
    void updateConferenceParticipantsList(ImsCall *bgCall);
    void addToConferenceParticipantList(ImsCall *childCall);
    void clearMergeInfo();
    bool isInCall();
    void setMute(bool muted);
    char* updateRequestToString(int updateRequest);
    void setCallSessionMergePending(bool callSessionMergePending);
       void    setMergePeer(ImsCall* mergePeer);
    void setMergeHost(ImsCall* mergeHost);
    bool isMerging();
    bool isMergeHost();
    bool isMergePeer();
    bool isCallSessionMergePending();
    bool shouldProcessConferenceResult();
    void conferenceStateUpdated(ImsConferenceState* state);
    void notifyConferenceStateUpdated(ImsConferenceState *state);
    void SendOnCallTermate(ImsReasonInfo* reasonInfo);
    void CallOnCallTermate(ImsReasonInfo* reasonInfo);


    int32_t mState = nsIImsCall::IDLE;
    //
    nsString  sCalleePartyNum;
    nsString  sCallerPartyNum;
    int32_t sessionType = nsIImsCallProfile::CALL_TYPE_VOICE;
    int32_t serviceType = nsIImsCallProfile::SERVICE_TYPE_NONE;
    bool isConf = false;
    int32_t OIPType = nsIImsCallProfile::OIPTYPE_NONE;
    int32_t m_HDIcon = 0;
    int64_t m_nSessionID;

  private:
    nsTArray<ConferenceParticipant *> mConferenceParticipants;
    //
    static const int32_t UPDATE_NONE = 0;
    static const int32_t UPDATE_HOLD = 1;
    static const int32_t UPDATE_HOLD_MERGE = 2;
    static const int32_t UPDATE_RESUME = 3;
    static const int32_t UPDATE_MERGE = 4;
    static const int32_t UPDATE_EXTEND_TO_CONFERENCE = 5;
    static const int32_t UPDATE_UNSPECIFIED = 6;

    //
    bool mInCall = false;
    //
    //
    //
    bool mHold = false;
    //
    bool mMute = false;
    //
    int32_t mUpdateRequest = UPDATE_NONE;
    //
    ImsCall *mMergePeer = nullptr;
    //
    ImsCall *mMergeHost = nullptr;

    //
    ImsCallSession *mSession = nullptr;
    //
    //
    ImsCallProfile *mCallProfile = nullptr;
    //
    //
    //
    ImsCallProfile *mProposedCallProfile = nullptr;
    ImsReasonInfo *mLastReasonInfo = nullptr;

    //
    //

    //
    ImsConferenceState *mConfState = nullptr;
    //

    //
    //
    ImsCallSession *mTransientConferenceSession = nullptr;
    //
    //
    //
    //
    //
    bool mSessionEndDuringMerge = false;
    //
    //
    //
    ImsReasonInfo *mSessionEndDuringMergeReasonInfo = nullptr;
    //
    //
    //
    bool mIsMerged = false;
    //
    //
    //
    //
    //
    bool mCallSessionMergePending = false;
    bool mIsCEPPresent = false;
};


#endif //
