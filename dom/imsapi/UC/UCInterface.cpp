/*









*/
#include <string.h>
#include <stdio.h>

#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include "IXPCOMCoreInterface.h"

#include <binder/Parcel.h>

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <utils/Log.h>
#undef LOG_TAG
#define LOG_TAG            "LGIMS_UC_INTERFACE"
#define LOG_NDDEBUG        0
#define IMS_LOGD            ALOGE

#include "UCInterface.h"
#include "IImsDbSettings.h"
#include "ImsManager.h"
#include "ImsEcbm.h"

namespace android
{
  class BpUCInterface: public BpInterface<IUCInterface>
  {
  public:
    BpUCInterface(const sp<IBinder> &impl)
          : BpInterface<IUCInterface>(impl)
    {
    }

    virtual void onMessage(const android::Parcel &pParcel)
    {
      IMS_LOGD("IImsInterprocess::onMessage()");
      Parcel data, reply;
      data.writeInterfaceToken(IUCInterface::getInterfaceDescriptor());
      data.appendFrom(&pParcel, (size_t)0, pParcel.dataSize());
      status_t status = remote()->transact(ONMESSAGE, data, &reply);
      if (status != NO_ERROR) {
              IMS_LOGD("import() could not contact remote: %d\n", status);
              return;
      }
    }
    virtual void onMessageEx(const android::Parcel &pParcel, Parcel *pOutParcel)
    {
      IMS_LOGD("IImsInterprocess::onMessageEx()");
      Parcel data, reply;
      data.writeInterfaceToken(IUCInterface::getInterfaceDescriptor());
      data.appendFrom(&pParcel, (size_t)0, pParcel.dataSize());
      data.appendFrom(pOutParcel, (size_t)0, pOutParcel->dataSize());
      status_t status = remote()->transact(ONMESSAGEEX, data, &reply);
      if (status != NO_ERROR) {
              IMS_LOGD("import() could not contact remote: %d\n", status);
              return;
      }
    }

     virtual void registrationConnected()
     {
        IMS_LOGD("IImsInterprocess::registrationConnected()");
        Parcel data, reply;
        data.writeInterfaceToken(IUCInterface::getInterfaceDescriptor());
        status_t status = remote()->transact(REGCONNECT, data, &reply);
        if (status != NO_ERROR) {
              IMS_LOGD("import() could not contact remote: %d\n", status);
              return;
      }
     }

     virtual void registrationDisconnected(const android::Parcel &pParcel)
     {
        IMS_LOGD("IImsInterprocess::registrationDisconnected()");
        Parcel data, reply;
        data.writeInterfaceToken(IUCInterface::getInterfaceDescriptor());
        data.appendFrom(&pParcel, (size_t)0, pParcel.dataSize());
        status_t status = remote()->transact(REGDISCONNECT, data, &reply);
        if (status != NO_ERROR) {
              IMS_LOGD("import() could not contact remote: %d\n", status);
              return;
      }
     }

     virtual void notifyIncomingCall(int phoneID, int serviceID, const char* callId)
     {
        IMS_LOGD("IImsInterprocess::notifyIncomingCall()");
        Parcel data, reply;
        data.writeInterfaceToken(IUCInterface::getInterfaceDescriptor());
        data.writeInt32(phoneID);
        data.writeInt32(serviceID);
        data.writeCString(callId);
        status_t status = remote()->transact(NOTIFYINCOMINGCALL, data, &reply);
        if (status != NO_ERROR) {
              IMS_LOGD("import() could not contact remote: %d\n", status);
              return;
      }
     }

     virtual void enteredECBM()
     {
        IMS_LOGD("IImsInterprocess::enteredECBM()");
        Parcel data, reply;
        data.writeInterfaceToken(IUCInterface::getInterfaceDescriptor());
        status_t status = remote()->transact(ENTEREDECBM, data, &reply);
        if (status != NO_ERROR) {
              IMS_LOGD("import() could not contact remote: %d\n", status);
              return;
      }
     }

     virtual void exitedECBM()
     {
        IMS_LOGD("IImsInterprocess::exitedECBM()");
        Parcel data, reply;
        data.writeInterfaceToken(IUCInterface::getInterfaceDescriptor());
        status_t status = remote()->transact(EXITEDECBM, data, &reply);
        if (status != NO_ERROR) {
              IMS_LOGD("import() could not contact remote: %d\n", status);
              return;
      }
     }

     virtual void set911State(int32_t state, int32_t ecbm)
     {
        IMS_LOGD("IImsInterprocess::set911State()");
        Parcel data, reply;
        data.writeInterfaceToken(IUCInterface::getInterfaceDescriptor());
        status_t status = remote()->transact(SET911STATE, data, &reply);
        if (status != NO_ERROR) {
              IMS_LOGD("import() could not contact remote: %d\n", status);
              return;
      }
     }
  };


  IMPLEMENT_META_INTERFACE(UCInterface, "com.lge.ims.ucinterface");

  status_t BnUCInterface::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags /*     */)
  {
    (void)flags;
    IMS_LOGD("In  Server BnUCInterface::onTransact");

    switch ( code )
    {
      case ONMESSAGE:
      {
        IMS_LOGD("In  BnXPCOMCoreInterface::onTransact -- ONMESSAGE");
        CHECK_INTERFACE(IXPCOMCoreInterface, data, reply);
        int pListernerID = data.readInt32();
        BnUCInterface *pListener = reinterpret_cast<BnUCInterface*>(pListernerID);
        IMS_LOGD("In  BnXPCOMCoreInterface::onTransact -- ONMESSAGE::pListernerID - %d", pListernerID);
        android::Parcel temp;
        temp.appendFrom(&data, (size_t)data.dataPosition(), data.dataAvail());
        if(pListener != nullptr) {
          pListener->onMessage(temp);
          reply->writeNoException();
        } else {
          IMS_LOGD("In  BnXPCOMCoreInterface::onTransact -- ONMESSAGE::pListener is nullptr");
        }
      }
      break;
      case ONMESSAGEEX:
      {
        IMS_LOGD("In  BnXPCOMCoreInterface::onTransact -- ONMESSAGEEX");
        CHECK_INTERFACE(IXPCOMCoreInterface, data, reply);
        int pListernerID = data.readInt32();
        BnUCInterface *pListener = reinterpret_cast<BnUCInterface*>(pListernerID);
        android::Parcel temp, outParcel;
        temp.appendFrom(&data, (size_t)data.dataPosition(), data.dataAvail());
        pListener->onMessageEx(temp, &outParcel);
        reply->writeNoException();
        reply->appendFrom(&outParcel, (size_t)0, outParcel.dataSize());
       }
      break;
      case REGCONNECT:
      {
        IMS_LOGD("In  BnXPCOMCoreInterface::onTransact -- REGCONNECT");
        CHECK_INTERFACE(IXPCOMCoreInterface, data, reply);
        (ImsManager::getInstance())->registrationConnected();
        reply->writeNoException();
       }
      break;
      case REGDISCONNECT:
      {
        IMS_LOGD("In  BnXPCOMCoreInterface::onTransact -- REGDISCONNECT");
        CHECK_INTERFACE(IXPCOMCoreInterface, data, reply);
        android::Parcel temp;
        temp.appendFrom(&data, (size_t)data.dataPosition(), data.dataAvail());
        (ImsManager::getInstance())->registrationDisconnected(temp);
        reply->writeNoException();
       }
      break;
      case NOTIFYINCOMINGCALL:
      {
        IMS_LOGD("In  BnXPCOMCoreInterface::onTransact -- NOTIFYINCOMINGCALL");
        CHECK_INTERFACE(IXPCOMCoreInterface, data, reply);
        (ImsManager::getInstance())->notifyIncomingCall(data.readInt32(), data.readInt32(), data.readCString());
        reply->writeNoException();
       }
      break;
      case ENTEREDECBM:
      {
        IMS_LOGD("In  BnXPCOMCoreInterface::onTransact -- ENTEREDECBM");
        CHECK_INTERFACE(IXPCOMCoreInterface, data, reply);
        ImsEcbm::ImsEcbmListenerProxy* mListener = reinterpret_cast<ImsEcbm::ImsEcbmListenerProxy*>(data.readInt32());
        mListener->enteredECBM();
        reply->writeNoException();
       }
      break;
      case EXITEDECBM:
      {
        IMS_LOGD("In  BnXPCOMCoreInterface::onTransact -- EXITEDECBM");
        CHECK_INTERFACE(IXPCOMCoreInterface, data, reply);
        //
        //
        reply->writeNoException();
       }
      break;
      case SET911STATE:
      {
        IMS_LOGD("In  BnXPCOMCoreInterface::onTransact -- SET911STATE");
        CHECK_INTERFACE(IXPCOMCoreInterface, data, reply);
        (ImsManager::getInstance())->set911State(data.readInt32(), data.readInt32());
        reply->writeNoException();
       }
      break;
    }
    return NO_ERROR;
  }

//
  class BpXPCOMCoreInterface
     : public BpInterface<IXPCOMCoreInterface>
  {
   public:
    BpXPCOMCoreInterface(const sp<IBinder> &impl)
        : BpInterface<IXPCOMCoreInterface>(impl)
    {
    }

    virtual int64_t XpcomCore_getInterface(int nInterfaceType)
    {
      IMS_LOGD("IUCInterface::XpcomCore_getInterface()");
      Parcel data, reply;
      data.writeInterfaceToken(IXPCOMCoreInterface::getInterfaceDescriptor());
      data.writeInt32(nInterfaceType);
      status_t status = remote()->transact(GETINTERFACE, data, &reply);
      if (status != NO_ERROR) {
        IMS_LOGD("import() could not contact remote: %d\n", status);
        return -1;
      }
      int32_t err = reply.readExceptionCode();
      if (err < 0) {
        IMS_LOGD("import() caught exception %d\n", err);
        return -1;
      }
      int64_t ret = reply.readInt32();
      return ret;
    }

    virtual void XpcomCore_setListener(int nServiceId, int pListener)
    {
      IMS_LOGD("IUCInterface::XpcomCore_setListener()::nServiceId - %d, pListener - %d", nServiceId, pListener);
      Parcel data, reply;
      data.writeInterfaceToken(IXPCOMCoreInterface::getInterfaceDescriptor());
      data.writeInt32(nServiceId);
      data.writeInt32(pListener);
      status_t status = remote()->transact(SETLISTENER, data, &reply);
      if (status != NO_ERROR) {
        IMS_LOGD("import() could not contact remote: %d\n", status);
        return;
      }
      int32_t err = reply.readExceptionCode();
      if (err < 0) {
        IMS_LOGD("import() caught exception %d\n", err);
        return;
      }
      return ;
    }

    virtual int XpcomCore_releaseInterface(int nNativeObject)
    {
      IMS_LOGD("IUCInterface::XpcomCore_releaseInterface()");
      Parcel data, reply;
      data.writeInterfaceToken(IXPCOMCoreInterface::getInterfaceDescriptor());
      data.writeInt32(nNativeObject);
      status_t status = remote()->transact(RELEASEINTERFACE, data, &reply);
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

    virtual int XpcomCore_sendData(int nNativeObject, const android::Parcel &parcel)
    {
      IMS_LOGD("IUCInterface::XpcomCore_sendData()");
      Parcel data, reply;
      data.writeInterfaceToken(IXPCOMCoreInterface::getInterfaceDescriptor());
      data.writeInt32(nNativeObject);
      data.appendFrom(&parcel, (size_t)0, parcel.dataSize());
      status_t status = remote()->transact(SENDDATA, data, &reply);
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

    virtual int XpcomCore_sendData(int nNativeObject, const android::Parcel &iparcel, android::Parcel &oparcel)
    {
        IMS_LOGD("IUCInterface::XpcomCore_sendData()");
        Parcel data, reply;
          data.writeInterfaceToken(IXPCOMCoreInterface::getInterfaceDescriptor());
        data.writeInt32(nNativeObject);
        data.appendFrom(&iparcel, (size_t)0, iparcel.dataSize());
        status_t status = remote()->transact(SENDDATAOUT, data, &reply);
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
        oparcel.appendFrom(&reply, (size_t)reply.dataPosition(), reply.dataAvail());
        return ret;
    }

    virtual int XpcomCore_sendDataEx(int nNativeObject, const android::Parcel &parcel, android::Parcel &Outparcel)
   {
       IMS_LOGD("IUCInterface::XpcomCore_sendDataEx()");
        Parcel data, reply;
          data.writeInterfaceToken(IXPCOMCoreInterface::getInterfaceDescriptor());
        data.writeInt32(nNativeObject);
        data.appendFrom(&parcel, (size_t)0, parcel.dataSize());
        status_t status = remote()->transact(SENDDATAEX, data, &reply);
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
        Outparcel.appendFrom(&reply, (size_t)reply.dataPosition(), reply.dataAvail());
        return ret;
    }

    virtual void XpcomCore_construct()
    {
        IMS_LOGD("IImsInterprocess::XpcomCore_construct()");
        Parcel data, reply;
          data.writeInterfaceToken(IXPCOMCoreInterface::getInterfaceDescriptor());
        status_t status = remote()->transact(CONSTRUCT, data, &reply);
        if (status != NO_ERROR) {
                IMS_LOGD("import() could not contact remote: %d\n", status);
                return ;
            }
        int32_t err = reply.readExceptionCode();
        if (err < 0) {
                IMS_LOGD("import() caught exception %d\n", err);
                return ;
            }
    }

    virtual void ECBM_setListener(long listener, long mImsEcbmImplObj)
    {
        IMS_LOGD("IImsInterprocess::ECBM_setListener()");
        Parcel data, reply;
          data.writeInterfaceToken(IXPCOMCoreInterface::getInterfaceDescriptor());
        data.writeInt64(listener);
        data.writeInt64(mImsEcbmImplObj);
        status_t status = remote()->transact(ECBM_SETLISTENER, data, &reply);
        if (status != NO_ERROR) {
                IMS_LOGD("import() could not contact remote: %d\n", status);
                return ;
            }
        int32_t err = reply.readExceptionCode();
        if (err < 0) {
                IMS_LOGD("import() caught exception %d\n", err);
                return ;
            }
    }

    virtual void exitEmergencyCallbackMode(long mImsEcbmImplObj)
    {
        IMS_LOGD("IImsInterprocess::exitEmergencyCallbackMode()");
        Parcel data, reply;
          data.writeInterfaceToken(IXPCOMCoreInterface::getInterfaceDescriptor());
        data.writeInt64(mImsEcbmImplObj);
        status_t status = remote()->transact(EXITEMERGENCYCALLBACKMODE, data, &reply);
        if (status != NO_ERROR) {
                IMS_LOGD("import() could not contact remote: %d\n", status);
                return ;
            }
        int32_t err = reply.readExceptionCode();
        if (err < 0) {
                IMS_LOGD("import() caught exception %d\n", err);
                return ;
            }
    }
 };

  IMPLEMENT_META_INTERFACE(XPCOMCoreInterface, "com.lge.ims.coreinterface");

//
  class BpXpcomDbSettingsInterface
     : public BpInterface<IXpcomDbSettingsInterface>
  {
    public:
    BpXpcomDbSettingsInterface(const sp<IBinder> &impl)
         : BpInterface<IXpcomDbSettingsInterface>(impl)
    {
    }

    virtual char ***ReadFromDb(const char *strTable)
    {
      IMS_LOGD("IImsInterprocess::ReadFromDb()");
      Parcel data, reply;
      data.writeInterfaceToken(IXpcomDbSettingsInterface::getInterfaceDescriptor());
      data.writeCString(strTable);
      status_t status = remote()->transact(READFROMDB, data, &reply);
      if (status != NO_ERROR) {
        IMS_LOGD("import() could not contact remote: %d\n", status);
        return NULL;
      }
      int32_t err = reply.readExceptionCode();
      if (err < 0) {
        IMS_LOGD("import() caught exception %d\n", err);
        return NULL;
      }
      return NULL;
    }

    virtual bool UpdateDb(const char *strTable, char ***content)
    {
      IMS_LOGD("IImsInterprocess::UpdateDb()");
      Parcel data, reply;
      data.writeInterfaceToken(IXpcomDbSettingsInterface::getInterfaceDescriptor());
      data.writeCString(strTable);
      status_t status = remote()->transact(UPDATEDB, data, &reply);
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

    virtual bool DbtoXML()
    {
      IMS_LOGD("IImsInterprocess::DbtoXML()");
      Parcel data, reply;
      data.writeInterfaceToken(IXpcomDbSettingsInterface::getInterfaceDescriptor());
      status_t status = remote()->transact(DBTOXML, data, &reply);
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

    virtual void UpdatePSIValue(const char *strPSI)
    {
      IMS_LOGD("IImsInterprocess::UpdatePSIValue()");
      Parcel data, reply;
      data.writeInterfaceToken(IXpcomDbSettingsInterface::getInterfaceDescriptor());
      data.writeCString(strPSI);
      status_t status = remote()->transact(UPDATEPSIVALUE, data, &reply);
      if (status != NO_ERROR) {
        IMS_LOGD("import() could not contact remote: %d\n", status);
        return;
      }
      int32_t err = reply.readExceptionCode();
      if (err < 0) {
        IMS_LOGD("import() caught exception %d\n", err);
        return;
      }
      return;
    }

    virtual bool isImsOn()
    {
      IMS_LOGD("IImsInterprocess::isImsOn()");
      Parcel data, reply;
      data.writeInterfaceToken(IXpcomDbSettingsInterface::getInterfaceDescriptor());
      status_t status = remote()->transact(ISIMSON, data, &reply);
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

    virtual int GetTestMaskValue()
    {
      IMS_LOGD("IImsInterprocess::GetTestMaskValue()");
      Parcel data, reply;
      data.writeInterfaceToken(IXpcomDbSettingsInterface::getInterfaceDescriptor());
      status_t status = remote()->transact(TESTMASK, data, &reply);
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

  IMPLEMENT_META_INTERFACE(XpcomDbSettingsInterface, "com.lge.ims.dbinterface");


}
