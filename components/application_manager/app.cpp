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

    app.base_url = url;
    app.entry_url = url;
    app.loaded_url = url;

    app.protocol_id = 0;
    app.control_code = 0;
    app.org_id = 0;
    app.app_id = 0;

    app.key_set_mask = 0;

    app.is_trusted = false;
    app.is_broadcast = false;
    app.is_service_bound = false;
    app.is_hidden = false;

    app.is_running = !app.entry_url.empty();

    return app;
}

App App::CreateAppFromAitDesc(const Ait::S_AIT_APP_DESC *desc,
    const Utils::S_DVB_TRIPLET current_service,
    bool is_network_available,
    const std::string &url_params,
    bool is_broadcast,
    bool is_trusted)
{
    App app;

    app.base_url = Ait::GetBaseURL(desc, current_service, is_network_available, &app.protocol_id);

    app.entry_url = Utils::MergeUrlParams(app.base_url, desc->location, url_params);
    app.loaded_url = app.entry_url;

    app.control_code = desc->control_code;
    app.org_id = desc->org_id;
    app.app_id = desc->app_id;

    app.key_set_mask = 0;

    app.is_trusted = is_trusted;
    app.is_broadcast = is_broadcast;
    app.is_service_bound = desc->app_desc.service_bound;
    app.is_hidden = is_broadcast; // Broadcast-related applications need to call show.
    app.parental_ratings = desc->parental_ratings;

    /* AUTOSTARTED apps are activated when they receive a key event */
    app.is_activated = !(desc->control_code == Ait::APP_CTL_AUTOSTART);

    for (uint8_t i = 0; i < desc->app_name.num_langs; i++)
    {
        app.names[desc->app_name.names[i].lang_code] = desc->app_name.names[i].name;
    }

    return app;
}
