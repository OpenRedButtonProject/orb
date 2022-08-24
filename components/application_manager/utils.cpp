/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
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
        .original_network_id = INVALID_ID,
        .transport_stream_id = INVALID_ID,
        .service_id = INVALID_ID,
    };
}

/**
 * Returns true if the DVB triplet is invalid.
 * @param triplet The triplet
 * @return true if invalid, false otherwise.
 */
bool Utils::IsInvalidDvbTriplet(const S_DVB_TRIPLET &triplet)
{
    return triplet.original_network_id == INVALID_ID ||
           triplet.transport_stream_id == INVALID_ID ||
           triplet.service_id == INVALID_ID;
}

/**
 *
 * @param url
 * @param current_service
 * @return
 */
Utils::CreateLocatorInfo Utils::ParseCreateLocatorInfo(const std::string &url, const
    Utils::S_DVB_TRIPLET &current_service)
{
    Utils::CreateLocatorInfo url_info;
    url_info.parameters = url;
    url_info.org_id = 0;
    url_info.app_id = 0;
    url_info.type = CreateLocatorType::UNKNOWN_LOCATOR;

    if (url.substr(0, 6) == "dvb://")
    {
        // Check if the URL is an Application Locator (TS 102 851)
        bool is_ait_filter_current_service = false;
        std::string url_parameters;
        size_t pos = url.find(".ait/");
        if (pos != std::string::npos)
        {
            url_parameters = url.substr(pos + 5);

            // Check if the ait filter (the string between dvb:// and .ait/) is the current service
            std::string sub = url.substr(6, pos - 6);
            if (sub == "current")
            {
                is_ait_filter_current_service = true;
            }
            else if (!IsInvalidDvbTriplet(current_service))
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
                        is_ait_filter_current_service =
                            (std::stoul(original_network_id, nullptr, 16)
                             == current_service.original_network_id) &&
                            ((transport_stream_id.empty()) ||
                             std::stoul(transport_stream_id, nullptr, 16)
                             == current_service.transport_stream_id) &&
                            (std::stoul(service_id, nullptr, 16)
                             == current_service.service_id);
                    }
                    catch (const std::exception &e)
                    {
                        is_ait_filter_current_service = false;
                        LOG(LOG_DEBUG, "Could not parse onet/tsid/sid");
                    }
                }
            }
        }

        if (is_ait_filter_current_service)
        {
            // Find query string or fragment remainder
            pos = url_parameters.find('?');
            if (pos == std::string::npos)
            {
                pos = url_parameters.find('#');
            }
            url_info.parameters = "";
            if (pos != std::string::npos)
            {
                url_info.parameters = url_parameters.substr(pos);
                url_parameters = url_parameters.substr(0, pos);
            }

            // Find AIT application parts (org_id "." app_id)
            pos = url_parameters.find('.');
            if (pos != std::string::npos)
            {
                std::string url_org_id = url_parameters.substr(0, pos);
                std::string url_app_id = url_parameters.substr(pos + 1);
                try
                {
                    url_info.org_id = std::stoul(url_org_id, nullptr, 16);
                    url_info.app_id = std::stoul(url_app_id, nullptr, 16);
                    url_info.type = CreateLocatorType::AIT_APPLICATION_LOCATOR;
                }
                catch (const std::exception &e)
                {
                    url_info.org_id = 0;
                    url_info.app_id = 0;
                    url_info.type = CreateLocatorType::UNKNOWN_LOCATOR;
                }
            }

            if (url_info.type != CreateLocatorType::AIT_APPLICATION_LOCATOR)
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
        url_info.type = CreateLocatorType::ENTRY_PAGE_OR_XML_AIT_LOCATOR;
    }
    else
    {
        url_info.type = CreateLocatorType::UNKNOWN_LOCATOR;
        LOG(LOG_DEBUG, "Unknown URL: %s (unknown scheme)", url.c_str());
    }

    return url_info;
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
 * @param document_url
 * @param app_base_url
 * @return
 */
bool Utils::IsPartOf(const std::string &document_url, const std::string &app_base_url)
{
    uint32_t len2;
    std::string str1 = document_url;
    std::string str2 = app_base_url;

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
    uint32_t count_chars = 0;
    const char *ptr;
    std::string retval;
    uint8_t port_str[6] = {0, 0, 0, 0, 0, 0};
    uint8_t scheme_str[6] = {0, 0, 0, 0, 0, 0};
    uint8_t port_str_index = 0, scheme_str_index = 0;

    ptr = url.c_str();
    int length = url.length();
    while ((count_chars < length) && (state < 4))
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
                        scheme_str[scheme_str_index++] = *ptr;
                    }
                    else
                    {
                        state = 5;
                        LOG(LOG_DEBUG, "Error parsing URL %s", url.c_str());
                    }
                }
                count_chars++;
                ptr++;
                break;
            }
            case 1: {
                if (strncmp((char *)ptr, "//", 2) == 0)
                {
                    state = 2;
                    ptr += 2;
                    count_chars += 2;
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
                    count_chars++;
                    ptr++;
                }
                else if (*ptr == '/')
                {
                    /* not incrementing count_chars to skip '/' */
                    state = 4;
                }
                else
                {
                    count_chars++;
                    ptr++;
                }
                break;
            }
            case 3: {
                if (*ptr == '/')
                {
                    state = 4;
                }
                else if (port_str_index > 5)
                {
                    state = 5;
                    LOG(LOG_DEBUG, "Error parsing URL %s", url.c_str());
                }
                else
                {
                    port_str[port_str_index++] = *ptr;
                    count_chars++;
                    ptr++;
                }

                break;
            }
        }
    }

    if (port_str_index == 0)
    {
        if (strncmp((char *)scheme_str, "https", 5) == 0)
        {
            strcpy((char *)port_str, "443");
        }
        else
        {
            strcpy((char *)port_str, "80");
        }
    }

    if ((count_chars > 0) && (count_chars <= length) && (state != 5))
    {
        retval = url.substr(0, count_chars);
        if (port_str_index == 0)
        {
            char buffer[50];
            sprintf(buffer, ":%s", (char *)port_str);
            retval += buffer;
        }
    }

    return retval;
}

/**
 * Returns true if url is within app boundaries
 * @param url
 * @param app_uri
 * @param app_boundaries
 * @param num_boundaries
 * @return
 */
bool Utils::CheckBoundaries(const std::string &url, const std::string &app_uri,
    const std::vector<std::string> &app_boundaries)
{
    bool retval;
    std::string origin1 = Utils::StrGetUrlOrigin(url);
    std::string origin2 = Utils::StrGetUrlOrigin(app_uri);
    retval = Utils::CompareUrls(origin1, origin2);
    if (!retval)
    {
        for (const auto & app_boundary : app_boundaries)
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
