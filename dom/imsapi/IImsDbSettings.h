#ifndef IIms_Db_Settings__
#define IIms_Db_Settings__
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#define COM_LGE_IMS_DBINTERFACE           "com.lge.ims.dbinterface"

namespace android
{

class IXpcomDbSettingsInterface : public IInterface
{
public:
    DECLARE_META_INTERFACE(XpcomDbSettingsInterface);
    virtual char ***ReadFromDb(const char *strTable) = 0;
    virtual bool UpdateDb(const char *strTable, char ***content) = 0;
    virtual bool DbtoXML() = 0;
    virtual void UpdatePSIValue(const char *strPSI)=0;
    virtual bool isImsOn()=0;
    virtual int GetTestMaskValue()=0;

protected:
        //
        enum
        {
            READFROMDB = IBinder::FIRST_CALL_TRANSACTION,
            UPDATEDB = IBinder::FIRST_CALL_TRANSACTION + 1,
            DBTOXML = IBinder::FIRST_CALL_TRANSACTION + 2,
            UPDATEPSIVALUE = IBinder::FIRST_CALL_TRANSACTION + 3,
            ISIMSON = IBinder::FIRST_CALL_TRANSACTION + 4,
            TESTMASK = IBinder::FIRST_CALL_TRANSACTION + 5
        };
};

class BnXpcomDbSettingsInterface
            : public BnInterface<IXpcomDbSettingsInterface>
{
  public :
       virtual status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply,
            uint32_t flags = 0);
};
//
}
#endif //