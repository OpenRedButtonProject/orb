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

    std::string getScheme() const;
    void setScheme(std::string value);

    std::string entryUrl;
    std::string loadedUrl;
    std::string baseUrl;

    uint16_t protocolId;
    uint8_t controlCode;
    uint32_t orgId;
    uint16_t appId;

    uint16_t keySetMask;
    std::vector<uint16_t> otherKeys;

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
    std::vector<uint16_t> graphicsConstraints;
    uint8_t versionMinor;
private:
    std::string m_scheme;
};

#endif // HBBTV_SERVICE_APP_H
