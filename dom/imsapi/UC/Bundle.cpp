#undef LOG_TAG
#define LOG_TAG "LGE_BUNDLE"
#include <utils/Log.h>
#include "Bundle.h"
#define LOGI(args...) \
     __android_log_print(ANDROID_LOG_INFO, LOG_TAG, ## args)

#define LOGE(args...) \
     __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, ## args)

Bundle::Bundle()
{
  LOGI("Bundle::Bundle()");
  mString.clear();
  mBool.clear();
  mInt.clear();
}

Bundle::Bundle(const android::Parcel &pParcel)
{
  LOGI("Bundle::Bundle(Parcel)");
  readFromParcel(pParcel);
}

Bundle::~Bundle()
{
  LOGI("Bundle::~Bundle()");
}

nsString Bundle::getString(nsString& str, nsString& defaultVal)
{
  nsString retVal;
  if(!defaultVal.IsEmpty())
    return defaultVal;

  if (mString.find(str) != mString.end()) {
    retVal = mString[str];
    NS_LossyConvertUTF16toASCII convertSrcVal(str);
    NS_LossyConvertUTF16toASCII convertRetVal(retVal);
    LOGI("Bundle::getString::Src - %s, Val - %s", convertSrcVal.get(), convertRetVal.get());
  }

  return retVal;
}

bool Bundle::getBoolean(nsString& str, bool defaultVal)
{
  bool retVal = false;
  if(true == defaultVal)
    return defaultVal;

  if (mBool.find(str) != mBool.end()) {
    retVal = mBool[str];
    NS_LossyConvertUTF16toASCII convertSrcVal(str);
    LOGI("Bundle::getString::Src - %s, Val - %d", convertSrcVal.get(), retVal);
  }

  return retVal;
}

int32_t Bundle::getInt(nsString& str, int32_t defaultVal)
{
  int32_t retVal = 0;
  if(defaultVal <= -1)
    return defaultVal;

  if (mInt.find(str) != mInt.end()) {
    retVal = mInt[str];
    NS_LossyConvertUTF16toASCII convertSrcVal(str);
    LOGI("Bundle::getString::Src - %s, Val - %d", convertSrcVal.get(), retVal);
  }

  return retVal;
}

void Bundle::putString(const nsString& str, nsString& val)
{
  NS_LossyConvertUTF16toASCII convertSrcVal(str);
  NS_LossyConvertUTF16toASCII convertVal(val);
  LOGI("Bundle::putString::Src - %s, Val - %s", convertSrcVal.get(), convertVal.get());
  mString.insert ( std::pair<nsString, nsString>(str,val) );
}

void Bundle::putBoolean(const nsString& str, bool val)
{
  NS_LossyConvertUTF16toASCII convertSrcVal(str);
  LOGI("Bundle::getString::Src - %s, Val - %d", convertSrcVal.get(), val);
  mBool.insert ( std::pair<nsString, bool>(str,val) );
}

void Bundle::putInt(const nsString& str, int32_t val)
{
  NS_LossyConvertUTF16toASCII convertSrcVal(str);
  LOGI("Bundle::getString::Src - %s, Val - %d", convertSrcVal.get(), val);
  mInt.insert ( std::pair<nsString, int32_t>(str,val) );
}

void Bundle::clear()
{
  mString.clear();
  mBool.clear();
  mInt.clear();
}

void Bundle::clone(Bundle *bundle)
{
  std::map<nsString, nsString>::iterator it;
  for (it = bundle->mString.begin(); it != bundle->mString.end(); ++it ) {
    putString(it->first, it->second);
  }

  std::map<nsString, bool>::iterator it1;
  for (it1 = bundle->mBool.begin(); it1 != bundle->mBool.end(); ++it1 ) {
    bool val = it1->second;
    putBoolean(it1->first, val);
  }

  std::map<nsString, int32_t>::iterator it2;
  for (it2 = bundle->mInt.begin(); it2 != bundle->mInt.end(); ++it2 ) {
    int32_t val = it1->second;
    putInt(it2->first, val);
  }
}

void Bundle::writeToParcel(android::Parcel &dest, int32_t flags)
{
  LOGI("Bundle::writeToParcel()");
  LOGI("Bundle::writeToParcel::iStringCount - %d", mString.size());
  LOGI("Bundle::writeToParcel::iBoolCount - %d", mBool.size());
  LOGI("Bundle::writeToParcel::iIntCount - %d", mInt.size());
  dest.writeInt32(mString.size());
  std::map<nsString, nsString>::iterator it;
  for (it = mString.begin(); it != mString.end(); ++it ) {
    LOGI("Bundle::writeToParcel::check1");
    NS_LossyConvertUTF16toASCII convertSource(it->first);
    dest.writeString16(android::String16(convertSource.get()));
    NS_LossyConvertUTF16toASCII convertVal(it->second);
    dest.writeString16(android::String16(convertVal.get()));
  }

  dest.writeInt32(mBool.size());
  std::map<nsString, bool>::iterator it1;
  for (it1 = mBool.begin(); it1 != mBool.end(); ++it1 ) {
    NS_LossyConvertUTF16toASCII convertSource(it1->first);
    dest.writeString16(android::String16(convertSource.get()));
    dest.writeInt32(it1->second);
  }

  dest.writeInt32(mInt.size());
  std::map<nsString, int32_t>::iterator it2;
  for (it2 = mInt.begin(); it2 != mInt.end(); ++it2 ) {
    NS_LossyConvertUTF16toASCII convertSource(it1->first);
    dest.writeString16(android::String16(convertSource.get()));
    dest.writeInt32(it1->second);
  }
}

void Bundle::readFromParcel(const android::Parcel &source)
{
  LOGI("Bundle::readFromParcel()");
  nsString src;
  nsString val;
  int32_t iStringCount = source.readInt32();
  LOGI("Bundle::readFromParcel::iStringCount - %d", iStringCount);
  for(int32_t count = 0; count < iStringCount; count++)
  {
    src = source.readString16();
    val = source.readString16();
    putString(src, val);
  }

  int32_t iBoolCount = source.readInt32();
  LOGI("Bundle::readFromParcel::iBoolCount - %d", iBoolCount);
  for(int32_t count = 0; count < iBoolCount; count++)
  {
    src = source.readString16();
    bool val = false;
    if(source.readInt32()) {
      val = true;
    } else {
      val = false;
    }
    putBoolean(src, val);
  }

  int32_t iIntCount = source.readInt32();
  LOGI("Bundle::readFromParcel::iIntCount - %d", iIntCount);
  for(int32_t count = 0; count < iIntCount; count++)
  {
    src = source.readString16();
    int32_t val = source.readInt32();
    putInt(src, val);
  }
}