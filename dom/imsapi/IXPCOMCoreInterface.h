#ifndef IXPCOM_Core_Interface_h__
#define IXPCOM_Core_Interface_h__
//

#include <binder/IInterface.h>
#include <binder/Parcel.h>

#define COM_LGE_IMS_COREITNTERFACE            "com.lge.ims.coreinterface"

namespace android
{
class IXPCOMCoreInterface: public IInterface
{
public:
    DECLARE_META_INTERFACE(XPCOMCoreInterface);
    virtual int64_t XpcomCore_getInterface(int nInterfaceType)=0;
    virtual int XpcomCore_releaseInterface(int nNativeObject)=0;
    virtual void XpcomCore_setListener(int nServiceId, int pListener)=0;
    virtual int XpcomCore_sendData(int nNativeObject, const android::Parcel &parcel)=0;
    virtual int XpcomCore_sendData(int nNativeObject, const android::Parcel &parcel, android::Parcel &Outparcel)=0;
    virtual int XpcomCore_sendDataEx(int nNativeObject, const android::Parcel &parcel, android::Parcel &Outparcel)=0;
    virtual void XpcomCore_construct()=0;
    virtual void ECBM_setListener(long listener, long mImsEcbmImplObj)=0;
    virtual void exitEmergencyCallbackMode(long mImsEcbmImplObj)=0;

protected:
        //
        enum
        {
            GETINTERFACE = IBinder::FIRST_CALL_TRANSACTION,
            SETLISTENER = IBinder::FIRST_CALL_TRANSACTION + 1,
            RELEASEINTERFACE = IBinder::FIRST_CALL_TRANSACTION + 2,
            SENDDATA = IBinder::FIRST_CALL_TRANSACTION + 3,
            SENDDATAOUT = IBinder::FIRST_CALL_TRANSACTION + 4,
            SENDDATAEX = IBinder::FIRST_CALL_TRANSACTION + 5,
            CONSTRUCT = IBinder::FIRST_CALL_TRANSACTION + 6,
            ECBM_SETLISTENER = IBinder::FIRST_CALL_TRANSACTION + 7,
            EXITEMERGENCYCALLBACKMODE = IBinder::FIRST_CALL_TRANSACTION + 8
        };
};

class BnXPCOMCoreInterface
            : public BnInterface<IXPCOMCoreInterface>
{
  public :
       virtual status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply,
            uint32_t flags = 0);

};


}
#endif //
