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
        uint16_t original_network_id;
        uint16_t transport_stream_id;
        uint16_t service_id;
    } S_DVB_TRIPLET;

    struct CreateLocatorInfo
    {
        CreateLocatorType type;
        uint32_t org_id;
        uint16_t app_id;
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
     * @param current_service
     * @return
     */
    static Utils::CreateLocatorInfo ParseCreateLocatorInfo(const std::string &url, const
        Utils::S_DVB_TRIPLET &current_service);

    /**
     * Compares two URLs ignoring trailing '/' or string terminators
     * @param url1
     * @param url2
     * @return
     */
    static bool CompareUrls(const std::string &url1, const std::string &url2);

    /**
     * Returns true if the specified document is contained in the specified application base URL
     * @param document_url
     * @param app_base_url
     * @return
     */
    static bool IsPartOf(const std::string &document_url, const std::string &app_base_url);

    /**
     * Returns true if url is within app boundaries
     * @param url
     * @param app_uri
     * @param app_boundaries
     * @return
     */
    static bool CheckBoundaries(const std::string &url, const std::string& app_uri, const
        std::vector<std::string>& app_boundaries);

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
            callback_(callback),
            timeout_(timeout),
            stopped_(true)
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
            stopped_ = false;
            thread_ = std::thread([&]
            {
                std::unique_lock<std::mutex> lock(cvm_);
                if (!cv_.wait_for(lock, timeout_, [&] {
                    return stopped_;
                }))
                {
                    callback_();
                }
            });
        }

        void stop()
        {
            if (thread_.joinable())
            {
                if (!stopped_)
                {
                    std::lock_guard<std::mutex> lock(cvm_);
                    stopped_ = true;
                }
                cv_.notify_all();
                thread_.join();
            }
        }

private:
        std::function<void(void)> callback_;
        std::chrono::milliseconds timeout_;
        bool stopped_;
        std::thread thread_;
        std::condition_variable cv_;
        std::mutex cvm_;
    };
};

#endif // __HBBTV_UTILS_H
