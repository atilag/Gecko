#include "NativeWrapper.h"

#define LOGI(args...) \
     __android_log_print(ANDROID_LOG_INFO, LOG_TAG, ## args)

#define LOGE(args...) \
     __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, ## args)


NativeWrapper* NativeWrapper::NativeWrapperObj;

NativeWrapper::NativeWrapper()
{
  NativeWrapperObj = this;
}

NativeWrapper::~NativeWrapper()
{
  delete NativeWrapperObj;
}

NativeWrapper*
NativeWrapper::GetInstance()
{
  if(nullptr == NativeWrapperObj)
  {
    NativeWrapperObj= new NativeWrapper();
  }
  return NativeWrapperObj;
}

void
NativeWrapper::NativeSystemInterface(ImsInterfaceImpl *imsImplob){
  LOGE("NativeWrapper::NativeSystemInterface is called");

  sp<IXPCOMCoreInterface> pXPCOMCoreInterfaceObj = NULL;
  sp<IServiceManager> ism = defaultServiceManager();
  sp<IBinder> binder;
  binder = ism->getService(String16("com.lge.ims.coreinterface"));
  if (binder != 0)
  {
    pXPCOMCoreInterfaceObj = interface_cast<IXPCOMCoreInterface>(binder);
    pXPCOMCoreInterfaceObj->XpcomCore_construct();
    mNativeObject = pXPCOMCoreInterfaceObj->XpcomCore_getInterface(51/*                       */);
    pXPCOMCoreInterfaceObj->XpcomCore_setListener(mNativeObject,    reinterpret_cast<int>(imsImplob));
  }
}

void
NativeWrapper::sendData2NativeImpl(int32_t nCmd, android::Parcel & Inparcel)
{
  LOGE("NativeWrapper:: sendData2NativeImpl is called");
  Parcel parcel , outParcel;

  parcel.writeInt32(nCmd);
  parcel.appendFrom(&Inparcel, (size_t)0, Inparcel.dataSize());
  sp<IXPCOMCoreInterface> pXPCOMCoreInterfaceObj = NULL;
  sp<IServiceManager> ism = defaultServiceManager();
  sp<IBinder> binder;
  binder = ism->getService(String16("com.lge.ims.coreinterface"));
  if (binder != 0)
  {
    pXPCOMCoreInterfaceObj = interface_cast<IXPCOMCoreInterface>(binder);
  }
  pXPCOMCoreInterfaceObj->XpcomCore_sendDataEx(mNativeObject, parcel,outParcel);
  Parcel parcelOut;
}
