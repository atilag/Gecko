/*








*/
#include "ImsEcbm.h"
#include "nsILgeImsUCManager.h"

NS_IMPL_ISUPPORTS(ImsEcbm, nsIImsEcbm)

ImsEcbm::ImsEcbm(int64_t iEcbm){
  miEcbm = iEcbm;
}

ImsEcbm::~ImsEcbm(){

}

NS_IMETHODIMP
ImsEcbm::SetEcbmStateListener(nsIImsEcbmListenerProxy* ecbmListener)
{
  sp<IXPCOMCoreInterface> pXPCOMCoreInterfaceObj = nullptr;
  if(pXPCOMCoreInterfaceObj == NULL){
    sp<IServiceManager> ism = defaultServiceManager();
    sp<IBinder> binder;
    binder = ism->getService(String16("com.lge.ims.coreinterface"));
    if (binder != 0)
    {
      pXPCOMCoreInterfaceObj = interface_cast<IXPCOMCoreInterface>(binder);
      pXPCOMCoreInterfaceObj->ECBM_setListener(reinterpret_cast<int>(new ImsEcbmListenerProxy(ecbmListener)),miEcbm);
    }
  }
}

NS_IMETHODIMP
ImsEcbm::ExitEmergencyCallbackMode()
{
  sp<IXPCOMCoreInterface> pXPCOMCoreInterfaceObj = NULL;
  sp<IServiceManager> ism = defaultServiceManager();
  sp<IBinder> binder;
  binder = ism->getService(String16("com.lge.ims.coreinterface"));
  if (binder != 0)
  {
    pXPCOMCoreInterfaceObj = interface_cast<IXPCOMCoreInterface>(binder);
    pXPCOMCoreInterfaceObj->exitEmergencyCallbackMode(miEcbm);
  }
}

ImsEcbm::ImsEcbmListenerProxy::ImsEcbmListenerProxy(nsIImsEcbmListenerProxy* listener){
  mListener = listener;
}

ImsEcbm::ImsEcbmListenerProxy::~ImsEcbmListenerProxy(){

}

void ImsEcbm::ImsEcbmListenerProxy::enteredECBM(){
  if (mListener != NULL) {
    mListener->EnteredECBM();
  }
}

void ImsEcbm::ImsEcbmListenerProxy::exitedECBM(){
  if (mListener != NULL) {
    mListener->ExitedECBM();
  }
}
