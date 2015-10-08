#ifndef __IMS_CONF_STATE_H_
#define __IMS_CONF_STATE_H_
#include <binder/Parcel.h>
#include <utils/String16.h>
#include "nsString.h"
#include "Bundle.h"
#include <map>
#include "nsTArray.h"
#include "nsComponentManagerUtils.h"
#include "nsILgeImsUCManager.h"
/*



 */
class ImsConferenceState: public nsIImsConferenceState
{
  public:
    NS_DECL_ISUPPORTS
    NS_DECL_NSIIMSCONFERENCESTATE
    /*

     */
    //
    nsString USER;//
    //
    nsString DISPLAY_TEXT;//
    //
    nsString ENDPOINT;//
    //
    nsString STATUS;//
    std::map<nsString, int32_t> Confstate;
    /*






















     */
    nsString STATUS_PENDING;//
    nsString STATUS_DIALING_OUT;//
    nsString STATUS_DIALING_IN;//
    nsString STATUS_ALERTING;//
    nsString STATUS_ON_HOLD;//
    nsString STATUS_CONNECTED;//
    nsString STATUS_DISCONNECTING;//
    nsString STATUS_DISCONNECTED;//
    nsString STATUS_MUTED_VIA_FOCUS;//
    nsString STATUS_CONNECT_FAIL;//

    /*

     */
    nsString SIP_STATUS_CODE;//

    std::map<nsString, Bundle *> mParticipants;

    ImsConferenceState();
    ImsConferenceState(const android::Parcel &pParcel);
    virtual ~ImsConferenceState();
    int describeContents();
    void readFromParcel(const android::Parcel &pParcel);
    void writeToParcel(android::Parcel &pParcel, int32_t flags);
    int getConnectionStateForStatus(const nsString& status);
};

#endif //