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

class Utils {
public:
    enum class CreateLocatorType
    {
        UNKNOWN_LOCATOR,
        AIT_APPLICATION_LOCATOR,
        ENTRY_PAGE_OR_XML_AIT_LOCATOR
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
     *
     * @param url
     * @param currentService
     * @return
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
        Timeout(std::function<void(void)> callback, std::chrono::milliseconds timeout) :
            m_callback(callback),
            m_timeout(timeout),
            m_stopped(true)
        {
        }

        ~Timeout()
        {
            stop();
        }

        Timeout(const Timeout&) = delete;

        Timeout& operator=(const Timeout&) = delete;

        void start()
        {
            stop();
            m_stopped = false;
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

        void stop()
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

private:
        std::function<void(void)> m_callback;
        std::chrono::milliseconds m_timeout;
        bool m_stopped;
        std::thread m_thread;
        std::condition_variable m_cv;
        std::mutex m_cvm;
    };
};

#endif // __HBBTV_UTILS_H
