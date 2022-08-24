/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "Utils.h"

using std::unique_ptr;
using std::vector;

namespace WPEFramework {
namespace WebKit {
namespace Utils {
// Adds string to WKMutableArray.
void AppendStringToWKArray(const string& item, WKMutableArrayRef array)
{
    WKStringRef itemString = WKStringCreateWithUTF8CString(item.c_str());
    WKArrayAppendItem(array, itemString);
    WKRelease(itemString);
}

// Reads string from WKArray.
string GetStringFromWKArray(WKArrayRef array, unsigned int index)
{
    WKStringRef itemString = static_cast<WKStringRef>(WKArrayGetItemAtIndex(array, index));
    return WKStringToString(itemString);
}

// Converts WKString to string.
string WKStringToString(WKStringRef wkStringRef)
{
    size_t bufferSize = WKStringGetMaximumUTF8CStringSize(wkStringRef);
    std::unique_ptr<char[]> buffer(new char[bufferSize]);
    size_t stringLength = WKStringGetUTF8CString(wkStringRef, buffer.get(), bufferSize);
    return Core::ToString(buffer.get(), stringLength - 1);
}

// Converts WKArray to string vector.
vector<string> ConvertWKArrayToStringVector(WKArrayRef array)
{
    size_t arraySize = WKArrayGetSize(array);

    vector<string> stringVector;

    for (unsigned int index = 0; index < arraySize; ++index)
    {
        stringVector.push_back(GetStringFromWKArray(array, index));
    }

    return stringVector;
}
}
}
}
