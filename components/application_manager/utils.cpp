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

#include "utils.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <string>

#include "ait.h"
#include "log.h"

/**
 * Make an invalid DVB triplet.
 * @return An invalid DVB triplet.
 */
Utils::S_DVB_TRIPLET Utils::MakeInvalidDvbTriplet()
{
    return {
        .originalNetworkId = INVALID_ID,
        .transportStreamId = INVALID_ID,
        .serviceId = INVALID_ID,
    };
}

/**
 * Returns true if the DVB triplet is invalid.
 * @param triplet The triplet
 * @return true if invalid, false otherwise.
 */
bool Utils::IsInvalidDvbTriplet(const S_DVB_TRIPLET &triplet)
{
    return triplet.originalNetworkId == INVALID_ID ||
           triplet.transportStreamId == INVALID_ID ||
           triplet.serviceId == INVALID_ID;
}

/**
 *
 * @param url
 * @param currentService
 * @return
 */
Utils::CreateLocatorInfo Utils::ParseCreateLocatorInfo(const std::string &url, const
    Utils::S_DVB_TRIPLET &currentService)
{
    Utils::CreateLocatorInfo urlInfo;
    urlInfo.parameters = url;
    urlInfo.orgId = 0;
    urlInfo.appId = 0;
    urlInfo.type = CreateLocatorType::UNKNOWN_LOCATOR;

    if (url.substr(0, 6) == "dvb://")
    {
        // Check if the URL is an Application Locator (TS 102 851)
        bool isAitFilterCurrentService = false;
        std::string urlParameters;
        size_t pos = url.find(".ait/");
        if (pos != std::string::npos)
        {
            urlParameters = url.substr(pos + 5);

            // Check if the ait filter (the string between dvb:// and .ait/) is the current service
            std::string sub = url.substr(6, pos - 6);
            if (sub == "current")
            {
                isAitFilterCurrentService = true;
            }
            else if (!IsInvalidDvbTriplet(currentService))
            {
                // original_network_id "." [ transport_stream_id ] "." service_id
                pos = sub.find('.');
                size_t pos2 = sub.find('.', pos + 1);
                if (pos != std::string::npos && pos2 != std::string::npos)
                {
                    std::string original_network_id = sub.substr(0, pos);
                    std::string transport_stream_id = sub.substr(pos + 1, pos2 - (pos + 1));
                    std::string service_id = sub.substr(pos2 + 1);
                    try
                    {
                        isAitFilterCurrentService =
                            (std::stoul(original_network_id, nullptr, 16)
                             == currentService.originalNetworkId) &&
                            ((transport_stream_id.empty()) ||
                             std::stoul(transport_stream_id, nullptr, 16)
                             == currentService.transportStreamId) &&
                            (std::stoul(service_id, nullptr, 16)
                             == currentService.serviceId);
                    }
                    catch (const std::exception &e)
                    {
                        isAitFilterCurrentService = false;
                        LOG(LOG_DEBUG, "Could not parse onet/tsid/sid");
                    }
                }
            }
        }

        if (isAitFilterCurrentService)
        {
            // Find query string or fragment remainder
            pos = urlParameters.find('?');
            if (pos == std::string::npos)
            {
                pos = urlParameters.find('#');
            }
            urlInfo.parameters = "";
            if (pos != std::string::npos)
            {
                urlInfo.parameters = urlParameters.substr(pos);
                urlParameters = urlParameters.substr(0, pos);
            }

            // Find AIT application parts (org_id "." app_id)
            pos = urlParameters.find('.');
            if (pos != std::string::npos)
            {
                std::string url_org_id = urlParameters.substr(0, pos);
                std::string url_app_id = urlParameters.substr(pos + 1);
                try
                {
                    urlInfo.orgId = std::stoul(url_org_id, nullptr, 16);
                    urlInfo.appId = std::stoul(url_app_id, nullptr, 16);
                    urlInfo.type = CreateLocatorType::AIT_APPLICATION_LOCATOR;
                }
                catch (const std::exception &e)
                {
                    urlInfo.orgId = 0;
                    urlInfo.appId = 0;
                    urlInfo.type = CreateLocatorType::UNKNOWN_LOCATOR;
                }
            }

            if (urlInfo.type != CreateLocatorType::AIT_APPLICATION_LOCATOR)
            {
                LOG(LOG_DEBUG, "Unknown URL: %s (could not parse org_id/app_id)", url.c_str());
            }
        }
        else
        {
            LOG(LOG_DEBUG, "Unknown URL: %s (not for current service)", url.c_str());
        }
    }
    else if (url.substr(0, 7) == "http://" || url.substr(0, 8) == "https://")
    {
        urlInfo.type = CreateLocatorType::ENTRY_PAGE_OR_XML_AIT_LOCATOR;
    }
    else
    {
        urlInfo.type = CreateLocatorType::UNKNOWN_LOCATOR;
        LOG(LOG_DEBUG, "Unknown URL: %s (unknown scheme)", url.c_str());
    }

    return urlInfo;
}

/**
 * Compares two URLs ignoring trailing '/' or string terminators
 * @param url1
 * @param url2
 * @return
 */
bool Utils::CompareUrls(const std::string &url1, const std::string &url2)
{
    uint32_t len1, len2;
    std::string str1 = url1;
    std::string str2 = url2;

    str1.erase(str1.find_last_not_of(" \t\n\r\f\v/") + 1);
    len1 = str1.length();

    str2.erase(str2.find_last_not_of(" \t\n\r\f\v/") + 1);
    len2 = str2.length();

    if (!str1.empty() && !str2.empty())
    {
        return(len1 == len2 && str1.compare(str2) == 0);
    }

    return false;
}

/**
 * Returns true if the specified document is contained in the specified application base URL
 * @param documentUrl
 * @param appBaseUrl
 * @return
 */
bool Utils::IsPartOf(const std::string &documentUrl, const std::string &appBaseUrl)
{
    uint32_t len2;
    std::string str1 = documentUrl;
    std::string str2 = appBaseUrl;

    str1.erase(str1.find_last_not_of(" \t\n\r\f\v/") + 1);
    str2.erase(str2.find_last_not_of(" \t\n\r\f\v/") + 1);
    len2 = str2.length();

    if (!str1.empty() && !str2.empty())
    {
        return str1.compare(0, str2.length(), str2) == 0;
    }

    return false;
}

/**
 * Returns a string containing the origin of the given URL as defined by RFC6454
 * <scheme>://<domain>:<port>
 * @param url
 * @return
 */
std::string Utils::StrGetUrlOrigin(const std::string &url)
{
    // TODO(C++-ize) This seems overly complex? We do something similar in Java
    uint8_t state = 0; // 0: scheme, 1: '://', 2: domain, 3: port, 4: end, 5: error
    int countChars = 0;
    const char *ptr;
    std::string retval;
    uint8_t portStr[6] = {0, 0, 0, 0, 0, 0};
    uint8_t schemeStr[6] = {0, 0, 0, 0, 0, 0};
    uint8_t portStrIndex = 0, scheme_str_index = 0;

    ptr = url.c_str();
    int length = url.length();
    while ((countChars < length) && (state < 4))
    {
        switch (state)
        {
            case 0: {
                if (*ptr == ':')
                {
                    state = 1;
                }
                else
                {
                    if (scheme_str_index < 6)
                    {
                        schemeStr[scheme_str_index++] = *ptr;
                    }
                    else
                    {
                        state = 5;
                        LOG(LOG_DEBUG, "Error parsing URL %s", url.c_str());
                    }
                }
                countChars++;
                ptr++;
                break;
            }
            case 1: {
                if (strncmp((char *)ptr, "//", 2) == 0)
                {
                    state = 2;
                    ptr += 2;
                    countChars += 2;
                }
                else
                {
                    state = 5;
                    LOG(LOG_DEBUG, "Error parsing URL %s", url.c_str());
                }
                break;
            }
            case 2: {
                if (*ptr == ':')
                {
                    state = 3;
                    countChars++;
                    ptr++;
                }
                else if (*ptr == '/')
                {
                    /* not incrementing count_chars to skip '/' */
                    state = 4;
                }
                else
                {
                    countChars++;
                    ptr++;
                }
                break;
            }
            case 3: {
                if (*ptr == '/')
                {
                    state = 4;
                }
                else if (portStrIndex > 5)
                {
                    state = 5;
                    LOG(LOG_DEBUG, "Error parsing URL %s", url.c_str());
                }
                else
                {
                    portStr[portStrIndex++] = *ptr;
                    countChars++;
                    ptr++;
                }

                break;
            }
        }
    }

    if (portStrIndex == 0)
    {
        if (strncmp((char *)schemeStr, "https", 5) == 0)
        {
            strcpy((char *)portStr, "443");
        }
        else
        {
            strcpy((char *)portStr, "80");
        }
    }

    if ((countChars > 0) && (countChars <= length) && (state != 5))
    {
        retval = url.substr(0, countChars);
        if (portStrIndex == 0)
        {
            char buffer[50];
            sprintf(buffer, ":%s", (char *)portStr);
            retval += buffer;
        }
    }

    return retval;
}

/**
 * Returns true if url is within app boundaries
 * @param url
 * @param appUri
 * @param appBoundaries
 * @param num_boundaries
 * @return
 */
bool Utils::CheckBoundaries(const std::string &url, const std::string &appUri,
    const std::vector<std::string> &appBoundaries)
{
    bool retval;
    std::string origin1 = Utils::StrGetUrlOrigin(url);
    std::string origin2 = Utils::StrGetUrlOrigin(appUri);
    retval = Utils::CompareUrls(origin1, origin2);
    if (!retval)
    {
        for (const auto & app_boundary : appBoundaries)
        {
            origin2 = Utils::StrGetUrlOrigin(app_boundary);
            retval = Utils::CompareUrls(origin1, origin2);
            if (retval)
            {
                break;
            }
        }
    }
    return retval;
}

/**
 *
 * @param base
 * @param locn
 * @param params
 * @return
 */
std::string Utils::MergeUrlParams(const std::string &base, const std::string &locn,
    const std::string &params)
{
    std::string result(base);
    std::string path = locn.substr(0, locn.find('#'));
    result += path;
    if (path.find('?') != std::string::npos)
    {
        // Has path params, replace ? for & in params
        std::string params2(params);
        std::replace(params2.begin(), params2.end(), '?', '&');
        result += params2;
    }
    else
    {
        result += params;
    }
    return result;
}


Utils::Timeout::Timeout(std::function<void(void)> callback) :
    m_callback(callback),
    m_stopped(true)
{ }

Utils::Timeout::~Timeout()
{
    stop();
}

void Utils::Timeout::start(std::chrono::milliseconds timeout)
{
    stop();
    m_startTimestamp = std::chrono::system_clock::now();
    m_stopped = false;
    m_timeout = timeout;
    m_thread = std::thread([&]
    {
        std::unique_lock<std::mutex> lock(m_cvm);
        if (!m_cv.wait_for(lock, m_timeout, [&] {
            return m_stopped;
        }))
        {
            m_callback();
        }
    });
}

void Utils::Timeout::stop()
{
    if (m_thread.joinable())
    {
        if (!m_stopped)
        {
            std::lock_guard<std::mutex> lock(m_cvm);
            m_stopped = true;
        }
        m_cv.notify_all();
        m_thread.join();
    }
}

std::chrono::milliseconds Utils::Timeout::elapsed() const
{
    std::lock_guard<std::mutex> lock(m_cvm);
    if (m_stopped)
    {
        return std::chrono::milliseconds(0);
    }
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_startTimestamp);
}

std::chrono::milliseconds Utils::Timeout::remaining() const
{
    std::lock_guard<std::mutex> lock(m_cvm);
    if (m_stopped)
    {
        return std::chrono::milliseconds(0);
    }
    return m_timeout - elapsed();
}

bool Utils::Timeout::isStopped() const
{
    std::lock_guard<std::mutex> lock(m_cvm);
    return m_stopped;
}