#undef LOG_TAG
#define LOG_TAG "LGE_IMS_UC_CALL_PROFILE"
#include <utils/Log.h>
#include "ImsCallProfile.h"
#include "Bundle.h"
#include "ImsStreamMediaProfile.h"

#define LOGI(args...) \
     __android_log_print(ANDROID_LOG_INFO, LOG_TAG, ## args)

#define LOGE(args...) \
     __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, ## args)
using namespace android ;

nsString ImsCallProfile::EXTRA_HD_VOICE;
nsString ImsCallProfile::EXTRA_OI;
nsString ImsCallProfile::EXTRA_CNA;
nsString ImsCallProfile::EXTRA_REMOTE_URI;

ImsCallProfile::ImsCallProfile(const android::Parcel &pParcel)
{
  LOGI("ImsCallProfile::ImsCallProfile(Pandroid::Parcel &pParcel)");

  EXTRA_CONFERENCE.AssignLiteral("conference");
  EXTRA_E_CALL.AssignLiteral("e_call");
  EXTRA_VMS.AssignLiteral("vms");
  EXTRA_CALL_MODE_CHANGEABLE.AssignLiteral("call_mode_changeable");
  EXTRA_CONFERENCE_AVAIL.AssignLiteral("conference_avail");

  EXTRA_OIR.AssignLiteral("oir");
  EXTRA_CNAP.AssignLiteral("cnap");
  EXTRA_DIALSTRING.AssignLiteral("dialstring");
  EXTRA_CALL_DOMAIN.AssignLiteral("call_domain");

  EXTRA_OI.AssignLiteral("oi");
  EXTRA_CNA.AssignLiteral("cna");
  EXTRA_USSD.AssignLiteral("ussd");
  EXTRA_REMOTE_URI.AssignLiteral("remote_uri");
  EXTRA_PARENT_CALL_ID.AssignLiteral("parentCallId");
  EXTRA_CHILD_NUMBER.AssignLiteral("ChildNum");
  EXTRA_CODEC.AssignLiteral("Codec");
  EXTRA_DISPLAY_TEXT.AssignLiteral("DisplayText");
  EXTRA_ADDITIONAL_CALL_INFO.AssignLiteral("AdditionalCallInfo");
  EXTRA_OEM_EXTRAS.AssignLiteral("OemCallExtras");
  EXTRA_REDIREC_NUMBER.AssignLiteral("RedirectNum");
  EXTRA_MMC.AssignLiteral("mmc");
  EXTRA_GTT.AssignLiteral("gtt");
  EXTRA_CONFERENCE_EVENT.AssignLiteral("conference_event");
  EXTRA_HD_VOICE.AssignLiteral("hd_voice");
  readFromParcel(pParcel);
}

ImsCallProfile::ImsCallProfile()
{
  LOGI("ImsCallProfile::ImsCallProfile");
  EXTRA_CONFERENCE.AssignLiteral("conference");
  EXTRA_E_CALL.AssignLiteral("e_call");
  EXTRA_VMS.AssignLiteral("vms");
  EXTRA_CALL_MODE_CHANGEABLE.AssignLiteral("call_mode_changeable");
  EXTRA_CONFERENCE_AVAIL.AssignLiteral("conference_avail");
  EXTRA_MMC.AssignLiteral("mmc");
  EXTRA_GTT.AssignLiteral("gtt");
  EXTRA_HD_VOICE.AssignLiteral("hd_voice");
  EXTRA_CONFERENCE_EVENT.AssignLiteral("conference_event");

  EXTRA_OIR.AssignLiteral("oir");
  EXTRA_CNAP.AssignLiteral("cnap");
  EXTRA_DIALSTRING.AssignLiteral("dialstring");
  EXTRA_CALL_DOMAIN.AssignLiteral("call_domain");

  EXTRA_OI.AssignLiteral("oi");
  EXTRA_CNA.AssignLiteral("cna");
  EXTRA_USSD.AssignLiteral("ussd");
  EXTRA_REMOTE_URI.AssignLiteral("remote_uri");
  EXTRA_PARENT_CALL_ID.AssignLiteral("parentCallId");
  EXTRA_CHILD_NUMBER.AssignLiteral("ChildNum");
  EXTRA_CODEC.AssignLiteral("Codec");
  EXTRA_DISPLAY_TEXT.AssignLiteral("DisplayText");
  EXTRA_ADDITIONAL_CALL_INFO.AssignLiteral("AdditionalCallInfo");
  EXTRA_OEM_EXTRAS.AssignLiteral("OemCallExtras");
  EXTRA_REDIREC_NUMBER.AssignLiteral("RedirectNum");

  mServiceType = SERVICE_TYPE_NORMAL;
  mCallType = CALL_TYPE_VOICE_N_VIDEO;
  mCallExtras = new Bundle();
  mMediaProfile = new ImsStreamMediaProfile();
}

ImsCallProfile::ImsCallProfile(int32_t serviceType, int32_t callType)
{
  EXTRA_CONFERENCE.AssignLiteral("conference");
  EXTRA_E_CALL.AssignLiteral("e_call");
  EXTRA_VMS.AssignLiteral("vms");
  EXTRA_CALL_MODE_CHANGEABLE.AssignLiteral("call_mode_changeable");
  EXTRA_CONFERENCE_AVAIL.AssignLiteral("conference_avail");
  EXTRA_MMC.AssignLiteral("mmc");
  EXTRA_GTT.AssignLiteral("gtt");
  EXTRA_HD_VOICE.AssignLiteral("hd_voice");
  EXTRA_CONFERENCE_EVENT.AssignLiteral("conference_event");

  EXTRA_OIR.AssignLiteral("oir");
  EXTRA_CNAP.AssignLiteral("cnap");
  EXTRA_DIALSTRING.AssignLiteral("dialstring");
  EXTRA_CALL_DOMAIN.AssignLiteral("call_domain");

  EXTRA_OI.AssignLiteral("oi");
  EXTRA_CNA.AssignLiteral("cna");
  EXTRA_USSD.AssignLiteral("ussd");
  EXTRA_REMOTE_URI.AssignLiteral("remote_uri");
  EXTRA_PARENT_CALL_ID.AssignLiteral("parentCallId");
  EXTRA_CHILD_NUMBER.AssignLiteral("ChildNum");
  EXTRA_CODEC.AssignLiteral("Codec");
  EXTRA_DISPLAY_TEXT.AssignLiteral("DisplayText");
  EXTRA_ADDITIONAL_CALL_INFO.AssignLiteral("AdditionalCallInfo");
  EXTRA_OEM_EXTRAS.AssignLiteral("OemCallExtras");
  EXTRA_REDIREC_NUMBER.AssignLiteral("RedirectNum");

  mServiceType = serviceType;
  mCallType = callType;
  mCallExtras = new Bundle();
  mMediaProfile = new ImsStreamMediaProfile(ImsStreamMediaProfile::AUDIO_QUALITY_AMR_WB, ImsStreamMediaProfile::DIRECTION_SEND_RECEIVE,
                        ImsStreamMediaProfile::VIDEO_QUALITY_NONE, ImsStreamMediaProfile::DIRECTION_INVALID);
}

ImsCallProfile::~ImsCallProfile()
{
}

nsString ImsCallProfile::getCallExtra(nsString& name)
{
  nsString empty;
  return getCallExtra(name, empty);
}

nsString ImsCallProfile::getCallExtra(nsString& name, nsString& defaultValue)
{
  if (mCallExtras == nullptr) {
    return defaultValue;
  }
  return mCallExtras->getString(name, defaultValue);
}

bool ImsCallProfile::getCallExtraBoolean(nsString& name)
{
  return getCallExtraBoolean(name, false);
}

bool ImsCallProfile::getCallExtraBoolean(nsString& name, bool defaultValue)
{
  if (mCallExtras == nullptr) {
    return defaultValue;
  }
  return mCallExtras->getBoolean(name, defaultValue);
}

int32_t ImsCallProfile::getCallExtraInt(nsString& name)
{
  return getCallExtraInt(name, -1);
}

int32_t ImsCallProfile::getCallExtraInt(nsString& name, int32_t defaultValue)
{
  if (mCallExtras == nullptr) {
    return defaultValue;
  }
  return mCallExtras->getInt(name, defaultValue);
}

void ImsCallProfile::setCallExtra(nsString& name, nsString& value)
{
  if (mCallExtras != nullptr) {
    mCallExtras->putString(name, value);
  }
}

void ImsCallProfile::setCallExtraBoolean(nsString& name, bool value)
{
  if (mCallExtras != nullptr) {
    mCallExtras->putBoolean(name, value);
  }
}

void ImsCallProfile::setCallExtraInt(nsString& name, int32_t value)
{
  if (mCallExtras != nullptr) {
    mCallExtras->putInt(name, value);
  }
}

void ImsCallProfile::updateCallType(ImsCallProfile *profile)
{
  mCallType = profile->mCallType;
}

void ImsCallProfile::updateCallExtras(ImsCallProfile *profile)
{
  mCallExtras->clear();
  mCallExtras->clone(profile->mCallExtras);
}

void ImsCallProfile::writeToParcel(android::Parcel &dest, int32_t flags)
{
  LOGI("ImsCallProfile::writeToParcel mServiceType - %d, mCallType - %d", mServiceType, mCallType);
  dest.writeInt32(mServiceType);
  dest.writeInt32(mCallType);
  mCallExtras->writeToParcel(dest, 0);
  mMediaProfile->writeToParcel(dest, 0);
}

void ImsCallProfile::readFromParcel(const android::Parcel &source)
{
  LOGI("ImsCallProfile::readFromParcel");
  mServiceType = source.readInt32();
  mCallType = source.readInt32();
  LOGI("ImsCallProfile::readFromParcel mServiceType - %d, mCallType - %d", mServiceType, mCallType);
  mCallExtras = new Bundle(source);
  mMediaProfile = new ImsStreamMediaProfile(source);
}

void ImsCallProfile::copyFrom(ImsCallProfile *pCallProfile)
{
  LOGI("ImsCallProfile::copyFrom");
  mServiceType = pCallProfile->mServiceType;
  mCallType = pCallProfile->mCallType;
  mCallExtras->clone(pCallProfile->mCallExtras);
  mMediaProfile->copyFrom(pCallProfile->mMediaProfile);
}

/*





 */
int32_t ImsCallProfile::getVideoStateFromImsCallProfile(ImsCallProfile *callProfile)
{
#if 0
  int32_t videostate = VideoProfile.VideoState.AUDIO_ONLY;
  switch (callProfile.mCallType) {
    case CALL_TYPE_VT_TX:
      videostate = VideoProfile.VideoState.TX_ENABLED;
    break;
    case CALL_TYPE_VT_RX:
      videostate = VideoProfile.VideoState.RX_ENABLED;
    break;
    case CALL_TYPE_VT:
      videostate = VideoProfile.VideoState.BIDIRECTIONAL;
    break;
    case CALL_TYPE_VOICE:
      videostate = VideoProfile.VideoState.AUDIO_ONLY;
    break;
    default:
      videostate = VideoProfile.VideoState.AUDIO_ONLY;
    break;
  }
  if (callProfile.isVideoPaused() && videostate != VideoProfile.VideoState.AUDIO_ONLY) {
    videostate |= VideoProfile.VideoState.PAUSED;
  } else {
    videostate &= ~VideoProfile.VideoState.PAUSED;
  }
  return videostate;
#endif
  return -1;
}

/*





*/
int32_t ImsCallProfile::getVideoStateFromCallType(int32_t callType)
{
#if 0
  switch (callType) {
    case CALL_TYPE_VT_NODIR:
      return VideoProfile.VideoState.PAUSED | VideoProfile.VideoState.BIDIRECTIONAL;
    case CALL_TYPE_VT_TX:
      return VideoProfile.VideoState.TX_ENABLED;
    case CALL_TYPE_VT_RX:
      return VideoProfile.VideoState.RX_ENABLED;
    case CALL_TYPE_VT:
      return VideoProfile.VideoState.BIDIRECTIONAL;
    case CALL_TYPE_VOICE:
      return VideoProfile.VideoState.AUDIO_ONLY;
    default:
      return VideoProfile.VideoState.AUDIO_ONLY;
  }
#endif
  return -1;
}

/*





*/
int32_t ImsCallProfile::getCallTypeFromVideoState(int32_t videoState)
{
#if 0
  bool videoTx = isVideoStateSet(videoState, VideoProfile.VideoState.TX_ENABLED);
  bool videoRx = isVideoStateSet(videoState, VideoProfile.VideoState.RX_ENABLED);
  bool isPaused = isVideoStateSet(videoState, VideoProfile.VideoState.PAUSED);
  if (isPaused) {
    return ImsCallProfile::CALL_TYPE_VT_NODIR;
  } else if (videoTx && !videoRx) {
    return ImsCallProfile::CALL_TYPE_VT_TX;
  } else if (!videoTx && videoRx) {
    return ImsCallProfile::CALL_TYPE_VT_RX;
  } else if (videoTx && videoRx) {
    return ImsCallProfile::CALL_TYPE_VT;
  }
#endif
  return ImsCallProfile::CALL_TYPE_VOICE;
}

/*



*/
int32_t ImsCallProfile::presentationToOIR(int32_t presentation)
{
#if 0
  switch (presentation) {
    case PhoneConstants::PRESENTATION_RESTRICTED:
      return ImsCallProfile::OIR_PRESENTATION_RESTRICTED;
    case PhoneConstants::PRESENTATION_ALLOWED:
      return ImsCallProfile::OIR_PRESENTATION_NOT_RESTRICTED;
    case PhoneConstants::PRESENTATION_PAYPHONE:
      return ImsCallProfile::OIR_PRESENTATION_PAYPHONE;
    case PhoneConstants.PRESENTATION_UNKNOWN:
      return ImsCallProfile::OIR_PRESENTATION_UNKNOWN;
    default:
      return ImsCallProfile::OIR_DEFAULT;
  }
#endif
  return ImsCallProfile::OIR_DEFAULT;
}

/*



*/
int32_t ImsCallProfile::OIRToPresentation(int32_t oir)
{
#if 0
  switch(oir) {
    case ImsCallProfile::OIR_PRESENTATION_RESTRICTED:
      return PhoneConstants::PRESENTATION_RESTRICTED;
    case ImsCallProfile::OIR_PRESENTATION_NOT_RESTRICTED:
      return PhoneConstants::PRESENTATION_ALLOWED;
    case ImsCallProfile::OIR_PRESENTATION_PAYPHONE:
      return PhoneConstants::PRESENTATION_PAYPHONE;
    case ImsCallProfile::OIR_PRESENTATION_UNKNOWN:
      return PhoneConstants::PRESENTATION_UNKNOWN;
    default:
      return PhoneConstants::PRESENTATION_UNKNOWN;
  }
#endif
  return -1;
}

/*


*/
bool ImsCallProfile::isVideoPaused()
{
  return mMediaProfile->mVideoDirection == ImsStreamMediaProfile::DIRECTION_INACTIVE;
}

/*





*/
bool ImsCallProfile::isVideoStateSet(int32_t videoState, int32_t videoStateToCheck)
{
  return (videoState & videoStateToCheck) == videoStateToCheck;
}
