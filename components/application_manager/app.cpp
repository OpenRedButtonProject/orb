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

#include "app.h"
#include "log.h"

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

    app.isTrusted = false;
    app.isBroadcast = false;
    app.isServiceBound = false;
    app.isHidden = false;

    app.isRunning = !app.entryUrl.empty();

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

    app.baseUrl = Ait::GetBaseURL(desc, currentService, isNetworkAvailable, &app.protocolId);

    app.entryUrl = Utils::MergeUrlParams(app.baseUrl, desc->location, urlParams);
    app.loadedUrl = app.entryUrl;

    app.controlCode = desc->controlCode;
    app.orgId = desc->orgId;
    app.appId = desc->appId;

    app.keySetMask = 0;

    app.isTrusted = isTrusted;
    app.isBroadcast = isBroadcast;
    app.isServiceBound = desc->appDesc.serviceBound;
    app.isHidden = isBroadcast; // Broadcast-related applications need to call show.
    app.parentalRatings = desc->parentalRatings;

    /* AUTOSTARTED apps are activated when they receive a key event */
    app.isActivated = !(desc->controlCode == Ait::APP_CTL_AUTOSTART);

    for (uint8_t i = 0; i < desc->appName.numLangs; i++)
    {
        app.names[desc->appName.names[i].langCode] = desc->appName.names[i].name;
    }

    return app;
}
