#undef LOG_TAG
#define LOG_TAG "LGE_IMS_UC_REASON_INFO"
#include <utils/Log.h>
#include "ImsReasonInfo.h"
#include "nsILgeImsUCManager.h"

#define LOGI(args...) \
     __android_log_print(ANDROID_LOG_INFO, LOG_TAG, ## args)

#define LOGE(args...) \
     __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, ## args)

NS_IMPL_ISUPPORTS(ImsReasonInfo, nsIImsReasonInfo)

ImsReasonInfo::ImsReasonInfo()
{
  LOGI("ImsReasonInfo::ImsReasonInfo");
}
ImsReasonInfo::ImsReasonInfo(const android::Parcel &pParcel)
{
  LOGI("ImsReasonInfo::ImsReasonInfo(android::Parcel &pParcel)");
  readFromParcel(pParcel);
}

ImsReasonInfo::ImsReasonInfo(int32_t code, int32_t extraCode)
{
  mReasonType = (int32_t) (code / 100);
  mCode = code;
  mExtraCode = extraCode;
}

ImsReasonInfo::ImsReasonInfo(int32_t code, int32_t extraCode, nsString extraMessage)
{
  mReasonType = (int32_t) (code / 100);
  mCode = code;
  mExtraCode = extraCode;
  mExtraMessage = extraMessage;
}

ImsReasonInfo::~ImsReasonInfo()
{
  LOGI("ImsReasonInfo::~ImsReasonInfo");
}

NS_IMETHODIMP
ImsReasonInfo::GetReasonType(int32_t *aReasonType)
{
  LOGI("ImsReasonInfo::GetReasonType - %d", mReasonType);
  *aReasonType = mReasonType;
  return NS_OK;
}

NS_IMETHODIMP
ImsReasonInfo::SetReasonType(int32_t aReasonType)
{
  LOGE("ImsReasonInfo::SetReasonType - %d", aReasonType);
  mReasonType = aReasonType;
  return NS_OK;
}

//
/*                               */
NS_IMETHODIMP
ImsReasonInfo::GetReasonCode(int32_t *aReasonCode)
{
  LOGE("ImsReasonInfo::GetReasonCode - %d", mCode);
  *aReasonCode = mCode;
  return NS_OK;
}
NS_IMETHODIMP
ImsReasonInfo::SetReasonCode(int32_t aReasonCode)
{
  LOGE("ImsReasonInfo::SetReasonCode - %d", aReasonCode);
  mCode = aReasonCode;
  return NS_OK;
}

/*                              */
NS_IMETHODIMP
ImsReasonInfo::GetExtraCode(int32_t *aExtraCode)
{
  LOGE("ImsReasonInfo::GetExtraCode - %d", mExtraCode);
  *aExtraCode = mExtraCode;
  return NS_OK;
}

NS_IMETHODIMP
ImsReasonInfo::SetExtraCode(int32_t aExtraCode)
{
  LOGE("ImsReasonInfo::SetExtraCode - %d",  aExtraCode);
  mExtraCode = aExtraCode;
  return NS_OK;
}

/*                                   */
NS_IMETHODIMP
ImsReasonInfo::GetExtraMessage(nsAString & aExtraMessage)
{
  LOGE("ImsReasonInfo::GetExtraMessage");
  aExtraMessage = mExtraMessage;
  return NS_OK;
}
NS_IMETHODIMP
ImsReasonInfo::SetExtraMessage(const nsAString & aExtraMessage)
{
  LOGE("ImsReasonInfo::SetExtraMessage");
  mExtraMessage = aExtraMessage;
  return NS_OK;
}

void ImsReasonInfo::readFromParcel(const android::Parcel &source)
{
  LOGI("ImsReasonInfo::readFromParcel");
  SetReasonType(source.readInt32());
  SetReasonCode(source.readInt32());
  SetExtraCode(source.readInt32());
  mExtraMessage = source.readString16();
  SetExtraMessage(mExtraMessage);
}

void ImsReasonInfo::writeToParcel(android::Parcel &dest, int flags)
{
  LOGI("ImsReasonInfo::writeToParcel");
  dest.writeInt32(mReasonType);
  dest.writeInt32(mCode);
  dest.writeInt32(mExtraCode);
  NS_LossyConvertUTF16toASCII convertMsg(mExtraMessage);
  if(convertMsg.Length() > 0)
    dest.writeString16(android::String16(convertMsg.get()));
}

void ImsReasonInfo::copyFrom(ImsReasonInfo *pImsReasonInfo)
{
  mReasonType = pImsReasonInfo->mReasonType;
  mCode = pImsReasonInfo->mCode;
  mExtraCode = pImsReasonInfo->mExtraCode;
  mExtraMessage = pImsReasonInfo->mExtraMessage;
}
