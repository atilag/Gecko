#ifndef LGE_IMS_BUNDLE_H
#define LGE_IMS_BUNDLE_H

#include <string.h>
#include <strings.h>
#include <map>
#include "nsTArray.h"
#include "nsString.h"
#include <binder/Parcel.h>
#include <utils/String16.h>

class Bundle
{
  public:
    Bundle();
    Bundle(const android::Parcel &pParcel);
    ~Bundle();

    std::map<nsString, bool> mBool;
    std::map<nsString, int32_t> mInt;
    std::map<nsString, nsString> mString;

    nsString getString(nsString& str, nsString& defaultVal);
    bool getBoolean(nsString& str, bool defaultVal);
    int32_t getInt(nsString& str, int32_t defaultVal);

    void putString(const nsString& str, nsString& ret);
    void putBoolean(const nsString& str, bool ret);
    void putInt(const nsString& str, int32_t ret);

    void clear();
    void clone(Bundle *bundle);

    void writeToParcel(android::Parcel &dest, int32_t flags);
    void readFromParcel(const android::Parcel &source);
};


#endif //