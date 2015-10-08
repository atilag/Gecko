#ifndef __IXPCOM_UC_IMS_ICALL_H_
#define __IXPCOM_UC_IMS_ICALL_H_
#include "nsString.h"

using namespace android ;
class ICall
{
  public:
    /*

     */
    virtual void close() = 0;
    /*




     */
    virtual bool checkIfRemoteUserIsSame(const nsString& userId) = 0;
    /*




     */
    virtual bool equalsTo(ICall *call) = 0;
};
#endif //