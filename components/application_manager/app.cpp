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

#include "app.h"
#include "log.h"

static std::string getAppSchemeFromUrlParams(const std::string &urlParams);
static std::string getUrlParamsFromAppScheme(const std::string &scheme);

App App::CreateAppFromUrl(const std::string &url)
{
    App app;

    app.baseUrl = url;
    app.entryUrl = url;
    app.loadedUrl = url;

    app.protocolId = 0;
    app.controlCode = 0;
    app.orgId = 0;
    app.appId = 0;

    app.keySetMask = 0;
    app.otherKeys = std::vector<uint16_t>();

    app.isTrusted = false;
    app.isBroadcast = false;
    app.isServiceBound = false;
    app.isHidden = false;

    app.isRunning = !app.entryUrl.empty();
    app.setScheme(getAppSchemeFromUrlParams(url));

    return app;
}

App App::CreateAppFromAitDesc(const Ait::S_AIT_APP_DESC *desc,
    const Utils::S_DVB_TRIPLET currentService,
    bool isNetworkAvailable,
    const std::string &urlParams,
    bool isBroadcast,
    bool isTrusted)
{
    App app;
    app.versionMinor = INT8_MAX;

    app.baseUrl = Ait::GetBaseURL(desc, currentService, isNetworkAvailable, &app.protocolId);

    app.entryUrl = Utils::MergeUrlParams(app.baseUrl, desc->location, urlParams);
    app.loadedUrl = app.entryUrl;

    app.controlCode = desc->controlCode;
    app.orgId = desc->orgId;
    app.appId = desc->appId;
    app.graphicsConstraints = desc->graphicsConstraints;

    app.keySetMask = 0;
    app.otherKeys = std::vector<uint16_t>();

    app.isTrusted = isTrusted;
    app.isBroadcast = isBroadcast;
    app.isServiceBound = desc->appDesc.serviceBound;
    app.isHidden = isBroadcast; // Broadcast-related applications need to call show.
    app.parentalRatings = desc->parentalRatings;

    for (uint8_t i = 0; i < desc->appDesc.appProfiles.size(); i++)
    {
        if (app.versionMinor >= desc->appDesc.appProfiles[i].versionMinor)
        {
            app.versionMinor = desc->appDesc.appProfiles[i].versionMinor;
        }
    }

    /* AUTOSTARTED apps are activated when they receive a key event */
    app.isActivated = !(desc->controlCode == Ait::APP_CTL_AUTOSTART);

    for (uint8_t i = 0; i < desc->appName.numLangs; i++)
    {
        app.names[desc->appName.names[i].langCode] = desc->appName.names[i].name;
    }

    app.setScheme(desc->scheme);
    if (!desc->scheme.empty())
    {
        app.entryUrl = Utils::MergeUrlParams("", app.entryUrl,
                                             getUrlParamsFromAppScheme(app.getScheme()));
        app.loadedUrl = app.entryUrl;
    }

    return app;
}

std::string App::getScheme() const {
    if (!m_scheme.empty()) {
        return m_scheme;
    }
    return LINKED_APP_SCHEME_1_1;
}

void App::setScheme(std::string value) {
    m_scheme = value;
}

std::string getAppSchemeFromUrlParams(const std::string &urlParams)
{
    if (urlParams.find("lloc=service") != std::string::npos)
    {
        return LINKED_APP_SCHEME_1_2;
    }
    if (urlParams.find("lloc=availability") != std::string::npos)
    {
        return LINKED_APP_SCHEME_2;
    }
    return LINKED_APP_SCHEME_1_1;
}

std::string getUrlParamsFromAppScheme(const std::string &scheme)
{
    if (scheme == LINKED_APP_SCHEME_1_2)
    {
        return "?lloc=service";
    }
    if (scheme == LINKED_APP_SCHEME_2)
    {
        return "?lloc=availability";
    }
    return "";
}
