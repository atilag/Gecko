#ifndef __IXPCOM_UC_REASON_INFO_H_
#define __IXPCOM_UC_REASON_INFO_H_
#include <binder/Parcel.h>
#include <utils/String16.h>
#include "nsString.h"
#include "nsILgeImsUCManager.h"

class ImsReasonInfo: public nsIImsReasonInfo
{
  public:
    NS_DECL_ISUPPORTS
    NS_DECL_NSIIMSREASONINFO

    /*











     */
    static const int32_t TYPE_UNSPECIFIED = 0;
    static const int32_t TYPE_LOCAL = 1;
    static const int32_t TYPE_TIMEOUT = 2;
    static const int32_t TYPE_STATUSCODE = 3;
    static const int32_t TYPE_MEDIA = 4;
    static const int32_t TYPE_USER = 5;
    /*



     */
    static const int32_t TYPE_REGISTRATION = 101;
    static const int32_t TYPE_UT = 8;
    static const int32_t TYPE_CALL_DROP = 100;

    /*

     */
    static const int32_t CODE_UNSPECIFIED = 0;

    /*

     */
    //
    //
    static const int32_t CODE_LOCAL_ILLEGAL_ARGUMENT = 101;
    //
    static const int32_t CODE_LOCAL_ILLEGAL_STATE = 102;
    //
    static const int32_t CODE_LOCAL_INTERNAL_ERROR = 103;
    //
    static const int32_t CODE_LOCAL_IMS_SERVICE_DOWN = 106;
    //
    static const int32_t CODE_LOCAL_NO_PENDING_CALL = 107;

    //
    //
    static const int32_t CODE_LOCAL_POWER_OFF = 111;
    //
    static const int32_t CODE_LOCAL_LOW_BATTERY = 112;
    //
    static const int32_t CODE_LOCAL_NETWORK_NO_SERVICE = 121;
    //
    //
    static const int32_t CODE_LOCAL_NETWORK_NO_LTE_COVERAGE = 122;
    //
    static const int32_t CODE_LOCAL_NETWORK_ROAMING = 123;
    //
    static const int32_t CODE_LOCAL_NETWORK_IP_CHANGED = 124;
    //
    static const int32_t CODE_LOCAL_SERVICE_UNAVAILABLE = 131;
    //
    static const int32_t CODE_LOCAL_NOT_REGISTERED = 132;

    //
    //
    static const int32_t CODE_LOCAL_CALL_EXCEEDED = 141;
    //
    //
    static const int32_t CODE_LOCAL_CALL_BUSY = 142;
    //
    static const int32_t CODE_LOCAL_CALL_DECLINE = 143;
    //
    //
    static const int32_t CODE_LOCAL_CALL_VCC_ON_PROGRESSING = 144;
    //
    static const int32_t CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED = 145;
    //
    //
    static const int32_t CODE_LOCAL_CALL_CS_RETRY_REQUIRED = 146;
    //
    static const int32_t CODE_LOCAL_CALL_VOLTE_RETRY_REQUIRED = 147;
    //
    static const int32_t CODE_LOCAL_CALL_TERMINATED = 148;

    /*

     */
    //
    static const int32_t CODE_TIMEOUT_1XX_WAITING = 201;
    //
    //
    //
    static const int32_t CODE_TIMEOUT_NO_ANSWER = 202;
    //
    //
    //
    static const int32_t CODE_TIMEOUT_NO_ANSWER_CALL_UPDATE = 203;

    //
    static const int32_t CODE_FDN_BLOCKED = 241;

    /*

     */
    //
    //
    static const int32_t CODE_SIP_REDIRECTED = 321;
    //
    //
    static const int32_t CODE_SIP_BAD_REQUEST = 331;
    //
    static const int32_t CODE_SIP_FORBIDDEN = 332;
    //
    static const int32_t CODE_SIP_NOT_FOUND = 333;
    //
    //
    //
    static const int32_t CODE_SIP_NOT_SUPPORTED = 334;
    //
    static const int32_t CODE_SIP_REQUEST_TIMEOUT = 335;
    //
    static const int32_t CODE_SIP_TEMPRARILY_UNAVAILABLE = 336;
    //
    static const int32_t CODE_SIP_BAD_ADDRESS = 337;
    //
    //
    static const int32_t CODE_SIP_BUSY = 338;
    //
    static const int32_t CODE_SIP_REQUEST_CANCELLED = 339;
    //
    //
    //
    static const int32_t CODE_SIP_NOT_ACCEPTABLE = 340;
    //
    //
    static const int32_t CODE_SIP_NOT_REACHABLE = 341;
    //
    static const int32_t CODE_SIP_CLIENT_ERROR = 342;
    //
    //
    static const int32_t CODE_SIP_SERVER_INTERNAL_ERROR = 351;
    //
    static const int32_t CODE_SIP_SERVICE_UNAVAILABLE = 352;
    //
    static const int32_t CODE_SIP_SERVER_TIMEOUT = 353;
    //
    static const int32_t CODE_SIP_SERVER_ERROR = 354;
    //
    //
    static const int32_t CODE_SIP_USER_REJECTED = 361;
    //
    static const int32_t CODE_SIP_GLOBAL_ERROR = 362;
    //
    static const int32_t CODE_EMERGENCY_TEMP_FAILURE = 363;
    static const int32_t CODE_EMERGENCY_PERM_FAILURE = 364;

    /*

     */
    //
    static const int32_t CODE_MEDIA_INIT_FAILED = 401;
    //
    static const int32_t CODE_MEDIA_NO_DATA = 402;
    //
    static const int32_t CODE_MEDIA_NOT_ACCEPTABLE = 403;
    //
    static const int32_t CODE_MEDIA_UNSPECIFIED = 404;

    /*

     */
    //
    //
    static const int32_t CODE_USER_TERMINATED = 501;
    //
    static const int32_t CODE_USER_NOANSWER = 502;
    //
    static const int32_t CODE_USER_IGNORE = 503;
    //
    static const int32_t CODE_USER_DECLINE = 504;
    //
    static const int32_t CODE_LOW_BATTERY = 505;
    //
    static const int32_t CODE_BLACKLISTED_CALL_ID = 506;
    //
    //
    static const int32_t CODE_USER_TERMINATED_BY_REMOTE = 510;

    /*


     */
    //
    static const int32_t EXTRA_CODE_CALL_RETRY_NORMAL = 1;
    //
    static const int32_t EXTRA_CODE_CALL_RETRY_SILENT_REDIAL = 2;
    //
    static const int32_t EXTRA_CODE_CALL_RETRY_BY_SETTINGS = 3;

    /*

     */
    static const int32_t CODE_UT_NOT_SUPPORTED = 801;
    static const int32_t CODE_UT_SERVICE_UNAVAILABLE = 802;
    static const int32_t CODE_UT_OPERATION_NOT_ALLOWED = 803;
    static const int32_t CODE_UT_NETWORK_ERROR = 804;
    static const int32_t CODE_UT_CB_PASSWORD_MISMATCH = 821;

    /*

     */
    static const int32_t CODE_ECBM_NOT_SUPPORTED = 901;
    /*

     */
    static const int32_t CODE_REGISTRATION_ERROR = TYPE_REGISTRATION * 100;

    /*


     */
    static const int32_t CODE_ANSWERED_ELSEWHERE = 1014;

    /*


     */

    /*




     */
    static const int32_t CODE_CALL_DROP_IWLAN_TO_LTE_UNAVAILABLE = 10004;
    /*


     */
    nsString EXTRA_MSG_SERVICE_NOT_AUTHORIZED;    //

    //
    int32_t mReasonType;
    //
    int32_t mCode;
    //
    int32_t mExtraCode;
    //
    nsString mExtraMessage;

    ImsReasonInfo();
    ImsReasonInfo(const android::Parcel &pParcel);
    ImsReasonInfo(int32_t code, int32_t extraCode);
    ImsReasonInfo(int32_t code, int32_t extraCode, nsString extraMessage);
    virtual ~ImsReasonInfo();

    void readFromParcel(const android::Parcel &pParcel);
    void writeToParcel(android::Parcel &pParcel, int32_t flags);
    void copyFrom(ImsReasonInfo *pImsReasonInfo);
    void logIn();
};
#endif //