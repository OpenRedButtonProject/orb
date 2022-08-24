/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
 
#ifndef __INJECTEDBUNDLE_UTILS_H
#define __INJECTEDBUNDLE_UTILS_H

#include "Module.h"

#include <WPE/WebKit.h>

#include <memory>
#include <vector>

namespace WPEFramework {
namespace WebKit {
    namespace Utils {
        void AppendStringToWKArray(const string& item, WKMutableArrayRef array);
        string GetStringFromWKArray(WKArrayRef array, unsigned int index);
        std::string GetURL();
        WKBundleRef GetBundle();
        string WKStringToString(WKStringRef wkStringRef);
        std::vector<string> ConvertWKArrayToStringVector(WKArrayRef array);
    };
}
}

#endif // __INJECTEDBUNDLE_UTILS_H
