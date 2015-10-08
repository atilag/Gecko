/*









*/

#include "IImsService.h"

#include <binder/IInterface.h>
#include <binder/Parcel.h>

#define IMS_LOGD            ALOGE

namespace android
{
  class BpImsService
      : public BpInterface<IImsService>
  {
  public:
    BpImsService(const sp<IBinder> &impl)
          : BpInterface<IImsService>(impl)
    {
    }

    virtual int open(int phoneId, int serviceClass, int RegistrationListener)
    {
        IMS_LOGD("BpImsService::open()");
        Parcel data, reply;
          data.writeInterfaceToken(IImsService::getInterfaceDescriptor());
        data.writeInt32(phoneId);
        data.writeInt32(serviceClass);
        data.writeInt32(RegistrationListener);
        status_t status = remote()->transact(OPEN, data, &reply);
        if (status != NO_ERROR) {
                IMS_LOGD("import() could not contact remote: %d\n", status);
                return -1;
            }
        int32_t err = reply.readExceptionCode();
        if (err < 0) {
          IMS_LOGD("import() caught exception %d\n", err);
          return -1;
        }
        int32_t ret = reply.readInt32();
        return ret;
    }

    virtual void close(int serviceId)
    {
        IMS_LOGD("BpImsService::close()");
        Parcel data, reply;
          data.writeInterfaceToken(IImsService::getInterfaceDescriptor());
        data.writeInt32(serviceId);
        status_t status = remote()->transact(CLOSE, data, &reply);
        if (status != NO_ERROR) {
                IMS_LOGD("import() could not contact remote: %d\n", status);
                return;
            }
    }

    virtual void setRegistrationListener(int serviceId, int64_t RegistrationListener)
    {
        IMS_LOGD("BpImsService::setRegistrationListener()");
        Parcel data, reply;
          data.writeInterfaceToken(IImsService::getInterfaceDescriptor());
        data.writeInt32(serviceId);
        data.writeInt32(RegistrationListener);
        status_t status = remote()->transact(SETREGLISTERNER, data, &reply);
        if (status != NO_ERROR) {
                IMS_LOGD("import() could not contact remote: %d\n", status);
                return;
            }
    }

    virtual void createCallProfile(int serviceId, int serviceType, int callType,  android::Parcel &OutCallProfile)
    {
        IMS_LOGD("BpImsService::createCallProfile()");
        Parcel data, reply;
          data.writeInterfaceToken(IImsService::getInterfaceDescriptor());
        data.writeInt32(serviceId);
        data.writeInt32(serviceType);
        data.writeInt32(callType);
        status_t status = remote()->transact(CREATECALLPROF, data, &reply);
        if (status != NO_ERROR) {
                IMS_LOGD("import() could not contact remote: %d\n", status);
                return;
            }
        OutCallProfile.appendFrom(&reply, (size_t)reply.dataPosition(), reply.dataAvail());
    }

    virtual int64_t createCallSession(int serviceId,const android::Parcel &profile)
    {
        IMS_LOGD("BpImsService::createCallSession()");
        Parcel data, reply;
          data.writeInterfaceToken(IImsService::getInterfaceDescriptor());
        data.writeInt32(serviceId);
        data.appendFrom(&profile, (size_t)0, profile.dataSize());
        status_t status = remote()->transact(CREATECALLSESSION, data, &reply);
        if (status != NO_ERROR) {
                IMS_LOGD("import() could not contact remote: %d\n", status);
                return -1;
            }
        int32_t err = reply.readExceptionCode();
        if (err < 0) {
          IMS_LOGD("import() caught exception %d\n", err);
          return -1;
        }
        int32_t ret = reply.readInt32();
        return ret;
    }

    virtual int64_t getPendingCallSession(int serviceId,const char* callId)
    {
        IMS_LOGD("BpImsService::getPendingCallSession()");
        Parcel data, reply;
          data.writeInterfaceToken(IImsService::getInterfaceDescriptor());
        data.writeInt32(serviceId);
        data.writeCString(callId);
        status_t status = remote()->transact(GETPENDINGCALLSESSION, data, &reply);
        if (status != NO_ERROR) {
                IMS_LOGD("import() could not contact remote: %d\n", status);
                return -1;
            }
        int32_t err = reply.readExceptionCode();
        if (err < 0) {
          IMS_LOGD("import() caught exception %d\n", err);
          return -1;
        }
        int32_t ret = reply.readInt32();
        return ret;
    }

    virtual int64_t getUtInterface(int serviceId)
    {
        IMS_LOGD("BpImsService::getUtInterface()");
        Parcel data, reply;
          data.writeInterfaceToken(IImsService::getInterfaceDescriptor());
        data.writeInt32(serviceId);
        status_t status = remote()->transact(GETUTINTERFACE, data, &reply);
        if (status != NO_ERROR) {
                IMS_LOGD("import() could not contact remote: %d\n", status);
                return -1;
            }
        int32_t err = reply.readExceptionCode();
        if (err < 0) {
          IMS_LOGD("import() caught exception %d\n", err);
          return -1;
        }
        int32_t ret = reply.readInt32();
        return ret;
    }

    virtual int64_t getEcbmInterface(int serviceId)
    {
        IMS_LOGD("BpImsService::getEcbmInterface()");
        Parcel data, reply;
          data.writeInterfaceToken(IImsService::getInterfaceDescriptor());
        data.writeInt32(serviceId);
        status_t status = remote()->transact(GETECBMINTERFACE, data, &reply);
        if (status != NO_ERROR) {
                IMS_LOGD("import() could not contact remote: %d\n", status);
                return -1;
            }
        int32_t err = reply.readExceptionCode();
        if (err < 0) {
          IMS_LOGD("import() caught exception %d\n", err);
          return -1;
        }
        int32_t ret = reply.readInt32();
        return ret;
    }

  };

  IMPLEMENT_META_INTERFACE(ImsService, "ims");

}
