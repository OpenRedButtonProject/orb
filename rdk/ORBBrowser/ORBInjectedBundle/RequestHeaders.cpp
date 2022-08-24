/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "RequestHeaders.h"

#include <WPE/WebKit.h>
#include <WPE/WebKit/WKType.h>
#include <WPE/WebKit/WKString.h>
#include <WPE/WebKit/WKBundleFrame.h>
#include <WPE/WebKit/WKURL.h>

#include <interfaces/json/JsonData_WebKitBrowser.h>

#include <core/JSON.h>

#include "Utils.h"

namespace WPEFramework {
namespace WebKit {

namespace
{

typedef std::vector<std::pair<std::string, std::string>> Headers;
typedef std::unordered_map<WKBundlePageRef, Headers> PageHeaders;
static PageHeaders s_pageHeaders;

bool ParseHeaders(const string& json, Headers& out)
{
    Core::OptionalType<Core::JSON::Error> error;
    Core::JSON::ArrayType<JsonData::WebKitBrowser::HeadersData> array;
    if (array.FromString(json, error)) {
        for (auto it = array.Elements(); it.Next();) {
            if (!it.IsValid())
                continue;
            const auto &data  = it.Current();
            out.emplace_back(data.Name.Value(), data.Value.Value());
            TRACE_GLOBAL(Trace::Information, (_T("header: '%s: %s'\n"), data.Name.Value().c_str(), data.Value.Value().c_str()));
        }
        return true;
    } else {
        TRACE_GLOBAL(Trace::Error,
                     (_T("Failed to parse headers array, error='%s', json='%s'\n"),
                      (error.IsSet() ? error.Value().Message().c_str() : "unknown"), json.c_str()));
    }

    return false;
}

} // namespace

void RemoveRequestHeaders(WKBundlePageRef page)
{
    s_pageHeaders.erase(page);
}

void SetRequestHeaders(WKBundlePageRef page, WKTypeRef messageBody)
{
    if (WKGetTypeID(messageBody) != WKStringGetTypeID())
        return;

    string message = WPEFramework::WebKit::Utils::WKStringToString(static_cast<WKStringRef>(messageBody));
    if (message.empty()) {
        RemoveRequestHeaders(page);
        return;
    }

    Headers newHeaders;
    if (ParseHeaders(message, newHeaders)) {
        if (newHeaders.empty())
            RemoveRequestHeaders(page);
        else
            s_pageHeaders[page] = std::move(newHeaders);
    }
}

void ApplyRequestHeaders(WKBundlePageRef page, WKURLRequestRef requestRef)
{
    auto it = s_pageHeaders.find(page);
    if (it == s_pageHeaders.end())
        return;

    for (const auto& h : it->second) {
        auto key = WKStringCreateWithUTF8CString(h.first.c_str());
        auto value = WKStringCreateWithUTF8CString(h.second.c_str());
        WKURLRequestSetHTTPHeaderField(requestRef, key, value);
        WKRelease(key);
        WKRelease(value);
    }
}

}  // WebKit
}  // WPEFramework
