#ifndef __IXPCOM_UC_MEDIA_PROFILE_H_
#define __IXPCOM_UC_MEDIA_PROFILE_H_
#include <binder/Parcel.h>
#include <utils/String16.h>
#include "nsString.h"
#include "nsILgeImsUCManager.h"

class ImsStreamMediaProfile : public nsIImsStreamMediaProfile
{
  public:
    NS_DECL_ISUPPORTS
    NS_DECL_NSIIMSSTREAMMEDIAPROFILE

    /*

     */
    static const int32_t DIRECTION_INVALID = (-1);
    static const int32_t DIRECTION_INACTIVE = 0;
    static const int32_t DIRECTION_RECEIVE = 1;
    static const int32_t DIRECTION_SEND = 2;
    static const int32_t DIRECTION_SEND_RECEIVE = 3;

    /*

     */
    static const int32_t AUDIO_QUALITY_NONE = 0;
    static const int32_t AUDIO_QUALITY_AMR = 1;
    static const int32_t AUDIO_QUALITY_AMR_WB = 2;
    static const int32_t AUDIO_QUALITY_QCELP13K = 3;
    static const int32_t AUDIO_QUALITY_EVRC = 4;
    static const int32_t AUDIO_QUALITY_EVRC_B = 5;
    static const int32_t AUDIO_QUALITY_EVRC_WB = 6;
    static const int32_t AUDIO_QUALITY_EVRC_NW = 7;
    static const int32_t AUDIO_QUALITY_GSM_EFR = 8;
    static const int32_t AUDIO_QUALITY_GSM_FR = 9;
    static const int32_t AUDIO_QUALITY_GSM_HR = 10;

   /*

     */
    static const int32_t VIDEO_QUALITY_NONE = 0;
    static const int32_t VIDEO_QUALITY_QCIF = (1 << 0);
    static const int32_t VIDEO_QUALITY_QVGA_LANDSCAPE = (1 << 1);
    static const int32_t VIDEO_QUALITY_QVGA_PORTRAIT = (1 << 2);
    static const int32_t VIDEO_QUALITY_VGA_LANDSCAPE = (1 << 3);
    static const int32_t VIDEO_QUALITY_VGA_PORTRAIT = (1 << 4);

    //
    int32_t mAudioQuality;
    int32_t mAudioDirection;
    //
    int32_t mVideoQuality;
    int32_t mVideoDirection;

    ImsStreamMediaProfile();
    ImsStreamMediaProfile(const android::Parcel &pParcel);
    ImsStreamMediaProfile(int32_t audioQuality, int32_t audioDirection, int32_t videoQuality, int32_t videoDirection);
    virtual ~ImsStreamMediaProfile();
    void copyFrom(ImsStreamMediaProfile *profile);
    void readFromParcel(const android::Parcel &pParcel);
    void writeToParcel(android::Parcel &pParcel, int32_t flags);
};
#endif //