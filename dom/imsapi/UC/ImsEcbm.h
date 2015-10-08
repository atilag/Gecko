#ifndef __IMS_ECBM_H_
#define __IMS_ECBM_H_

#include <binder/IInterface.h>
#include <binder/IServiceManager.h>
#include "IXPCOMCoreInterface.h"

#include "nsString.h"
#include "nsILgeImsUCManager.h"
#include "UCInterface.h"

using namespace android ;
using namespace mozilla;

class ImsEcbm: public nsIImsEcbm {
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIIMSECBM

  ImsEcbm(int64_t iEcbm);
  virtual ~ImsEcbm();

  /*

   */
  class ImsEcbmListenerProxy //
  {
    private:
      nsIImsEcbmListenerProxy* mListener;
    public:
      ImsEcbmListenerProxy(nsIImsEcbmListenerProxy* listener);
      virtual ~ImsEcbmListenerProxy();
      void enteredECBM();
      void exitedECBM();
    };
private:
  int64_t miEcbm;
};
#endif //
