#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <binder/IServiceManager.h>
#include "ImsInterfaceImpl.h"
#include "IXPCOMCoreInterface.h"

using namespace android ;
using namespace mozilla;

using namespace std;

class NativeWrapper
{
public:
  NativeWrapper();
  virtual ~NativeWrapper();

  static NativeWrapper* GetInstance();
  void sendData2NativeImpl(int32_t nCmd, android::Parcel & Inparcel);
  void NativeSystemInterface(ImsInterfaceImpl* imsImplob);
  int mNativeObject = 0;
private:

  static NativeWrapper* NativeWrapperObj;

};
