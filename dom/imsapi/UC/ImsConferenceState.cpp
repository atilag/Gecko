#undef LOG_TAG
#define LOG_TAG "LGE_IMS_CONF_STATE"
#include <utils/Log.h>
#include "ImsConferenceState.h"

#define LOGI(args...) \
     __android_log_print(ANDROID_LOG_INFO, LOG_TAG, ## args)

#define LOGE(args...) \
     __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, ## args)

NS_IMPL_ISUPPORTS(ImsConferenceState, nsIImsConferenceState)

ImsConferenceState::ImsConferenceState()
{
  LOGI("ImsConferenceState::ImsConferenceState");

  USER.AssignLiteral("user");
  DISPLAY_TEXT.AssignLiteral("display-text");
  ENDPOINT.AssignLiteral("endpoint");
  STATUS.AssignLiteral("status");

  STATUS_PENDING.AssignLiteral("pending");
  STATUS_DIALING_OUT.AssignLiteral("dialing-out");
  STATUS_DIALING_IN.AssignLiteral("dialing-in");
  STATUS_ALERTING.AssignLiteral("alerting");
  STATUS_ON_HOLD.AssignLiteral("on-hold");
  STATUS_CONNECTED.AssignLiteral("connected");
  STATUS_DISCONNECTING.AssignLiteral("disconnecting");
  STATUS_DISCONNECTED.AssignLiteral("disconnected");
  STATUS_MUTED_VIA_FOCUS.AssignLiteral("muted-via-focus");
  STATUS_CONNECT_FAIL.AssignLiteral("connect-fail");

  SIP_STATUS_CODE.AssignLiteral("sipstatuscode");
}

ImsConferenceState::ImsConferenceState(const android::Parcel &pParcel)
{
  LOGI("ImsConferenceState::ImsConferenceState(android::Parcel &pParcel)");
  USER.AssignLiteral("user");
  DISPLAY_TEXT.AssignLiteral("display-text");
  ENDPOINT.AssignLiteral("endpoint");
  STATUS.AssignLiteral("status");

  STATUS_PENDING.AssignLiteral("pending");
  STATUS_DIALING_OUT.AssignLiteral("dialing-out");
  STATUS_DIALING_IN.AssignLiteral("dialing-in");
  STATUS_ALERTING.AssignLiteral("alerting");
  STATUS_ON_HOLD.AssignLiteral("on-hold");
  STATUS_CONNECTED.AssignLiteral("connected");
  STATUS_DISCONNECTING.AssignLiteral("disconnecting");
  STATUS_DISCONNECTED.AssignLiteral("disconnected");
  STATUS_MUTED_VIA_FOCUS.AssignLiteral("muted-via-focus");
  STATUS_CONNECT_FAIL.AssignLiteral("connect-fail");

  SIP_STATUS_CODE.AssignLiteral("sipstatuscode");
  readFromParcel(pParcel);
}

ImsConferenceState::~ImsConferenceState()
{
  LOGI("ImsConferenceState::~ImsConferenceState");
  mParticipants.clear();
}

int ImsConferenceState::describeContents()
{
  return 0;
}

void ImsConferenceState::readFromParcel(const android::Parcel &source)
{
  LOGI("ImsConferenceState::readFromParcel");
  int size = source.readInt32();
  LOGI("ImsConferenceState::readFromParcel::size - %d", size);
  nsString user;
  for(int index = 0; index < size; index++) {
    user = source.readString16();
    Bundle *pBundle = new Bundle(source);
    mParticipants.insert ( std::pair<nsString, Bundle *>(user,pBundle) );
  }
}

void ImsConferenceState::writeToParcel(android::Parcel &dest, int flags)
{
  LOGI("ImsConferenceState::writeToParcel");
  dest.writeInt32(mParticipants.size());
  std::map<nsString, Bundle *>::iterator it;
  for (it = mParticipants.begin(); it != mParticipants.end(); ++it ) {
    nsString user = it->first;
    NS_LossyConvertUTF16toASCII convertUser(user);
    if(convertUser.Length() > 0)
      dest.writeString16(android::String16(convertUser.get()));
    Bundle *pBundle = it->second;
    pBundle->writeToParcel(dest, 0);
  }
}

int ImsConferenceState::getConnectionStateForStatus(const nsString& status) {
  LOGI("ImsConferenceState::getConnectionStateForStatus");
  NS_LossyConvertUTF16toASCII convertStatus(status);
  if(convertStatus.Length() > 0) {
    LOGI("ImsConferenceState::getConnectionStateForStatus::callee - %s", convertStatus.get());
  } else {
    return nsIImsConferenceState::STATE_ACTIVE;
  }
  if (status.Equals(STATUS_PENDING)) {
    return nsIImsConferenceState::STATE_INITIALIZING;
  } else if (status.Equals(STATUS_DIALING_IN)) {
    return nsIImsConferenceState::STATE_RINGING;
  } else if (status.Equals(STATUS_ALERTING) ||
        status.Equals(STATUS_DIALING_OUT)) {
    return nsIImsConferenceState::STATE_DIALING;
  } else if (status.Equals(STATUS_ON_HOLD)) {
    return nsIImsConferenceState::STATE_HOLDING;
  } else if (status.Equals(STATUS_CONNECTED) ||
        status.Equals(STATUS_MUTED_VIA_FOCUS) ||
        status.Equals(STATUS_DISCONNECTING)) {
    return nsIImsConferenceState::STATE_ACTIVE;
  } else if (status.Equals(STATUS_DISCONNECTED)) {
    return nsIImsConferenceState::STATE_DISCONNECTED;
  }
  return nsIImsConferenceState::STATE_ACTIVE;
}

//
NS_IMETHODIMP
ImsConferenceState::GetConfState(std::map<nsString, int32_t> & aState)
{
  LOGE("ImsCallSession::GetConfState");
  aState = Confstate;
  return NS_OK;
}

NS_IMETHODIMP
ImsConferenceState::SetConfState(std::map<nsString, int32_t> & aState)
{
  LOGE("ImsCallSession::SetConfState");
  Confstate = aState;
  return NS_OK;
}
