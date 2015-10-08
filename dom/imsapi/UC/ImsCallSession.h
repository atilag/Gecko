/*








                                                                   */
#ifndef __IXPCOM_UC_IMS_CALL_SESSION_H_
#define __IXPCOM_UC_IMS_CALL_SESSION_H_
#pragma once
#include <binder/IInterface.h>
#include "IXPCOMCoreInterface.h"
#include "UCInterface.h"
#include "nsString.h"
#include "nsThreadUtils.h"
#include "nsComponentManagerUtils.h"
#include "nsITimer.h"
#include "ImsReasonInfo.h"
#include "ImsStreamMediaProfile.h"
#include "ImsCallProfile.h"
#include "ImsUtils.h"
#include <binder/IInterface.h>
#include "IXPCOMCoreInterface.h"
#include "ImsConferenceState.h"

using namespace android ;
class State
{
  public:
    static const int32_t IDLE = 0;
    static const int32_t INITIATED = 1;
    static const int32_t NEGOTIATING = 2;
    static const int32_t ESTABLISHING = 3;
    static const int32_t ESTABLISHED = 4;

    static const int32_t RENEGOTIATING = 5;
    static const int32_t REESTABLISHING = 6;

    static const int32_t TERMINATING = 7;
    static const int32_t TERMINATED = 8;

    static const int32_t INVALID = (-1);
};

class ImsCall;
class ImsCallSession: public BnUCInterface
{
  private:
    int m_nCallID;
    bool bIsTerminated = false;
    nsCOMPtr<nsIThread> xpcomUCSessionThread;
    //
    sp<IXPCOMCoreInterface> pXPCOMCoreInterfaceObj = nullptr;

  public:
    ImsCallSession(int64_t SessionID);
    ImsCallSession();
    ~ImsCallSession();
    friend class ImsCall;
    ImsCall *pImsCall;
    void binderDied(const wp<IBinder>&);
    ImsStreamMediaProfile *pMediaProfile = nullptr;
    ImsCallProfile *profile = nullptr;
    ImsReasonInfo *reasonInfo = nullptr;
    ImsConferenceState *mstate = nullptr;
    int64_t m_nSessionID = -1;

    int getCallID();
    void close();
    void close(int seconds);
    ImsCallProfile *getCallProfile();
    ImsCallProfile *getLocalCallProfile();
    ImsCallProfile *getRemoteCallProfile();
    int32_t getCallSubstate();
    int32_t getState();
    bool isAlive();
    bool isInCall();
    void setMute(bool muted);

    /*                                                                             */
    void start(const nsString& callee, ImsCallProfile *profile);
    void start(nsTArray<nsString> &participants, ImsCallProfile *profile);
    void accept(int32_t callType, ImsStreamMediaProfile *profile);
    void deflect(const nsString& number);
    void reject(int32_t reason);
    void terminate(int32_t reason);
    void hold(ImsStreamMediaProfile *profile);
    void resume(ImsStreamMediaProfile *profile);
    void merge();
    void update(int32_t callType, ImsStreamMediaProfile *profile);
    void extendToConference(nsTArray<nsString> &participants);
    void inviteParticipants(nsTArray<nsString> &participants);
    void removeParticipants(nsTArray<nsString> &participants);
    void SendDtmf(const nsAString & c);
    void startDtmf(char *c);
    void stopDtmf();
    void sendUssd(const nsAString& ussdMessage);
    bool isMultiparty();
    void explicitCallTransfer();
    //
    nsString getProperty(const nsString& name);
    void setListener(ImsCall *listener);

    /*                                                                             */
    virtual void onMessage(const android::Parcel &pParcel);
    virtual void onMessageEx(const android::Parcel &pParcel,  Parcel *pOutParcel);
    void onMessageInternal(const android::Parcel& pParcel);
    void registrationConnected();
    void registrationDisconnected(const android::Parcel& pParcel);
    void notifyIncomingCall(int phoneID, int serviceID, const char* callId);
    void enteredECBM();
    void exitedECBM();
    void set911State(int32_t state, int32_t ecbm);
    bool isSessionAlive = true;
};
#endif //
