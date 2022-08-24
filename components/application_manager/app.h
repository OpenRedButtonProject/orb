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
        const Utils::S_DVB_TRIPLET current_service,
        bool is_network_available,
        const std::string &url_params,
        bool is_broadcast,
        bool is_trusted);

    std::string entry_url;
    std::string loaded_url;
    std::string base_url;

    uint16_t protocol_id;
    uint8_t control_code;
    uint32_t org_id;
    uint16_t app_id;

    uint16_t key_set_mask;

    bool is_trusted;
    bool is_broadcast;
    bool is_service_bound;
    bool is_hidden;

    std::vector<std::string> boundaries;

    bool is_running = false;
    uint16_t id;
    std::vector<Ait::S_APP_PARENTAL_RATING> parental_ratings;
};

#endif // HBBTV_SERVICE_APP_H
