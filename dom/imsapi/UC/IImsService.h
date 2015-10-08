#ifndef IImsService_h__
#define IImsService_h__


#include <binder/IInterface.h>
#include <binder/Parcel.h>

#define IMS_SERVICE            "ims"

namespace android
{
class IImsService: public IInterface
{
public:
    DECLARE_META_INTERFACE(ImsService);
    virtual int open(int phoneId, int serviceClass, int RegistrationListener) = 0;
    virtual void close(int serviceId) = 0;
    virtual void setRegistrationListener(int serviceId, int64_t RegistrationListener) = 0;
    virtual void createCallProfile(int serviceId, int serviceType, int callType,  android::Parcel &OutCallProfile) = 0;
    virtual int64_t createCallSession(int serviceId,const android::Parcel &profile) = 0;
    virtual int64_t getPendingCallSession(int serviceId,const char* callId) = 0;
    virtual int64_t getUtInterface(int serviceId) = 0;
   /*

    */
   virtual int64_t getEcbmInterface(int serviceId) = 0;

 /*

   */
    //

protected:
        //
        enum
        {
            OPEN = IBinder::FIRST_CALL_TRANSACTION,
            CLOSE = IBinder::FIRST_CALL_TRANSACTION + 1,
            SETREGLISTERNER = IBinder::FIRST_CALL_TRANSACTION + 2,
            CREATECALLPROF = IBinder::FIRST_CALL_TRANSACTION + 3,
            CREATECALLSESSION = IBinder::FIRST_CALL_TRANSACTION + 4,
            GETPENDINGCALLSESSION = IBinder::FIRST_CALL_TRANSACTION + 5,
            GETUTINTERFACE = IBinder::FIRST_CALL_TRANSACTION + 6,
            GETECBMINTERFACE = IBinder::FIRST_CALL_TRANSACTION + 7
        };
};

class BnImsService
            : public BnInterface<IImsService>
{
  public :
       virtual status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply,
            uint32_t flags = 0);

};


}
#endif //
