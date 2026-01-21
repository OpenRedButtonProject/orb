/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * General utilities class
 *
 * Note: This file is part of the platform-agnostic application manager library.
 */

#ifndef __HBBTV_UTILS_H
#define __HBBTV_UTILS_H

#include <cstdint>
#include <vector>
#include <string>

#include <functional>
#include <chrono>
#include <thread>
#include <condition_variable>

#define INVALID_ID 0xFFFF

namespace orb
{

class Utils {
public:
    enum class CreateLocatorType
    {
        UNKNOWN_LOCATOR,
        AIT_APPLICATION_LOCATOR,
        ENTRY_PAGE_OR_XML_AIT_LOCATOR,
        OPAPP_PACKAGE_LOCATOR  // hbbtv-package://appid.orgid (TS 103 606 Section 9.4.1)
    };

    typedef struct
    {
        uint16_t originalNetworkId;
        uint16_t transportStreamId;
        uint16_t serviceId;
    } S_DVB_TRIPLET;

    struct CreateLocatorInfo
    {
        CreateLocatorType type;
        uint32_t orgId;
        uint16_t appId;
        std::string parameters;
    };

    enum { AIT_TIMEOUT = 5000 };

    /**
     * Make an invalid DVB triplet.
     * @return An invalid DVB triplet.
     */
    static S_DVB_TRIPLET MakeInvalidDvbTriplet();

    /**
     * Returns true if the DVB triplet is invalid.
     * @param triplet The triplet
     * @return true if invalid, false otherwise.
     */
    static bool IsInvalidDvbTriplet(const S_DVB_TRIPLET &triplet);

    /**
     * Returns a string containing the origin of the given URL as defined by RFC6454
     * <scheme>://<domain>:<port>
     * @param url
     * @return
     */
    static std::string StrGetUrlOrigin(const std::string &url);

    /**
     * Parses HbbTV/DVB application locator URLs and extracts application metadata.
     *
     * Handles four URL formats:
     * - DVB AIT Application Locator (TS 102 851): dvb://[onid.tsid.]sid.ait/orgId.appId[?params][#fragment]
     *   or dvb://current.ait/orgId.appId - validates the AIT filter matches currentService
     * - OpApp Package Locator (TS 103 606 Section 9.4.1): hbbtv-package://appid.orgid[/path]
     *   where appid and orgid are lowercase hex with no leading zeros; path stored in parameters
     * - HTTP/HTTPS URLs: marked as ENTRY_PAGE_OR_XML_AIT_LOCATOR
     * - Other schemes: marked as UNKNOWN_LOCATOR
     *
     * @param url The URL to parse
     * @param currentService The DVB triplet of the currently tuned service, used to validate
     *                       that DVB AIT locators reference the current broadcast context
     * @return CreateLocatorInfo containing the locator type, orgId, appId, and any URL parameters
     */
    static Utils::CreateLocatorInfo ParseCreateLocatorInfo(const std::string &url, const
        Utils::S_DVB_TRIPLET &currentService);

    /**
     * Compares two URLs ignoring trailing '/' or string terminators
     * @param url1
     * @param url2
     * @return
     */
    static bool CompareUrls(const std::string &url1, const std::string &url2);

    /**
     * Returns true if the specified document is contained in the specified application base URL
     * @param documentUrl
     * @param appBaseUrl
     * @return
     */
    static bool IsPartOf(const std::string &documentUrl, const std::string &appBaseUrl);

    /**
     * Returns true if url is within app boundaries
     * @param url
     * @param appUri
     * @param appBoundaries
     * @return
     */
    static bool CheckBoundaries(const std::string &url, const std::string& appUri, const
        std::vector<std::string>& appBoundaries);

    /**
     *
     * @param base
     * @param locn
     * @param params
     * @return
     */
    static std::string MergeUrlParams(const std::string &base, const std::string &locn, const
        std::string &params);

    class Timeout {
public:
        Timeout(std::function<void(void)> callback);
        ~Timeout();

        Timeout(const Timeout&) = delete;
        Timeout& operator=(const Timeout&) = delete;

        void start(std::chrono::milliseconds timeout);

        void stop();

        std::chrono::milliseconds elapsed() const;

        std::chrono::milliseconds remaining() const;

        bool isStopped() const;

private:
        std::function<void(void)> m_callback;
        bool m_stopped;
        std::thread m_thread;
        std::condition_variable m_cv;
        mutable std::mutex m_cvm;
        std::chrono::system_clock::time_point m_startTimestamp;
        std::chrono::milliseconds m_timeout;
    };
};

} // namespace orb

#endif // __HBBTV_UTILS_H
