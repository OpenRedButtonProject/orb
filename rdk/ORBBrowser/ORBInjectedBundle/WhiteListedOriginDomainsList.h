/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
 
#ifndef __WHITELISTEDORIGINDOMAINSLIST_H
#define __WHITELISTEDORIGINDOMAINSLIST_H

#include "Module.h"

#ifdef WEBKIT_GLIB_API
#include <wpe/webkit-web-extension.h>
typedef WebKitWebExtension* WKBundleRef;
#else
#include <WPE/WebKit.h>
#endif

#include <vector>

namespace WPEFramework {
namespace WebKit {

    class WhiteListedOriginDomainsList {
    private:
        WhiteListedOriginDomainsList(const WhiteListedOriginDomainsList&) = delete;
        WhiteListedOriginDomainsList& operator=(const WhiteListedOriginDomainsList&) = delete;

    public:
        typedef std::pair<bool, string> Domain;
        typedef std::vector<Domain> Domains;
        typedef std::map<string, Domains> WhiteMap;

    public:
        static std::unique_ptr<WhiteListedOriginDomainsList> RequestFromWPEFramework(const char* whitelist = nullptr);
        ~WhiteListedOriginDomainsList()
        {
        }

    public:
        void AddWhiteListToWebKit(WKBundleRef bundle);

    private:
        WhiteListedOriginDomainsList()
        {
        }

        WhiteMap _whiteMap;
    };
}
}

#endif // __WHITELISTEDORIGINDOMAINSLIST_H
