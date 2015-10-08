#ifndef __IXPCOM_UC_CALL_PROFILE_H_
#define __IXPCOM_UC_CALL_PROFILE_H_
#include <binder/Parcel.h>
#include <utils/String16.h>
#include "nsString.h"
#include "Bundle.h"
#include "ImsStreamMediaProfile.h"
using namespace android ;

class ImsCallProfile
{
  public:
    /*

     */
    /*




     */
    static const int32_t SERVICE_TYPE_NONE = 0;
    /*

     */
    static const int32_t SERVICE_TYPE_NORMAL = 1;
    /*

     */
    static const int32_t SERVICE_TYPE_EMERGENCY = 2;

    /*

     */
    /*

     */
    static const int32_t CALL_TYPE_VOICE_N_VIDEO = 1;
    /*

     */
    static const int32_t CALL_TYPE_VOICE = 2;
    /*

     */
    static const int32_t CALL_TYPE_VIDEO_N_VOICE = 3;
    /*

     */
    static const int32_t CALL_TYPE_VT = 4;
    /*

     */
    static const int32_t CALL_TYPE_VT_TX = 5;
    /*

     */
    static const int32_t CALL_TYPE_VT_RX = 6;
    /*

     */
    static const int32_t CALL_TYPE_VT_NODIR = 7;
    /*

     */
    static const int32_t CALL_TYPE_VS = 8;
    /*

     */
    static const int32_t CALL_TYPE_VS_TX = 9;
    /*

     */
    static const int32_t CALL_TYPE_VS_RX = 10;

    /*

     */
    /*







     */
    nsString EXTRA_CONFERENCE;//
    nsString EXTRA_E_CALL;//
    nsString EXTRA_VMS;//
    nsString EXTRA_CALL_MODE_CHANGEABLE;//
    nsString EXTRA_CONFERENCE_AVAIL;//
    nsString EXTRA_MMC;//
    nsString EXTRA_GTT;//
    static nsString EXTRA_HD_VOICE;//
    nsString EXTRA_CONFERENCE_EVENT;//

    /*













     */
    nsString EXTRA_OIR;//
    nsString EXTRA_CNAP;//
    nsString EXTRA_DIALSTRING;//
    nsString EXTRA_CALL_DOMAIN;//

    /*

     */
    const int32_t OIR_DEFAULT = 0;    //
    const int32_t OIR_PRESENTATION_RESTRICTED = 1;
    const int32_t OIR_PRESENTATION_NOT_RESTRICTED = 2;
    const int32_t OIR_PRESENTATION_UNKNOWN = 3;
    const int32_t OIR_PRESENTATION_PAYPHONE = 4;

    /*

     */
    //
    static const int32_t DIALSTRING_NORMAL = 0;
    //
    static const int32_t DIALSTRING_SS_CONF = 1;
    //
    static const int32_t DIALSTRING_USSD = 2;

    /*

     */
    //
    static const int32_t CALL_RESTRICT_CAUSE_NONE = 0;
    //
    static const int32_t CALL_RESTRICT_CAUSE_RAT = 1;
    //
    static const int32_t CALL_RESTRICT_CAUSE_DISABLED = 2;
    //
    static const int32_t CALL_RESTRICT_CAUSE_HD = 3;

    /*










     */
    static nsString EXTRA_OI;//
    static nsString EXTRA_CNA;//
    nsString EXTRA_USSD;//
    static nsString EXTRA_REMOTE_URI;//
    nsString EXTRA_PARENT_CALL_ID;//
    nsString EXTRA_CHILD_NUMBER;//
    nsString EXTRA_CODEC;//
    nsString EXTRA_DISPLAY_TEXT;//
    nsString EXTRA_ADDITIONAL_CALL_INFO;//

    //
    //
    nsString EXTRA_OEM_EXTRAS;//

/*                                                                                          */
    nsString EXTRA_REDIREC_NUMBER;//
/*                                                                                          */

    int32_t mServiceType;
    int32_t mCallType;
    int32_t mRestrictCause = CALL_RESTRICT_CAUSE_NONE;
    Bundle *mCallExtras;
    ImsStreamMediaProfile *mMediaProfile;

    ImsCallProfile();
    virtual ~ImsCallProfile();
    ImsCallProfile(const android::Parcel &pParcel);
    ImsCallProfile(int32_t serviceType, int32_t callType);

    nsString getCallExtra(nsString& name);
    nsString getCallExtra(nsString& name, nsString& defaultValue);
    bool getCallExtraBoolean(nsString& name);
    bool getCallExtraBoolean(nsString& name, bool defaultValue);
    int32_t getCallExtraInt(nsString& name);
    int32_t getCallExtraInt(nsString& name, int32_t defaultValue);
    void setCallExtra(nsString& name, nsString& value);
    void setCallExtraBoolean(nsString& name, bool value);
    void setCallExtraInt(nsString& name, int32_t value);
    void updateCallType(ImsCallProfile *profile);
    void updateCallExtras(ImsCallProfile *profile);
    void readFromParcel(const android::Parcel &pParcel);
    void writeToParcel(android::Parcel &pParcel, int32_t flags);
    void copyFrom(ImsCallProfile *pCallProfile);

    int32_t getVideoStateFromImsCallProfile(ImsCallProfile *callProfile);
    int32_t getVideoStateFromCallType(int32_t callType);
    int32_t getCallTypeFromVideoState(int32_t videoState);
    int32_t presentationToOIR(int32_t presentation);
    int32_t OIRToPresentation(int32_t oir);
    bool isVideoPaused();

  private:
    bool isVideoStateSet(int32_t videoState, int32_t videoStateToCheck);
};


#endif //