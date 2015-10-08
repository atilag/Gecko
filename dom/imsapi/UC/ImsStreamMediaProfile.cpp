#undef LOG_TAG
#define LOG_TAG "LGE_IMS_UC_MEDIA_PROFILE"
#include <utils/Log.h>
#include "nsTArray.h"
#include <utils/String16.h>
#include "nsString.h"
#include "ImsStreamMediaProfile.h"
#include "nsILgeImsUCManager.h"

#define LOGI(args...) \
     __android_log_print(ANDROID_LOG_INFO, LOG_TAG, ## args)

#define LOGE(args...) \
     __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, ## args)

ImsStreamMediaProfile::ImsStreamMediaProfile()
{
  LOGI("ImsStreamMediaProfile::ImsStreamMediaProfile");

  mAudioQuality = AUDIO_QUALITY_NONE;
  mAudioDirection = DIRECTION_SEND_RECEIVE;
  mVideoQuality = VIDEO_QUALITY_NONE;
  mVideoDirection = DIRECTION_INVALID;
}
ImsStreamMediaProfile::ImsStreamMediaProfile(const android::Parcel &pParcel)
{
  LOGI("ImsStreamMediaProfile::ImsStreamMediaProfile(Pandroid::Parcel &pParcel)");
  readFromParcel(pParcel);
}

ImsStreamMediaProfile::ImsStreamMediaProfile(int32_t audioQuality, int32_t audioDirection, int32_t videoQuality, int32_t videoDirection)
{
  LOGI("ImsStreamMediaProfile::ImsStreamMediaProfile(Pandroid::Parcel &pParcel)");
  mAudioQuality = audioQuality;
  mAudioDirection = audioDirection;
  mVideoQuality = videoQuality;
  mVideoDirection = videoDirection;
}

ImsStreamMediaProfile::~ImsStreamMediaProfile()
{
  LOGI("ImsStreamMediaProfile::~ImsStreamMediaProfile");
}

void ImsStreamMediaProfile::copyFrom(ImsStreamMediaProfile *profile)
{
  mAudioQuality = profile->mAudioQuality;
  mAudioDirection = profile->mAudioDirection;
  mVideoQuality = profile->mVideoQuality;
  mVideoDirection = profile->mVideoDirection;
}

void ImsStreamMediaProfile::readFromParcel(const android::Parcel &source)
{
  LOGI("ImsStreamMediaProfile::readFromParcel");
  SetAudioQuality(source.readInt32());
  SetAudioDirection(source.readInt32());
  SetVideoQuality(source.readInt32());
  SetVideoDirection(source.readInt32());
  LOGI("ImsStreamMediaProfile::readFromParcel::mAudioQuality - %d, mAudioDirection - %d, mVideoQuality - %d, mVideoDirection - %d",
                                    mAudioQuality, mAudioDirection, mVideoQuality, mVideoDirection);
}

void ImsStreamMediaProfile::writeToParcel(android::Parcel &dest, int flags)
{
  LOGI("ImsStreamMediaProfile::writeToParcel::mAudioQuality - %d, mAudioDirection - %d, mVideoQuality - %d, mVideoDirection - %d",
                                    mAudioQuality, mAudioDirection, mVideoQuality, mVideoDirection);
  dest.writeInt32(mAudioQuality);
  dest.writeInt32(mAudioDirection);
  dest.writeInt32(mVideoQuality);
  dest.writeInt32(mVideoDirection);
}

NS_IMPL_ISUPPORTS(ImsStreamMediaProfile, nsIImsStreamMediaProfile)

//
/*                                 */
NS_IMETHODIMP
ImsStreamMediaProfile::GetAudioQuality(int32_t *aAudioQuality)
{
  LOGI("ImsStreamMediaProfile::GetAudioQuality - %d",  mAudioQuality);
  *aAudioQuality = mAudioQuality;
  return NS_OK;
}

NS_IMETHODIMP
ImsStreamMediaProfile::SetAudioQuality(int32_t aAudioQuality)
{
  LOGI("ImsStreamMediaProfile::SetAudioQuality");
  mAudioQuality = aAudioQuality;
  return NS_OK;
}

/*                                   */
NS_IMETHODIMP
ImsStreamMediaProfile::GetAudioDirection(int32_t *aAudioDirection)
{
  LOGI("ImsStreamMediaProfile::GetAudioDirection - %d", mAudioDirection);
  *aAudioDirection = mAudioDirection;
  return NS_OK;
}
NS_IMETHODIMP
ImsStreamMediaProfile::SetAudioDirection(int32_t aAudioDirection)
{
  LOGI("ImsStreamMediaProfile::SetAudioDirection");
  mAudioDirection = aAudioDirection;
  return NS_OK;
}

/*                                 */
NS_IMETHODIMP
ImsStreamMediaProfile::GetVideoQuality(int32_t *aVideoQuality)
{
  LOGI("ImsStreamMediaProfile::GetVideoQuality - %d",  mVideoQuality);
  *aVideoQuality = mVideoQuality;
  return NS_OK;
}
NS_IMETHODIMP
ImsStreamMediaProfile::SetVideoQuality(int32_t aVideoQuality)
{
  LOGI("ImsStreamMediaProfile::SetVideoQuality");
  mVideoQuality = aVideoQuality;
  return NS_OK;
}

/*                                   */
NS_IMETHODIMP
ImsStreamMediaProfile::GetVideoDirection(int32_t *aVideoDirection)
{
  LOGI("ImsStreamMediaProfile::GetVideoDirection - %d", mVideoDirection);
  *aVideoDirection = mVideoDirection;
  return NS_OK;
}
NS_IMETHODIMP
ImsStreamMediaProfile::SetVideoDirection(int32_t aVideoDirection)
{
  LOGI("ImsStreamMediaProfile::SetVideoDirection");
  mVideoDirection = aVideoDirection;
  return NS_OK;
}
