/*                */

#ifndef lge_imsxpcom_ucinterface_h__
#define lge_imsxpcom_ucinterface_h__

#include <binder/IInterface.h>
#include <binder/Parcel.h>

#define COM_LGE_IMS_UCINTERFACE         "com.lge.ims.ucinterface"

namespace android
{

  class IUCInterface: public IInterface
  {
  public:
    DECLARE_META_INTERFACE(UCInterface);

    virtual void onMessage(const android::Parcel &pParcel)=0;
    virtual void onMessageEx(const android::Parcel &pParcel, Parcel* pOutParcel)=0;
    virtual    void registrationConnected()=0;
    virtual void registrationDisconnected(const android::Parcel &pParcel)=0;
    virtual void notifyIncomingCall(int phoneID, int serviceID, const char* callId)=0;
    virtual void enteredECBM()=0;
    virtual void exitedECBM()=0;
    virtual void set911State(int32_t state, int32_t ecbm)=0;

  protected:
            //
    enum
    {
      ONMESSAGE = IBinder::FIRST_CALL_TRANSACTION,
      ONMESSAGEEX = IBinder::FIRST_CALL_TRANSACTION + 1,
      REGCONNECT = IBinder::FIRST_CALL_TRANSACTION + 2,
      REGDISCONNECT = IBinder::FIRST_CALL_TRANSACTION + 3,
      NOTIFYINCOMINGCALL = IBinder::FIRST_CALL_TRANSACTION + 4,
      ENTEREDECBM = IBinder::FIRST_CALL_TRANSACTION + 5,
      EXITEDECBM = IBinder::FIRST_CALL_TRANSACTION + 6,
      SET911STATE = IBinder::FIRST_CALL_TRANSACTION + 7
    };

  };

  class BnUCInterface
            : public BnInterface<IUCInterface>
  {
    public :
      virtual status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply,
            uint32_t flags = 0);
  };

}

#endif  //
