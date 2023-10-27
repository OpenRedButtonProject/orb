/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 *
 * App model
 *
 * Note: This file is part of the platform-agnostic application manager library.
 */

#ifndef HBBTV_SERVICE_APP_H
#define HBBTV_SERVICE_APP_H

#include <memory>
#include <vector>
#include <map>
#include <cstdint>

#include "utils.h"
#include "ait.h"
#include "app.h"

class App
{
public:
    /**
     *
     * @param url
     */
    static App CreateAppFromUrl(const std::string &url);

    static App CreateAppFromAitDesc(const Ait::S_AIT_APP_DESC *desc,
        const Utils::S_DVB_TRIPLET currentService,
        bool isNetworkAvailable,
        const std::string &urlParams,
        bool isBroadcast,
        bool isTrusted);

    std::string entryUrl;
    std::string loadedUrl;
    std::string baseUrl;

    uint16_t protocolId;
    uint8_t controlCode;
    uint32_t orgId;
    uint16_t appId;

    uint16_t keySetMask;

    bool isTrusted;
    bool isBroadcast;
    bool isServiceBound;
    bool isHidden;

    std::vector<std::string> boundaries;
    std::map<uint32_t, std::string> names;

    bool isRunning = false;
    /* Activated by default. Deactivate if they are AUTOSTARTED */
    bool isActivated = true;
    uint16_t id;
    std::vector<Ait::S_APP_PARENTAL_RATING> parentalRatings;
    uint8_t versionMinor;
};

#endif // HBBTV_SERVICE_APP_H
