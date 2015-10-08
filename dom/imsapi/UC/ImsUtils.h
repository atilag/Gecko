#ifndef __IXPCOM_UC_IMS_UTIL_H_
#define __IXPCOM_UC_IMS_UTIL_H_

using namespace android ;

//
class IImsCallSession
{
  public:
    IImsCallSession(){}
    virtual ~IImsCallSession(){}
    static const int32_t close                                = 0;
    static const int32_t getCallId                            = 1;
    static const int32_t getCallProfile                       = 2;
    static const int32_t getLocalCallProfile                  = 3;
    static const int32_t getRemoteCallProfile                 = 4;
    static const int32_t getProperty                          = 5;
    static const int32_t getState                             = 6;
    static const int32_t isInCall                             = 7;
    static const int32_t setListener                          = 8;
    static const int32_t setMute                              = 9;
    static const int32_t start                                = 10;
    static const int32_t startConference                      = 11;
    static const int32_t accept                               = 12;
    static const int32_t deflect                              = 13;
    static const int32_t reject                               = 14;
    static const int32_t terminate                            = 15;
    static const int32_t hold                                 = 16;
    static const int32_t resume                               = 17;
    static const int32_t merge                                = 18;
    static const int32_t update                               = 19;
    static const int32_t extendToConference                   = 20;
    static const int32_t inviteParticipants                   = 21;
    static const int32_t removeParticipants                   = 22;
    static const int32_t sendDtmf                             = 23;
    static const int32_t startDtmf                            = 24;
    static const int32_t stopDtmf                             = 25;
    static const int32_t sendUssd                             = 26;
    static const int32_t getVideoCallProvider                 = 27;
    static const int32_t getCallSubstate                      = 28;
    static const int32_t isMultiparty                         = 29;
};

//
class IImsCallSessionListener
{
  public:
    IImsCallSessionListener(){}
    virtual ~IImsCallSessionListener(){}
    static const int32_t callSessionProgressing                              = 0;
    static const int32_t callSessionStarted                                  = 1;
    static const int32_t callSessionStartFailed                              = 2;
    static const int32_t callSessionTerminated                               = 3;
    static const int32_t callSessionHeld                                     = 4;
    static const int32_t callSessionHoldFailed                               = 5;
    static const int32_t callSessionHoldReceived                             = 6;
    static const int32_t callSessionResumed                                  = 7;
    static const int32_t callSessionResumeFailed                             = 8;
    static const int32_t callSessionResumeReceived                           = 9;
    static const int32_t callSessionMergeStarted                             = 10;
    static const int32_t callSessionMergeComplete                            = 11;
    static const int32_t callSessionUpdated                                  = 12;
    static const int32_t callSessionUpdateFailed                             = 13;
    static const int32_t callSessionUpdateReceived                           = 14;
    static const int32_t callSessionConferenceExtended                       = 15;
    static const int32_t callSessionConferenceExtendFailed                   = 16;
    static const int32_t callSessionConferenceExtendReceived                 = 17;
    static const int32_t callSessionInviteParticipantsRequestDelivered       = 18;
    static const int32_t callSessionInviteParticipantsRequestFailed          = 19;
    static const int32_t callSessionRemoveParticipantsRequestDelivered       = 20;
    static const int32_t callSessionRemoveParticipantsRequestFailed          = 21;
    static const int32_t callSessionConferenceStateUpdated                   = 22;
    static const int32_t callSessionUssdMessageReceived                      = 23;
    static const int32_t callSessionHandover                                 = 24;
    static const int32_t callSessionHandoverFailed                           = 25;
    static const int32_t callSessionTtyModeReceived                          = 26;
    static const int32_t callSessionDeflected                                = 27;
    static const int32_t callSessionDeflectFailed                            = 28;
    static const int32_t callSessionSuppServiceReceived                      = 29;
    static const int32_t callSessionRetryErrorReceived                       = 30;
    static const int32_t callSessionMergeFailed                              = 31;
    static const int32_t callSessionExplicitCallTransferred                  = 32;
    static const int32_t callSessionExplicitCallTransferFailed               = 33;
};

#endif //