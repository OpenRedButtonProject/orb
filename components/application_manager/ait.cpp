/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 *
 * AIT parsing
 *
 * Note: This file is part of the platform-agnostic application manager library.
 */

#include "ait.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <pthread.h>

#include "log.h"
#include "utils.h"
#include "application_manager.h"

#define DTAG_APP_DESC 0x00
#define DTAG_APP_NAME 0x01
#define DTAG_TRANSPORT_PROTOCOL 0x02
#define DTAG_EXT_AUTH 0x05
#define DTAG_APPLICATION_ICON 0x0b
#define DTAG_SIMPLE_APP_LOCATION 0x15
#define DTAG_APP_USAGE 0x16
#define DTAG_SIMPLE_APP_BOUNDARY 0x17
#define DTAG_PARENTAL_RATING 0x55

#define GET_SECTION_MASK_INDEX(section_number) static_cast<unsigned int>(section_number / 8u)
#define GET_SECTION_MASK_SHIFT(section_number) static_cast<unsigned int>(section_number % 8u)

/**
 * Get the last completed AIT table. This value may be invalidated by calling ProcessSection(),
 * consumers of this API should ensure serialization.
 * @return An AIT table or nullptr.
 */
Ait::S_AIT_TABLE * Ait::Get()
{
    return ait_completed_.get();
}

/**
 * Clear any partial or completed data. This should be called when the service is changed or the
 * AIT PID is changed.
 */
void Ait::Clear()
{
    ait_ = nullptr;
    ait_completed_ = nullptr;
}

/**
 * Process the input AIT section and update the AIT returned by Get(). Prior values from Get()
 * may be invalidated by calling this method, consumers of this API should ensure serialization.
 * @param ait_service_id Service ID the AIT refers to
 * @param data AIT section data
 * @param nbytes size of AIT section data in bytes
 * @return true if the Get() value was changed (i.e. a table was completed or the service changed)
 */
bool Ait::ProcessSection(const uint8_t *data, uint32_t nbytes)
{
    bool updated = false;
    uint32_t ait_size;

    if (nbytes > 2)
    {
        ait_size = ((static_cast<uint32_t>(data[1]) << 8u | data[2]) & 0xFFFu) + 3;
        if (nbytes != ait_size)
        {
            LOG(LOG_ERROR, "Ait::ProcessSection Data size mismatch %d/%d.", nbytes, ait_size);

            return updated;
        }
    }
    else
    {
        LOG(LOG_ERROR, "Ait::ProcessSection Data size too small.");

        return updated;
    }

    if (ParseSection(data))
    {
        if (ait_ != nullptr && ait_->complete)
        {
            ait_completed_ = std::make_shared<S_AIT_TABLE>(*ait_);
            updated = true;
        }
    }

    return updated;
}

/**
 *
 * @param ait_table AIT table.
 * @param parental_control_age PC age set in the device.
 * @param parental_control_region 2 letter ISO 3166 region code.
 * @param parental_control_region3 3 letter ISO 3166 region code.
 * @return App to auto start
 */
const Ait::S_AIT_APP_DESC * Ait::AutoStartApp(const S_AIT_TABLE *ait_table, int
    parental_control_age,
    std::string &parental_control_region, std::string &parental_control_region3)
{
    int index;
    const S_AIT_APP_DESC *app;

    app = nullptr;
    if (ait_table)
    {
        for (index = 0; index != ait_table->num_apps; index++)
        {
            const S_AIT_APP_DESC *candidate = &ait_table->app_array[index];
            if (candidate->control_code == APP_CTL_AUTOSTART)
            {
                // Only run supported HbbTV versions
                bool supported = false;
                for (S_APP_PROFILE ad : candidate->app_desc.app_profiles)
                {
                    if ((ad.version_major < HBBTV_VERSION_MAJOR) ||
                        ((ad.version_major == HBBTV_VERSION_MAJOR) && (ad.version_minor <
                                                                       HBBTV_VERSION_MINOR)) ||
                        ((ad.version_major == HBBTV_VERSION_MAJOR) && (ad.version_minor ==
                                                                       HBBTV_VERSION_MINOR) &&
                         (ad.version_micro <= HBBTV_VERSION_MICRO)))
                    {
                        supported = true;
                        break;
                    }
                    else
                    {
                        LOG(LOG_ERROR, "Ait::AutoStartApp %d.%d.%d Version not supported.",
                            ad.version_major, ad.version_minor, ad.version_micro);
                    }
                }
                if (!supported)
                {
                    continue;
                }

                // Check parental restrictions
                if (IsAgeRestricted(candidate->parental_ratings, parental_control_age,
                    parental_control_region, parental_control_region3))
                {
                    LOG(LOG_DEBUG,
                        "Parental Control Age RESTRICTED for %s: only %d content accepted",
                        parental_control_region.c_str(), parental_control_age);
                    continue;
                }

                // Check we have a viable transport
                bool has_viable_transport = false;
                for (int j = 0; j < candidate->num_transports; j++)
                {
                    if (candidate->transport_array[j].protocol_id == AIT_PROTOCOL_HTTP ||
                        candidate->transport_array[j].protocol_id == AIT_PROTOCOL_OBJECT_CAROUSEL)
                    {
                        if (!candidate->transport_array[j].failed_to_load)
                        {
                            has_viable_transport = true;
                            break;
                        }
                    }
                }

                if (has_viable_transport)
                {
                    if (app == nullptr || app->app_desc.priority < candidate->app_desc.priority)
                    {
                        app = candidate;
                    }
                }
            }
        }
    }

    return app;
}

/**
 *
 * @param ait_table
 * @return
 */
const Ait::S_AIT_APP_DESC * Ait::TeletextApp(const S_AIT_TABLE *ait_table)
{
    int index;
    const S_AIT_APP_DESC *app;

    app = nullptr;
    if (ait_table)
    {
        for (index = 0; index != ait_table->num_apps; index++)
        {
            if (ait_table->app_array[index].usage_type == AIT_USAGE_TELETEXT)
            {
                app = &ait_table->app_array[index];
                break;
            }
        }
    }

    return app;
}

/**
 *
 * @param ait_table
 * @param org_id
 * @param app_id
 * @return
 */
Ait::S_AIT_APP_DESC * Ait::FindApp(S_AIT_TABLE *ait_table, uint32_t org_id, uint16_t app_id)
{
    int index;
    S_AIT_APP_DESC *app;

    app = nullptr;
    if (ait_table)
    {
        for (index = 0; index != ait_table->num_apps; index++)
        {
            app = &ait_table->app_array[index];
            if (app->org_id == org_id && app->app_id == app_id)
            {
                break;
            }
        }
        if (index == ait_table->num_apps)
        {
            app = nullptr;
        }
    }

    return app;
}

/**
 *
 * @param parsed_ait
 * @return
 */
bool Ait::PrintInfo(const S_AIT_TABLE *parsed_ait)
{
    S_AIT_APP_DESC hAitApp;
    int iExtUrl;
    const S_AIT_TABLE *sTable;

    sTable = parsed_ait;
    LOG(LOG_INFO, "Available apps: %d", sTable->num_apps);
    for (int i = 0; i < sTable->num_apps; i++)
    {
        LOG(LOG_INFO, "App(%d):", i);
        hAitApp = sTable->app_array[i];
        LOG(LOG_INFO, "\tApplication ID: %d", hAitApp.app_id);
        LOG(LOG_INFO, "\tOrganization ID: %d", hAitApp.org_id);
        LOG(LOG_INFO, "\tNumber of transports: %d", hAitApp.num_transports);
        for (int j = 0; j < hAitApp.num_transports; j++)
        {
            LOG(LOG_INFO, "\t\tTransport ID: %d ", (uint8_t)hAitApp.transport_array[j].protocol_id);
            switch (hAitApp.transport_array[j].protocol_id)
            {
                case 3:
                {
                    LOG(LOG_INFO, "\t\t\tBase URL: %s",
                        hAitApp.transport_array[j].url.base_url.c_str());
                    iExtUrl = hAitApp.transport_array[j].url.extension_urls.size();
                    if (iExtUrl > 1)
                    {
                        for (int k = 1; k < iExtUrl; k++)
                        {
                            LOG(LOG_INFO, "\t\t\tExtension url(%d): %s", k,
                                hAitApp.transport_array[j].url.extension_urls[k].c_str());
                        }
                    }
                    break;
                }
                case 1:
                {
                    LOG(LOG_INFO, "\t\t\tRemote connection: %d",
                        hAitApp.transport_array[j].oc.remote_connection);
                    LOG(LOG_INFO, "\t\t\tNet ID: %u",
                        hAitApp.transport_array[j].oc.dvb.original_network_id);
                    LOG(LOG_INFO, "\t\t\tStream ID: %u",
                        hAitApp.transport_array[j].oc.dvb.transport_stream_id);
                    LOG(LOG_INFO, "\t\t\tService ID: %u",
                        hAitApp.transport_array[j].oc.dvb.service_id);
                    LOG(LOG_INFO, "\t\t\tComponent tag: %d",
                        hAitApp.transport_array[j].oc.component_tag);
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        LOG(LOG_INFO, "\t\tLocation: %s", hAitApp.location.c_str());
        for (int j = 0; j < hAitApp.app_name.num_langs; j++)
        {
            LOG(LOG_INFO, "\t\tName(%d): %s (lang code: %c%c%c)", j,
                hAitApp.app_name.names[j].name.c_str(),
                (hAitApp.app_name.names[j].lang_code >> 16u) & 0xFFu,
                (hAitApp.app_name.names[j].lang_code >> 8u) & 0xFFu,
                hAitApp.app_name.names[j].lang_code & 0xFFu);
        }
        LOG(LOG_INFO, "\t\tXML type: %d", hAitApp.xml_type);
        LOG(LOG_INFO, "\t\tXML version: %d", hAitApp.xml_version);
        LOG(LOG_INFO, "\t\tUsage type: %d", hAitApp.usage_type);
        LOG(LOG_INFO, "\t\tVisibility: %d", hAitApp.app_desc.visibility);
        LOG(LOG_INFO, "\t\tPriority: %d", hAitApp.app_desc.priority);
        LOG(LOG_INFO, "\t\tService bound: %d", hAitApp.app_desc.service_bound);
        for (int j = 0; j < hAitApp.app_desc.app_profiles.size(); j++)
        {
            LOG(LOG_INFO, "\t\tProfile(%d): %d, version %d.%d.%d", j,
                hAitApp.app_desc.app_profiles[j].app_profile,
                hAitApp.app_desc.app_profiles[j].version_major,
                hAitApp.app_desc.app_profiles[j].version_minor,
                hAitApp.app_desc.app_profiles[j].version_micro);
        }
        for (int j = 0, num_boundaries = hAitApp.boundaries.size(); j < num_boundaries; j++)
        {
            LOG(LOG_INFO, "\t\tBoundary(%d): %s", j, hAitApp.boundaries[j].c_str());
        }
        LOG(LOG_INFO, "\t\tControl code: %d", hAitApp.control_code);
        for (int j = 0, num_prs = hAitApp.parental_ratings.size(); j < num_prs; j++)
        {
            LOG(LOG_INFO, "\t\tParentalRating(%d): %d Scheme: %s Region: %s", j,
                hAitApp.parental_ratings[j].value,
                hAitApp.parental_ratings[j].scheme.c_str(),
                hAitApp.parental_ratings[j].region.c_str());
        }
    }
    return true;
}

/**
 *
 * @param app_description
 * @return
 */
std::string Ait::GetBaseURL(const Ait::S_AIT_APP_DESC *app_description,
    const Utils::S_DVB_TRIPLET current_service, const bool is_network_available,
    uint16_t *protocol_id_selected)
{
    std::string result;
    int ti;
    char tmp_url[256]; // TODO(C++-ize)

    if (protocol_id_selected != nullptr)
    {
        *protocol_id_selected = 0;
    }

    for (ti = 0; ti != app_description->num_transports; ti++)
    {
        if (app_description->transport_array[ti].protocol_id == AIT_PROTOCOL_HTTP &&
            !app_description->transport_array[ti].failed_to_load &&
            is_network_available)
        {
            if (protocol_id_selected != nullptr)
            {
                *protocol_id_selected = AIT_PROTOCOL_HTTP;
            }
            result = app_description->transport_array[ti].url.base_url;
            break;
        }
        else if (app_description->transport_array[ti].protocol_id == AIT_PROTOCOL_OBJECT_CAROUSEL &&
                 !app_description->transport_array[ti].failed_to_load)
        {
            if (protocol_id_selected != nullptr)
            {
                *protocol_id_selected = AIT_PROTOCOL_OBJECT_CAROUSEL;
            }
            strcpy(tmp_url, "dvb://");
            if (app_description->transport_array[ti].oc.remote_connection)
            {
                sprintf(tmp_url + 6, "%x.%x.%x.%x",
                    app_description->transport_array[ti].oc.dvb.original_network_id,
                    app_description->transport_array[ti].oc.dvb.transport_stream_id,
                    app_description->transport_array[ti].oc.dvb.service_id,
                    app_description->transport_array[ti].oc.component_tag);
                strcat(tmp_url, "/");
            }
            else
            {
                sprintf(tmp_url + 6, "%x.%x.%x.%x",
                    current_service.original_network_id,
                    current_service.transport_stream_id,
                    current_service.service_id,
                    app_description->transport_array[ti].oc.component_tag);
                strcat(tmp_url, "/");
            }

            result = tmp_url;
            break;
        }
        else
        {
            // TODO In this scenario there are no valid transports but we don't handle it!
        }
    }

    return result;
}

/**
 * Determine whether the application has a transport with a certain protocol.
 * @param app_description The application description.
 * @param protocol_id The protocol to check for.
 * @return True if the application has a transport with the protocol, false otherwise.
 */
bool Ait::AppHasTransport(const Ait::S_AIT_APP_DESC *app_description, uint16_t protocol_id)
{
    for (int i = 0; i < app_description->num_transports; i++)
    {
        if (app_description->transport_array[i].protocol_id == protocol_id)
        {
            return true;
        }
    }
    return false;
}

/**
 * Set that the protocol for this app failed to load.
 * @param app_description The application description.
 * @param protocol_id The protocol that failed to load.
 */
void Ait::AppSetTransportFailedToLoad(Ait::S_AIT_APP_DESC *app_description, uint16_t protocol_id)
{
    for (int i = 0; i < app_description->num_transports; i++)
    {
        if (app_description->transport_array[i].protocol_id == protocol_id)
        {
            app_description->transport_array[i].failed_to_load = true;
        }
    }
}

/**
 *
 * @param data_ptr
 * @param desc
 */
void Ait::ParseAppDesc(const uint8_t *data_ptr, S_APP_DESC *desc)
{
    uint8_t desc_len;
    uint8_t profile_len;
    S_APP_PROFILE app_profile;

    if (desc->visibility == 2)
    {
        /* Not yet parsed for this application (visibility is invalid) */
        desc_len = *data_ptr;
        data_ptr++;

        profile_len = *data_ptr;
        data_ptr++;
        desc_len--;

        while (profile_len >= 5)
        {
            app_profile.app_profile = (*data_ptr << 8u) + *(data_ptr + 1);
            data_ptr += 2;

            app_profile.version_major = *data_ptr;
            app_profile.version_minor = *(data_ptr + 1);
            app_profile.version_micro = *(data_ptr + 2);
            data_ptr += 3;

            profile_len -= 5;
            desc_len -= 5;
            desc->app_profiles.push_back(app_profile);
        }

        desc->service_bound = ((*data_ptr & 0x80u) != 0);
        desc->visibility = (*data_ptr & 0x60u) >> 5u;
        data_ptr++;
        desc_len--;

        desc->priority = *data_ptr;
        data_ptr++;
        desc_len--;

        desc->num_labels = desc_len;
        desc->transport_protocol_labels.resize(desc->num_labels);
        for (int i = 0; i < desc->num_labels; i++)
        {
            desc->transport_protocol_labels[i] = data_ptr[i];
        }
#ifdef ANDROID_DEBUG
        {
            uint8_t num;
            LOG(LOG_DEBUG, "\tapp desc: bound=%u, visibility=%u, priority=%u", desc->service_bound,
                desc->visibility, desc->priority);
            for (num = 0; num < desc->app_profiles.size(); num++)
            {
                LOG(LOG_DEBUG,
                    "\tprofile %u: profile=0x%04x, major=%u, minor=%u, micro=%u", num,
                    desc->app_profiles[num].app_profile, desc->app_profiles[num].version_major,
                    desc->app_profiles[num].version_minor, desc->app_profiles[num].version_micro);
            }
            for (num = 0; num < desc->num_labels; num++)
            {
                LOG(LOG_DEBUG, "\tlabel %u: 0x%02x", num, desc->transport_protocol_labels[num]);
            }
        }
#endif
    }
    else
    {
        /* Already parsed for this application, skip */
        LOG(LOG_DEBUG, "Ait::ParseAppDesc Already parsed for this app, skipping");
    }
}

/**
 *
 * @param data_ptr
 * @param app_name
 */
void Ait::ParseAppNameDesc(const uint8_t *data_ptr, S_APP_NAME_DESC *app_name)
{
    uint32_t lang_code;
    uint16_t length;
    uint8_t desc_len;
    uint8_t name_len;
    const uint8_t *name_ptr;
    uint8_t num_langs;

    if (app_name->names.empty())
    {
        num_langs = 0;
        desc_len = *data_ptr;
        name_ptr = data_ptr + 1;
        length = 0;
        while (desc_len > 4)
        {
            name_len = *(name_ptr + 3);
            desc_len -= (name_len + 4);
            name_ptr += 4;
            if (name_len > 0)
            {
                num_langs++;
                length += name_len + 1;
            }
        }
        app_name->names.resize(num_langs);
        {
            app_name->num_langs = num_langs;
            num_langs = 0;
            desc_len = *data_ptr;
            data_ptr++;
            while (desc_len > 4)
            {
                lang_code = (*data_ptr << 16u) + (*(data_ptr + 1) << 8u) + *(data_ptr + 2);
                name_len = *(data_ptr + 3);
                desc_len -= (name_len + 4);
                data_ptr += 4;

                if (name_len > 0)
                {
                    app_name->names[num_langs].lang_code = lang_code;
                    app_name->names[num_langs].name = std::string(reinterpret_cast<const
                                                                                   char *>(data_ptr),
                        name_len);
                    // fprintf(stderr,"      app name: lang=%c%c%c, name=\"%s\" \n",
                    // (lang_code >> 16) & 0xff,(lang_code >> 8) & 0xff, (lang_code &
                    // 0xff), app_name->names[num_langs].zptr);
                    num_langs++;
                }
            }
        }
    }
    else
    {
        LOG(LOG_DEBUG, "Ait::ParseAppNameDesc Already parsed for this app, skipping");
    }
}

/**
 *
 * @param data_ptr
 * @param trns
 * @return true if the descriptor is a new valid one.
 */
bool Ait::ParseTransportProtocolDesc(const uint8_t *data_ptr, S_TRANSPORT_PROTOCOL_DESC *trns)
{
    uint8_t desc_len;
    uint8_t url_len, i, free_index;
    uint16_t protocol_id;
    bool new_desc;
    S_TRANSPORT_PROTOCOL_DESC *trns_ptr;

    desc_len = *data_ptr;
    data_ptr++;
    new_desc = false;

    if (desc_len >= 3)
    {
        protocol_id = (*data_ptr << 8u) + *(data_ptr + 1);
        free_index = AIT_MAX_NUM_PROTOCOLS;

        for (i = 0; i < AIT_MAX_NUM_PROTOCOLS; i++)
        {
            if (protocol_id == trns[i].protocol_id)
            {
                break;
            }
            if ((trns[i].protocol_id == 0) && (free_index == AIT_MAX_NUM_PROTOCOLS))
            {
                /* Save the first free index to be used for the new protocol */
                free_index = i;
            }
        }
        if (free_index == AIT_MAX_NUM_PROTOCOLS)
        {
            LOG(LOG_ERROR, "No free slots for this protocol: %d", protocol_id);
            i = AIT_MAX_NUM_PROTOCOLS;
        }

        if (i >= AIT_MAX_NUM_PROTOCOLS)
        {
            new_desc = true;
            trns_ptr = &(trns[free_index]);

            /* Protocol not present yet in this application */
            trns_ptr->protocol_id = (*data_ptr << 8u) + *(data_ptr + 1);
            data_ptr += 2;
            desc_len -= 2;

            trns_ptr->transport_protocol_label = *data_ptr;
            data_ptr++;
            desc_len--;

            LOG(LOG_DEBUG, "\ttransport: protocol_id=0x%04x, label=0x%02x",
                trns_ptr->protocol_id, trns_ptr->transport_protocol_label);

            /* Any remaining data are selector bytes */
            if (desc_len > 0)
            {
                switch (trns_ptr->protocol_id)
                {
                    case AIT_PROTOCOL_OBJECT_CAROUSEL:
                        /* The selector bytes represent an object carousel description */
                        trns_ptr->oc.remote_connection = ((*data_ptr & 0x80u) != 0);
                        data_ptr++;
                        if (trns_ptr->oc.remote_connection)
                        {
                            trns_ptr->oc.dvb.original_network_id = (*data_ptr << 8u) + *(data_ptr +
                                                                                         1);
                            data_ptr += 2;
                            trns_ptr->oc.dvb.transport_stream_id = (*data_ptr << 8u) + *(data_ptr +
                                                                                         1);
                            data_ptr += 2;
                            trns_ptr->oc.dvb.service_id = (*data_ptr << 8u) + *(data_ptr + 1);
                            data_ptr += 2;
                        }
                        else
                        {
                            trns_ptr->oc.dvb.original_network_id = 0;
                            trns_ptr->oc.dvb.transport_stream_id = 0;
                            trns_ptr->oc.dvb.service_id = 0;
                        }

                        trns_ptr->oc.component_tag = *data_ptr;
                        //data_ptr++;
                        // fprintf(stderr,"\n\tOC: remote=%u, net_id=0x%04x, stream_id=0x%04x,
                        // service_id=0x%04x,
                        // component_tag=0x%02x",trns_ptr->oc.remote_connection,
                        // trns_ptr->oc.dvb.original_network_id,
                        // trns_ptr->oc.dvb.transport_stream_id,trns_ptr->oc.dvb.service_id,
                        // trns_ptr->oc.component_tag);
                        break;

                    case AIT_PROTOCOL_HTTP:
                        /* The selector bytes represent a url description */
                        url_len = *data_ptr;
                        data_ptr++;
                        trns_ptr->url.base_url = std::string(reinterpret_cast<const
                                                                              char *>(data_ptr),
                            url_len);
                        data_ptr += url_len;
                        // fprintf(stderr,"\n\tURL: base_url=\"%s\"",
                        // trns_ptr->url.base_url.zptr);
                        i = *data_ptr;
                        data_ptr++;
                        while (i--)
                        {
                            url_len = *data_ptr;
                            data_ptr++;
                            trns_ptr->url.extension_urls.emplace_back(std::string(
                                reinterpret_cast<const char *>(data_ptr), url_len));
                            data_ptr += url_len;
                        }
                        break;

                    default:
                        break;
                }
            }

            trns_ptr->failed_to_load = false;
        }
        else
        {
            // fprintf(stderr,"\n\tProtocol %d already parsed for this application,
            // skipping", protocol_id);
        }
    }

    return new_desc;
}

/**
 *
 * @param data_ptr
 * @param str
 */
void Ait::ParseSimpleAppLocationDesc(const uint8_t *data_ptr, std::string &str)
{
    uint8_t desc_len;

    if (str.empty())
    {
        desc_len = *data_ptr;
        if (desc_len > 0)
        {
            data_ptr++;
            str = std::string(reinterpret_cast<const char *>(data_ptr), desc_len);
            LOG(LOG_DEBUG, "\tapp location: \"%s\"", str.c_str());
        }
    }
    else
    {
        LOG(LOG_DEBUG, "Ait::ParseSimpleAppLocationDesc Already parsed for this app, skipping");
    }
}

/**
 * Parses the Simple Application Boundary Descriptor and updates the boundary list.
 * @param data_ptr
 * @param boundaries
 * @param num_boundaries
 */
void Ait::ParseSimpleAppBoundaryDesc(const uint8_t *data_ptr, S_AIT_APP_DESC *app_ptr)
{
    uint8_t count, i, ext_length;
    const uint8_t *ptr;

    ptr = data_ptr + 1;
    count = *ptr;
    ptr++;
    for (i = 0; i < count; i++)
    {
        ext_length = *ptr;
        ptr++;
        std::string boundary(reinterpret_cast<const char *>(ptr), ext_length);
        app_ptr->boundaries.push_back(boundary);
        ptr += ext_length;
    }
}

/**
 * Parses the Parental Rating Descriptors.
 * @param data_ptr
 * @param app_ptr
 */
void Ait::ParseParentalRatingDesc(const uint8_t *data_ptr, S_AIT_APP_DESC *app_ptr)
{
    uint8_t desc_len;
    Ait::S_APP_PARENTAL_RATING pr;

    if (app_ptr->parental_ratings.empty())
    {
        desc_len = *data_ptr;
        data_ptr++;
        do
        {
            pr.scheme = "dvb-si";
            pr.region = std::string(reinterpret_cast<const char *>(data_ptr), 3);
            pr.value = *(data_ptr + 3) + 3;
            data_ptr += 4;
            app_ptr->parental_ratings.push_back(pr);
            desc_len -= 4;
        }
        while (desc_len > 4);
    }
    else
    {
        LOG(LOG_DEBUG, "Ait::ParseParentalRatingDesc Already parsed for this app, skipping");
    }
}

/**
 *
 * @param data
 * @param len
 * @param app_ptr
 */
void Ait::ParseApplication(const uint8_t *data, uint16_t len, S_AIT_APP_DESC *app_ptr)
{
    uint8_t tmp;
    uint16_t done;

    for (done = 0; done < len;)
    {
        tmp = *data;
        data++;
        switch (tmp)
        {
            case DTAG_APP_DESC:
                ParseAppDesc(data, &app_ptr->app_desc);
                break;

            case DTAG_APP_NAME:
                ParseAppNameDesc(data, &app_ptr->app_name);
                break;

            case DTAG_TRANSPORT_PROTOCOL:
                if (ParseTransportProtocolDesc(data, app_ptr->transport_array))
                {
                    app_ptr->num_transports++;
                }
                break;

            case DTAG_SIMPLE_APP_LOCATION:
                ParseSimpleAppLocationDesc(data, app_ptr->location);
                break;

            case DTAG_APP_USAGE:
                if ((*data == 1) && (app_ptr->usage_type != 1))
                {
                    app_ptr->usage_type = *(data + 1);
                    // fprintf(stderr,"\n\tApp usage: type=0x%02x", app_ptr->usage_type);
                }
                else
                {
                    // fprintf(stderr,"\n\tSkipping usage descriptor (this app has
                    // 0x%02x)",app_ptr->usage_type);
                }
                break;

            case DTAG_SIMPLE_APP_BOUNDARY: {
                ParseSimpleAppBoundaryDesc(data, app_ptr);
                break;
            }

            case DTAG_PARENTAL_RATING: {
                ParseParentalRatingDesc(data, app_ptr);
                break;
            }

            default:
                // fprintf(stderr,"\n\tunsupported desc: 0x%02x, len=%u", tmp, *data);
                break;
        }

        tmp = *data++;
        done += tmp + 2;
        data += tmp;
    }
}

/**
 * Returns true if the specified section has already been received
 * @param ait AIT structure
 * @param section_number Section number
 * @return true if the specified section has already been received
 */
bool Ait::SectionReceived(const S_AIT_TABLE *ait, uint8_t section_number)
{
    bool result;
    uint8_t *section_mask;
    uint8_t mask, index;

    section_mask = (uint8_t *)ait->section_data;
    index = GET_SECTION_MASK_INDEX(section_number);
    mask = 1 << GET_SECTION_MASK_SHIFT(section_number);
    result = ((section_mask[index] & mask) != 0);

    return result;
}

/**
 * Marks the bit representing the specified section number and returns true if all the sections
 * have been received
 * @param ait AIT structure
 * @param section_number Section number
 * @param last_section_number Last section number
 * @return true if all the sections have been received, false otherwise
 */
bool Ait::MarkSectionReceived(S_AIT_TABLE *ait, uint8_t section_number, uint8_t last_section_number)
{
    bool complete;
    uint8_t *section_mask;
    uint8_t mask, index, i;

    section_mask = (uint8_t *)ait->section_data;
    index = GET_SECTION_MASK_INDEX(section_number);
    mask = 1 << GET_SECTION_MASK_SHIFT(section_number);
    section_mask[index] |= mask;

    if ((section_number == 0) && (last_section_number == 0))
    {
        /* shortcut for the most frequent and simple case */
        complete = true;
    }
    else
    {
        index = GET_SECTION_MASK_INDEX(last_section_number);
        complete = true;

        /* Check the mask in the full slots */
        for (i = 0; i < index; i++)
        {
            if (section_mask[i] != 0xFF)
            {
                complete = false;
                break;
            }
        }
        /* Now check the mask in the last slot, which can be incomplete */
        if (complete)
        {
            mask = 1u << (GET_SECTION_MASK_SHIFT(last_section_number) + 1);
            mask--;
            if (section_mask[index] != mask)
            {
                complete = false;
            }
        }
    }

    return complete;
}

/**
 * Parses a section of the AIT table and updates the table structure
 * @param data_ptr Pointer to the first section byte
 * @return true if the table structure has changed
 */
bool Ait::ParseSection(const uint8_t *data_ptr)
{
    bool changed = false;
    std::shared_ptr<S_AIT_TABLE> ait;
    const uint8_t *app_data, *loop_end;
    uint16_t app_type;
    uint16_t desc_len, app_desc_len;
    uint8_t num_apps, version, num_new_apps;
    uint8_t i;
    uint8_t section_number, last_section_number;
    S_AIT_APP_DESC *app_ptr;
    uint32_t org_id;
    uint16_t app_id;

    /* Skip the table ID and section len */
    data_ptr += 3;

    app_type = (*data_ptr << 8u) + *(data_ptr + 1);
    data_ptr += 2;

    version = (*data_ptr & 0x3Eu) >> 1u;

    data_ptr++;
    section_number = *data_ptr;
    data_ptr++;
    last_section_number = *data_ptr;

    if (app_type != 0x0010)
    {
        LOG(LOG_DEBUG,
            "Ait::ParseSection AIT sub-table with unsupported application_type %x IGNORED",
            app_type);
    }
    else if (ait_ == nullptr || !SectionReceived(ait_.get(), section_number) || ait_->version !=
             version)
    {
        changed = true;

        if ((ait_ != nullptr) && (ait_->version != version))
        {
            /* This version is different, discard the old table and create a new one */
            ait_ = nullptr;
        }

        /* Skip to the common descriptors */
        data_ptr += 1;
        desc_len = ((*data_ptr & 0x0Fu) << 8u) + *(data_ptr + 1);
        data_ptr += 2;
        /* Skip the common descriptors */
        data_ptr += desc_len;

        desc_len = ((*data_ptr & 0x0Fu) << 8u) + *(data_ptr + 1);
        data_ptr += 2;

        num_new_apps = 0;
        loop_end = data_ptr + desc_len;
        app_data = data_ptr;
        for (num_apps = 0; app_data < loop_end; num_apps++)
        {
            app_data += 4;
            app_id = (*app_data << 8u);
            app_data++;
            app_id += *app_data;
            app_data += 2;
            app_desc_len = ((*app_data & 0x0Fu) << 8u) + *(app_data + 1);
            app_data += 2 + app_desc_len;
            if (ait_ != nullptr)
            {
                for (i = 0; i < ait_->num_apps; i++)
                {
                    if (ait_->app_array[i].app_id == app_id)
                    {
                        break;
                    }
                }
                if (i == ait_->num_apps)
                {
                    /* app_id not present, count it */
                    num_new_apps++;
                }
            }
        }
        if (ait_ == nullptr)
        {
            num_new_apps = num_apps;
        }

        LOG(LOG_DEBUG, "app_type=%x, version=%u num_apps=%u, section=%d/%d", app_type, version,
            num_apps, section_number, last_section_number);

        if (ait_ != nullptr)
        {
            ait = std::make_shared<S_AIT_TABLE>(*ait_);
        }
        else
        {
            ait = std::make_shared<S_AIT_TABLE>();
            ait->app_type = app_type;
            ait->version = version;
            ait->num_apps = 0;
        }

        ait->complete = MarkSectionReceived(ait.get(), section_number, last_section_number);
        if (num_new_apps > 0)
        {
            LOG(LOG_DEBUG, "Ait::ParseSection %d new apps in this section", num_new_apps);
            {
                ait->app_array.resize(ait->num_apps + num_new_apps);
                for (num_apps = 0, app_data = data_ptr; app_data < loop_end; num_apps++)
                {
                    org_id = (*app_data << 24u) + (*(app_data + 1) << 16u) +
                        (*(app_data + 2) << 8u) + *(app_data + 3);
                    app_data += 4;
                    app_id = (*app_data << 8u) + *(app_data + 1);
                    app_data += 2;

                    if (ait_ != nullptr)
                    {
                        for (i = 0; i < ait->num_apps; i++)
                        {
                            if (ait->app_array[i].app_id == app_id)
                            {
                                break;
                            }
                        }
                    }
                    else
                    {
                        /* if table wasn't provided (first section) just add the
                         * applications as they come */
                        i = num_apps;
                    }

                    app_ptr = &(ait->app_array[i]);

                    /* Check if the app in this loop is already present in the incoming
                     * table structure */
                    if ((ait_ == nullptr) || (i == ait->num_apps))
                    {
                        /* This is a new app */
                        app_ptr->org_id = org_id;
                        app_ptr->app_id = app_id;

                        app_ptr->control_code = *app_data;
                        app_data++;

                        app_desc_len = ((*app_data & 0x0Fu) << 8u) + *(app_data + 1);
                        app_data += 2;

                        /* Initialise the app_desc with an invalid visibility so that we
                           know what it has been parsed for this application */
                        app_ptr->app_desc.visibility = 2;

                        ait->num_apps++;

                        // f(stderr,"[%u] org_id=0x%08x, app_id=0x%04x,
                        // control_code=0x%02x, app_desc_len=%u \n\n",num_apps,
                        // app_ptr->org_id, app_ptr->app_id, app_ptr->control_code,
                        // app_desc_len);
                    }
                    else
                    {
                        /* The app is already present */
                        app_data += 7;
                        app_desc_len = ((*app_data & 0x0Fu) << 8u) + *(app_data + 1);
                        app_data += 2;
                    }

                    ParseApplication(app_data, app_desc_len, app_ptr);

                    app_data += app_desc_len;
                }
            }
        }
        else
        {
            LOG(LOG_DEBUG, "Ait::ParseSection Skip this section, no new apps (version=%u)",
                version);
        }

        ait_.swap(ait);

#ifdef ANDROID_DEBUG
        if (ait_ != nullptr)
        {
            PrintInfo(ait_.get());
        }
#endif
    }
    else
    {
        LOG(LOG_DEBUG,
            "Ait::ParseSection Section already received and existing ait_ is same version");
    }

    return changed;
}

/**
 * Checks whether an App has parental restrictions.
 * @param parental_ratings List of parental ratings included in the AIT.
 * @param parental_control_age PC age set in the device.
 * @param parental_control_region 2 letter ISO 3166 region code.
 * @param parental_control_region3 3 letter ISO 3166 region code.
 */
bool Ait::IsAgeRestricted(const std::vector<Ait::S_APP_PARENTAL_RATING> parental_ratings, int
    parental_control_age,
    std::string &parental_control_region, std::string &parental_control_region3)
{
    bool restricted;
    if (!parental_ratings.empty())
    {
        restricted = true;
        for (const Ait::S_APP_PARENTAL_RATING& pr : parental_ratings)
        {
            LOG(LOG_ERROR, "APP_PARENTAL_RATING %s %s/%s/%s %d/%d", pr.scheme.c_str(),
                pr.region.c_str(), parental_control_region.c_str(),
                parental_control_region3.c_str(),
                pr.value, parental_control_age);
            LOG(LOG_ERROR, "%d %d %d %d", (pr.scheme == "dvb-si"),
                ((pr.region.size() == 2) && (strcasecmp(pr.region.c_str(),
                    parental_control_region.c_str()) == 0)),
                ((pr.region.size() == 3) && (strcasecmp(pr.region.c_str(),
                    parental_control_region3.c_str()) == 0)),
                (pr.value <= parental_control_age));
            LOG(LOG_ERROR, "%d %d %d", pr.region.size(), strcasecmp(pr.region.c_str(),
                parental_control_region.c_str()),
                (strcasecmp(pr.region.c_str(), parental_control_region3.c_str()) == 0));
            if ((pr.scheme == "dvb-si") &&
                (((pr.region.size() == 2) && (strcasecmp(pr.region.c_str(),
                    parental_control_region.c_str()) == 0)) ||
                 ((pr.region.size() == 3) && (strcasecmp(pr.region.c_str(),
                     parental_control_region3.c_str()) == 0))) &&
                (pr.value <= parental_control_age))
            {
                restricted = false;
            }
        }
    }
    else
    {
        restricted = false;
    }
    return restricted;
}
