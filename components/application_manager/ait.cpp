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
#include <sstream>

#include "log.h"
#include "utils.h"
#include "application_manager.h"

#define DTAG_APP_DESC 0x00
#define DTAG_APP_NAME 0x01
#define DTAG_TRANSPORT_PROTOCOL 0x02
#define DTAG_EXT_AUTH 0x05
#define DTAG_APPLICATION_ICON 0x0b
#define DTAG_GRAPHICS_CONSTRAINTS 0x14
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
    return m_aitCompleted.get();
}

/**
 * Clear any partial or completed data. This should be called when the service is changed or the
 * AIT PID is changed.
 */
void Ait::Clear()
{
    m_ait = nullptr;
    m_aitCompleted = nullptr;
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
    uint32_t aitSize;

    if (nbytes > 2)
    {
        aitSize = ((static_cast<uint32_t>(data[1]) << 8u | data[2]) & 0xFFFu) + 3;
        if (nbytes != aitSize)
        {
            LOG(LOG_ERROR, "Ait::ProcessSection Data size mismatch %d/%d.", nbytes, aitSize);

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
        if (m_ait != nullptr && m_ait->complete)
        {
            m_aitCompleted = std::make_shared<S_AIT_TABLE>(*m_ait);
            updated = true;
        }
    }

    return updated;
}

void Ait::ApplyAitTable(std::unique_ptr<Ait::S_AIT_TABLE> &aitTable)
{
    m_ait = std::move(aitTable);
    m_aitCompleted = m_ait;
}

/**
 *
 * @param aitTable AIT table.
 * @param parentalControlAge PC age set in the device.
 * @param parentalControlRegion 2 letter ISO 3166 region code.
 * @param parentalControlRegion3 3 letter ISO 3166 region code.
 * @return App to auto start
 */
const Ait::S_AIT_APP_DESC * Ait::AutoStartApp(const S_AIT_TABLE *aitTable, int
    parentalControlAge,
    std::string &parentalControlRegion, std::string &parentalControlRegion3)
{
    int index;
    const S_AIT_APP_DESC *app;

    app = nullptr;
    if (aitTable)
    {
        for (index = 0; index != aitTable->numApps; index++)
        {
            const S_AIT_APP_DESC *candidate = &aitTable->appArray[index];
            if (candidate->controlCode == APP_CTL_AUTOSTART)
            {
                // Only run supported HbbTV versions
                bool supported = false;
                for (S_APP_PROFILE ad : candidate->appDesc.appProfiles)
                {
                    if ((ad.versionMajor < HBBTV_VERSION_MAJOR) ||
                        ((ad.versionMajor == HBBTV_VERSION_MAJOR) && (ad.versionMinor <
                                                                      HBBTV_VERSION_MINOR)) ||
                        ((ad.versionMajor == HBBTV_VERSION_MAJOR) && (ad.versionMinor ==
                                                                      HBBTV_VERSION_MINOR) &&
                         (ad.versionMicro <= HBBTV_VERSION_MICRO)))
                    {
                        // TODO(COIT-53) Add flags for PVR and DL options that are used in comparison with the application profile.
                        if (ad.appProfile == 0)
                        {
                            supported = true;
                            break;
                        }
                        else
                        {
                            LOG(LOG_ERROR, "Ait::AutoStartApp '%d' profile not supported.",
                                ad.appProfile);
                        }
                    }
                    else
                    {
                        LOG(LOG_ERROR, "Ait::AutoStartApp %d.%d.%d Version not supported.",
                            ad.versionMajor, ad.versionMinor, ad.versionMicro);
                    }
                }
                if (!supported)
                {
                    continue;
                }

                // Check parental restrictions
                if (IsAgeRestricted(candidate->parentalRatings, parentalControlAge,
                    parentalControlRegion, parentalControlRegion3))
                {
                    LOG(LOG_DEBUG,
                        "Parental Control Age RESTRICTED for %s: only %d content accepted",
                        parentalControlRegion.c_str(), parentalControlAge);
                    continue;
                }

                // Check we have a viable transport
                bool hasViableTransport = false;
                for (int j = 0; j < candidate->numTransports; j++)
                {
                    if (candidate->transportArray[j].protocolId == AIT_PROTOCOL_HTTP ||
                        candidate->transportArray[j].protocolId == AIT_PROTOCOL_OBJECT_CAROUSEL)
                    {
                        if (!candidate->transportArray[j].failedToLoad)
                        {
                            hasViableTransport = true;
                            break;
                        }
                    }
                }

                if (hasViableTransport)
                {
                    if (app == nullptr || app->appDesc.priority < candidate->appDesc.priority)
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
 * @param aitTable
 * @return
 */
const Ait::S_AIT_APP_DESC * Ait::TeletextApp(const S_AIT_TABLE *aitTable)
{
    int index;
    const S_AIT_APP_DESC *app;

    app = nullptr;
    if (aitTable)
    {
        for (index = 0; index != aitTable->numApps; index++)
        {
            if (aitTable->appArray[index].usageType == AIT_USAGE_TELETEXT)
            {
                app = &aitTable->appArray[index];
                break;
            }
        }
    }

    return app;
}

/**
 *
 * @param aitTable
 * @param orgId
 * @param appId
 * @return
 */
Ait::S_AIT_APP_DESC * Ait::FindApp(S_AIT_TABLE *aitTable, uint32_t orgId, uint16_t appId)
{
    int index;
    S_AIT_APP_DESC *app;

    app = nullptr;
    if (aitTable)
    {
        for (index = 0; index != aitTable->numApps; index++)
        {
            app = &aitTable->appArray[index];
            if (app->orgId == orgId && app->appId == appId)
            {
                break;
            }
        }
        if (index == aitTable->numApps)
        {
            app = nullptr;
        }
    }

    return app;
}

/**
 *
 * @param parsedAit
 * @return
 */
bool Ait::PrintInfo(const S_AIT_TABLE *parsedAit)
{
    S_AIT_APP_DESC hAitApp;
    int iExtUrl;
    const S_AIT_TABLE *sTable;

    sTable = parsedAit;
    LOG(LOG_INFO, "Available apps: %d", sTable->numApps);
    for (int i = 0; i < sTable->numApps; i++)
    {
        LOG(LOG_INFO, "HbbTVApp(%d):", i);
        hAitApp = sTable->appArray[i];
        LOG(LOG_INFO, "\tApplication ID: %d", hAitApp.appId);
        LOG(LOG_INFO, "\tOrganization ID: %d", hAitApp.orgId);
        LOG(LOG_INFO, "\tClassification scheme: %s", hAitApp.scheme.c_str());
        LOG(LOG_INFO, "\tNumber of transports: %d", hAitApp.numTransports);
        for (int j = 0; j < hAitApp.numTransports; j++)
        {
            LOG(LOG_INFO, "\t\tTransport ID: %d ", (uint8_t)hAitApp.transportArray[j].protocolId);
            switch (hAitApp.transportArray[j].protocolId)
            {
                case 3:
                {
                    LOG(LOG_INFO, "\t\t\tBase URL: %s",
                        hAitApp.transportArray[j].url.baseUrl.c_str());
                    iExtUrl = hAitApp.transportArray[j].url.extensionUrls.size();
                    if (iExtUrl > 1)
                    {
                        for (int k = 1; k < iExtUrl; k++)
                        {
                            LOG(LOG_INFO, "\t\t\tExtension url(%d): %s", k,
                                hAitApp.transportArray[j].url.extensionUrls[k].c_str());
                        }
                    }
                    break;
                }
                case 1:
                {
                    LOG(LOG_INFO, "\t\t\tRemote connection: %d",
                        hAitApp.transportArray[j].oc.remoteConnection);
                    LOG(LOG_INFO, "\t\t\tNet ID: %u",
                        hAitApp.transportArray[j].oc.dvb.originalNetworkId);
                    LOG(LOG_INFO, "\t\t\tStream ID: %u",
                        hAitApp.transportArray[j].oc.dvb.transportStreamId);
                    LOG(LOG_INFO, "\t\t\tService ID: %u",
                        hAitApp.transportArray[j].oc.dvb.serviceId);
                    LOG(LOG_INFO, "\t\t\tComponent tag: %d",
                        hAitApp.transportArray[j].oc.componentTag);
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        LOG(LOG_INFO, "\t\tLocation: %s", hAitApp.location.c_str());
        for (int j = 0; j < hAitApp.appName.numLangs; j++)
        {
            LOG(LOG_INFO, "\t\tName(%d): %s (lang code: %c%c%c)", j,
                hAitApp.appName.names[j].name.c_str(),
                (hAitApp.appName.names[j].langCode >> 16u) & 0xFFu,
                (hAitApp.appName.names[j].langCode >> 8u) & 0xFFu,
                hAitApp.appName.names[j].langCode & 0xFFu);
        }
        LOG(LOG_INFO, "\t\tXML type: %d", hAitApp.xmlType);
        LOG(LOG_INFO, "\t\tXML version: %d", hAitApp.xmlVersion);
        LOG(LOG_INFO, "\t\tUsage type: %d", hAitApp.usageType);
        LOG(LOG_INFO, "\t\tVisibility: %d", hAitApp.appDesc.visibility);
        LOG(LOG_INFO, "\t\tPriority: %d", hAitApp.appDesc.priority);
        LOG(LOG_INFO, "\t\tService bound: %d", hAitApp.appDesc.serviceBound);
        for (unsigned int j = 0; j < hAitApp.appDesc.appProfiles.size(); j++)
        {
            LOG(LOG_INFO, "\t\tProfile(%d): %d, version %d.%d.%d", j,
                hAitApp.appDesc.appProfiles[j].appProfile,
                hAitApp.appDesc.appProfiles[j].versionMajor,
                hAitApp.appDesc.appProfiles[j].versionMinor,
                hAitApp.appDesc.appProfiles[j].versionMicro);
        }
        for (int j = 0, num_boundaries = hAitApp.boundaries.size(); j < num_boundaries; j++)
        {
            LOG(LOG_INFO, "\t\tBoundary(%d): %s", j, hAitApp.boundaries[j].c_str());
        }
        LOG(LOG_INFO, "\t\tControl code: %d", hAitApp.controlCode);
        for (int j = 0, num_prs = hAitApp.parentalRatings.size(); j < num_prs; j++)
        {
            LOG(LOG_INFO, "\t\tParentalRating(%d): %d Scheme: %s Region: %s", j,
                hAitApp.parentalRatings[j].value,
                hAitApp.parentalRatings[j].scheme.c_str(),
                hAitApp.parentalRatings[j].region.c_str());
        }
        if (!hAitApp.graphicsConstraints.empty())
        {
            std::stringstream ss;
            for (int j = 0; j < hAitApp.graphicsConstraints.size(); ++j)
            {
                std::string sep = (j < hAitApp.graphicsConstraints.size() - 1) ? "p, " : "p";
                ss << std::to_string(hAitApp.graphicsConstraints[j]) << sep;
            }
            LOG(LOG_INFO, "\t\tGraphics constraints: %s", ss.str().c_str());
        }
    }
    return true;
}

/**
 *
 * @param appDescription
 * @return
 */
std::string Ait::ExtractBaseURL(const Ait::S_AIT_APP_DESC &appDescription,
    const Utils::S_DVB_TRIPLET currentService, const bool isNetworkAvailable)
{
    std::string result;
    char tmp_url[256]; // TODO(C++-ize)

    for (int ti = 0; ti != appDescription.numTransports; ti++)
    {
        if (appDescription.transportArray[ti].protocolId == AIT_PROTOCOL_HTTP &&
            !appDescription.transportArray[ti].failedToLoad &&
            isNetworkAvailable)
        {
            result = appDescription.transportArray[ti].url.baseUrl;
            break;
        }
        else if (appDescription.transportArray[ti].protocolId == AIT_PROTOCOL_OBJECT_CAROUSEL &&
                 !appDescription.transportArray[ti].failedToLoad)
        {
            strcpy(tmp_url, "dvb://");
            if (appDescription.transportArray[ti].oc.remoteConnection)
            {
                sprintf(tmp_url + 6, "%x.%x.%x.%x",
                    appDescription.transportArray[ti].oc.dvb.originalNetworkId,
                    appDescription.transportArray[ti].oc.dvb.transportStreamId,
                    appDescription.transportArray[ti].oc.dvb.serviceId,
                    appDescription.transportArray[ti].oc.componentTag);
                strcat(tmp_url, "/");
            }
            else
            {
                sprintf(tmp_url + 6, "%x.%x.%x.%x",
                    currentService.originalNetworkId,
                    currentService.transportStreamId,
                    currentService.serviceId,
                    appDescription.transportArray[ti].oc.componentTag);
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

uint16_t Ait::ExtractProtocolId(const Ait::S_AIT_APP_DESC &appDescription, const bool isNetworkAvailable)
{
    for (int ti = 0; ti != appDescription.numTransports; ti++)
    {
        if (appDescription.transportArray[ti].protocolId == AIT_PROTOCOL_HTTP &&
            !appDescription.transportArray[ti].failedToLoad &&
            isNetworkAvailable)
        {
            return AIT_PROTOCOL_HTTP;
        }
        else if (appDescription.transportArray[ti].protocolId == AIT_PROTOCOL_OBJECT_CAROUSEL &&
                 !appDescription.transportArray[ti].failedToLoad)
        {
            return AIT_PROTOCOL_OBJECT_CAROUSEL;
        }
    }

    return 0;
}

/**
 * Determine whether the application has a transport with a certain protocol.
 * @param appDescription The application description.
 * @param protocolId The protocol to check for.
 * @return True if the application has a transport with the protocol, false otherwise.
 */
bool Ait::AppHasTransport(const Ait::S_AIT_APP_DESC *appDescription, uint16_t protocolId)
{
    for (int i = 0; i < appDescription->numTransports; i++)
    {
        if (appDescription->transportArray[i].protocolId == protocolId)
        {
            return true;
        }
    }
    return false;
}

/**
 * Set that the protocol for this app failed to load.
 * @param appDescription The application description.
 * @param protocolId The protocol that failed to load.
 */
void Ait::AppSetTransportFailedToLoad(Ait::S_AIT_APP_DESC *appDescription, uint16_t protocolId)
{
    for (int i = 0; i < appDescription->numTransports; i++)
    {
        if (appDescription->transportArray[i].protocolId == protocolId)
        {
            appDescription->transportArray[i].failedToLoad = true;
        }
    }
}

/**
 *
 * @param dataPtr
 * @param desc
 */
void Ait::ParseAppDesc(const uint8_t *dataPtr, S_APP_DESC *desc)
{
    uint8_t descLen;
    uint8_t profileLen;
    S_APP_PROFILE app_profile;

    if (desc->visibility == 2)
    {
        /* Not yet parsed for this application (visibility is invalid) */
        descLen = *dataPtr;
        dataPtr++;

        profileLen = *dataPtr;
        dataPtr++;
        descLen--;

        while (profileLen >= 5)
        {
            app_profile.appProfile = (*dataPtr << 8u) + *(dataPtr + 1);
            dataPtr += 2;

            app_profile.versionMajor = *dataPtr;
            app_profile.versionMinor = *(dataPtr + 1);
            app_profile.versionMicro = *(dataPtr + 2);
            dataPtr += 3;

            profileLen -= 5;
            descLen -= 5;
            desc->appProfiles.push_back(app_profile);
        }

        desc->serviceBound = ((*dataPtr & 0x80u) != 0);
        desc->visibility = (*dataPtr & 0x60u) >> 5u;
        dataPtr++;
        descLen--;

        desc->priority = *dataPtr;
        dataPtr++;
        descLen--;

        desc->numLabels = descLen;
        desc->transportProtocolLabels.resize(desc->numLabels);
        for (int i = 0; i < desc->numLabels; i++)
        {
            desc->transportProtocolLabels[i] = dataPtr[i];
        }
#ifdef ANDROID_DEBUG
        {
            uint8_t num;
            LOG(LOG_DEBUG, "\tapp desc: bound=%u, visibility=%u, priority=%u", desc->serviceBound,
                desc->visibility, desc->priority);
            for (num = 0; num < desc->appProfiles.size(); num++)
            {
                LOG(LOG_DEBUG,
                    "\tprofile %u: profile=0x%04x, major=%u, minor=%u, micro=%u", num,
                    desc->appProfiles[num].appProfile, desc->appProfiles[num].versionMajor,
                    desc->appProfiles[num].versionMinor, desc->appProfiles[num].versionMicro);
            }
            for (num = 0; num < desc->numLabels; num++)
            {
                LOG(LOG_DEBUG, "\tlabel %u: 0x%02x", num, desc->transportProtocolLabels[num]);
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
 * @param dataPtr
 * @param appName
 */
void Ait::ParseAppNameDesc(const uint8_t *dataPtr, S_APP_NAME_DESC *appName)
{
    uint32_t langCode;
    uint16_t length;
    uint8_t descLen;
    uint8_t nameLen;
    const uint8_t *namePtr;
    uint8_t numLangs;

    if (appName->names.empty())
    {
        numLangs = 0;
        descLen = *dataPtr;
        namePtr = dataPtr + 1;
        length = 0;
        while (descLen > 4)
        {
            nameLen = *(namePtr + 3);
            descLen -= (nameLen + 4);
            namePtr += 4;
            if (nameLen > 0)
            {
                numLangs++;
                length += nameLen + 1;
            }
        }
        appName->names.resize(numLangs);
        {
            appName->numLangs = numLangs;
            numLangs = 0;
            descLen = *dataPtr;
            dataPtr++;
            while (descLen > 4)
            {
                langCode = (*dataPtr << 16u) + (*(dataPtr + 1) << 8u) + *(dataPtr + 2);
                nameLen = *(dataPtr + 3);
                descLen -= (nameLen + 4);
                dataPtr += 4;

                if (nameLen > 0)
                {
                    appName->names[numLangs].langCode = langCode;
                    appName->names[numLangs].name = std::string(reinterpret_cast<const
                                                                                 char *>(dataPtr),
                        nameLen);
                    // fprintf(stderr,"      app name: lang=%c%c%c, name=\"%s\" \n",
                    // (lang_code >> 16) & 0xff,(lang_code >> 8) & 0xff, (lang_code &
                    // 0xff), app_name->names[num_langs].zptr);
                    numLangs++;
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
 * @param dataPtr
 * @param trns
 * @return true if the descriptor is a new valid one.
 */
bool Ait::ParseTransportProtocolDesc(const uint8_t *dataPtr, S_TRANSPORT_PROTOCOL_DESC *trns)
{
    uint8_t descLen;
    uint8_t urlLen, i, freeIndex;
    uint16_t protocolId;
    bool new_desc;
    S_TRANSPORT_PROTOCOL_DESC *trnsPtr;

    descLen = *dataPtr;
    dataPtr++;
    new_desc = false;

    if (descLen >= 3)
    {
        protocolId = (*dataPtr << 8u) + *(dataPtr + 1);
        freeIndex = AIT_MAX_NUM_PROTOCOLS;

        for (i = 0; i < AIT_MAX_NUM_PROTOCOLS; i++)
        {
            if (protocolId == trns[i].protocolId)
            {
                break;
            }
            if ((trns[i].protocolId == 0) && (freeIndex == AIT_MAX_NUM_PROTOCOLS))
            {
                /* Save the first free index to be used for the new protocol */
                freeIndex = i;
            }
        }
        if (freeIndex == AIT_MAX_NUM_PROTOCOLS)
        {
            LOG(LOG_ERROR, "No free slots for this protocol: %d", protocolId);
            i = AIT_MAX_NUM_PROTOCOLS;
        }

        if (i >= AIT_MAX_NUM_PROTOCOLS)
        {
            new_desc = true;
            trnsPtr = &(trns[freeIndex]);

            /* Protocol not present yet in this application */
            trnsPtr->protocolId = (*dataPtr << 8u) + *(dataPtr + 1);
            dataPtr += 2;
            descLen -= 2;

            trnsPtr->transportProtocolLabel = *dataPtr;
            dataPtr++;
            descLen--;

            LOG(LOG_DEBUG, "\ttransport: protocol_id=0x%04x, label=0x%02x",
                trnsPtr->protocolId, trnsPtr->transportProtocolLabel);

            /* Any remaining data are selector bytes */
            if (descLen > 0)
            {
                switch (trnsPtr->protocolId)
                {
                    case AIT_PROTOCOL_OBJECT_CAROUSEL:
                        /* The selector bytes represent an object carousel description */
                        trnsPtr->oc.remoteConnection = ((*dataPtr & 0x80u) != 0);
                        dataPtr++;
                        if (trnsPtr->oc.remoteConnection)
                        {
                            trnsPtr->oc.dvb.originalNetworkId = (*dataPtr << 8u) + *(dataPtr +
                                                                                     1);
                            dataPtr += 2;
                            trnsPtr->oc.dvb.transportStreamId = (*dataPtr << 8u) + *(dataPtr +
                                                                                     1);
                            dataPtr += 2;
                            trnsPtr->oc.dvb.serviceId = (*dataPtr << 8u) + *(dataPtr + 1);
                            dataPtr += 2;
                        }
                        else
                        {
                            trnsPtr->oc.dvb.originalNetworkId = 0;
                            trnsPtr->oc.dvb.transportStreamId = 0;
                            trnsPtr->oc.dvb.serviceId = 0;
                        }

                        trnsPtr->oc.componentTag = *dataPtr;
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
                        urlLen = *dataPtr;
                        dataPtr++;
                        trnsPtr->url.baseUrl = std::string(reinterpret_cast<const
                                                                            char *>(dataPtr),
                            urlLen);
                        dataPtr += urlLen;
                        // fprintf(stderr,"\n\tURL: base_url=\"%s\"",
                        // trns_ptr->url.base_url.zptr);
                        i = *dataPtr;
                        dataPtr++;
                        while (i--)
                        {
                            urlLen = *dataPtr;
                            dataPtr++;
                            trnsPtr->url.extensionUrls.emplace_back(std::string(
                                reinterpret_cast<const char *>(dataPtr), urlLen));
                            dataPtr += urlLen;
                        }
                        break;

                    default:
                        break;
                }
            }

            trnsPtr->failedToLoad = false;
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
 * @param dataPtr
 * @param str
 */
void Ait::ParseSimpleAppLocationDesc(const uint8_t *dataPtr, std::string &str)
{
    uint8_t descLen;

    if (str.empty())
    {
        descLen = *dataPtr;
        if (descLen > 0)
        {
            dataPtr++;
            str = std::string(reinterpret_cast<const char *>(dataPtr), descLen);
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
 * @param dataPtr
 * @param boundaries
 * @param num_boundaries
 */
void Ait::ParseSimpleAppBoundaryDesc(const uint8_t *dataPtr, S_AIT_APP_DESC *appPtr)
{
    uint8_t count, i, extLength;
    const uint8_t *ptr;

    ptr = dataPtr + 1;
    count = *ptr;
    ptr++;
    for (i = 0; i < count; i++)
    {
        extLength = *ptr;
        ptr++;
        std::string boundary(reinterpret_cast<const char *>(ptr), extLength);
        appPtr->boundaries.push_back(boundary);
        ptr += extLength;
    }
}

/**
 * Parses the Parental Rating Descriptors.
 * @param dataPtr
 * @param appPtr
 */
void Ait::ParseParentalRatingDesc(const uint8_t *dataPtr, S_AIT_APP_DESC *appPtr)
{
    uint8_t descLen;
    Ait::S_APP_PARENTAL_RATING pr;

    if (appPtr->parentalRatings.empty())
    {
        descLen = *dataPtr;
        dataPtr++;
        do
        {
            pr.scheme = "dvb-si";
            pr.region = std::string(reinterpret_cast<const char *>(dataPtr), 3);
            pr.value = *(dataPtr + 3) + 3;
            dataPtr += 4;
            appPtr->parentalRatings.push_back(pr);
            descLen -= 4;
        }
        while (descLen > 4);
    }
    else
    {
        LOG(LOG_DEBUG, "Ait::ParseParentalRatingDesc Already parsed for this app, skipping");
    }
}

/**
 * Parses the Graphics Constraints Descriptors.
 * @param dataPtr
 * @param appPtr
 */
void Ait::ParseGraphicsConstraints(const uint8_t *dataPtr, S_AIT_APP_DESC *appPtr)
{
    if (appPtr->graphicsConstraints.empty())
    {
        appPtr->graphicsConstraints.push_back(720);
        int numGraphics = (*dataPtr) - 1;
        dataPtr++;
        for (int i = 0; i < numGraphics; ++i)
        {
            dataPtr++;
            int id = *dataPtr;
            switch (id)
            {
                case 4:
                    appPtr->graphicsConstraints.push_back(1080);
                    break;
                case 5:
                    appPtr->graphicsConstraints.push_back(2160);
                    break;
                case 6:
                    appPtr->graphicsConstraints.push_back(4320);
                    break;
            }
        }
    }
    else
    {
        LOG(LOG_DEBUG, "Ait::ParseGraphicsConstraints Already parsed for this app, skipping");
    }
}

/**
 *
 * @param data
 * @param len
 * @param appPtr
 */
void Ait::ParseApplication(const uint8_t *data, uint16_t len, S_AIT_APP_DESC *appPtr)
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
                ParseAppDesc(data, &appPtr->appDesc);
                break;

            case DTAG_APP_NAME:
                ParseAppNameDesc(data, &appPtr->appName);
                break;

            case DTAG_TRANSPORT_PROTOCOL:
                if (ParseTransportProtocolDesc(data, appPtr->transportArray))
                {
                    appPtr->numTransports++;
                }
                break;

            case DTAG_SIMPLE_APP_LOCATION:
                ParseSimpleAppLocationDesc(data, appPtr->location);
                break;

            case DTAG_APP_USAGE:
                if ((*data == 1) && (appPtr->usageType != 1))
                {
                    appPtr->usageType = *(data + 1);
                    // fprintf(stderr,"\n\tApp usage: type=0x%02x", app_ptr->usage_type);
                }
                else
                {
                    // fprintf(stderr,"\n\tSkipping usage descriptor (this app has
                    // 0x%02x)",app_ptr->usage_type);
                }
                break;

            case DTAG_SIMPLE_APP_BOUNDARY: {
                ParseSimpleAppBoundaryDesc(data, appPtr);
                break;
            }

            case DTAG_PARENTAL_RATING: {
                ParseParentalRatingDesc(data, appPtr);
                break;
            }

            case DTAG_GRAPHICS_CONSTRAINTS: {
                ParseGraphicsConstraints(data, appPtr);
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
 * @param sectionNumber Section number
 * @return true if the specified section has already been received
 */
bool Ait::SectionReceived(const S_AIT_TABLE *ait, uint8_t sectionNumber)
{
    bool result;
    uint8_t *sectionMask;
    uint8_t mask, index;

    sectionMask = (uint8_t *)ait->sectionData;
    index = GET_SECTION_MASK_INDEX(sectionNumber);
    mask = 1 << GET_SECTION_MASK_SHIFT(sectionNumber);
    result = ((sectionMask[index] & mask) != 0);

    return result;
}

/**
 * Marks the bit representing the specified section number and returns true if all the sections
 * have been received
 * @param ait AIT structure
 * @param sectionNumber Section number
 * @param lastSectionNumber Last section number
 * @return true if all the sections have been received, false otherwise
 */
bool Ait::MarkSectionReceived(S_AIT_TABLE *ait, uint8_t sectionNumber, uint8_t lastSectionNumber)
{
    bool complete;
    uint8_t *sectionMask;
    uint8_t mask, index, i;

    sectionMask = (uint8_t *)ait->sectionData;
    index = GET_SECTION_MASK_INDEX(sectionNumber);
    mask = 1 << GET_SECTION_MASK_SHIFT(sectionNumber);
    sectionMask[index] |= mask;

    if ((sectionNumber == 0) && (lastSectionNumber == 0))
    {
        /* shortcut for the most frequent and simple case */
        complete = true;
    }
    else
    {
        index = GET_SECTION_MASK_INDEX(lastSectionNumber);
        complete = true;

        /* Check the mask in the full slots */
        for (i = 0; i < index; i++)
        {
            if (sectionMask[i] != 0xFF)
            {
                complete = false;
                break;
            }
        }
        /* Now check the mask in the last slot, which can be incomplete */
        if (complete)
        {
            mask = 1u << (GET_SECTION_MASK_SHIFT(lastSectionNumber) + 1);
            mask--;
            if (sectionMask[index] != mask)
            {
                complete = false;
            }
        }
    }

    return complete;
}

/**
 * Parses a section of the AIT table and updates the table structure
 * @param dataPtr Pointer to the first section byte
 * @return true if the table structure has changed
 */
bool Ait::ParseSection(const uint8_t *dataPtr)
{
    bool changed = false;
    std::shared_ptr<S_AIT_TABLE> ait;
    const uint8_t *appData, *loopEnd;
    uint16_t appType;
    uint16_t descLen, appDescLen;
    uint8_t numApps, version, numNewApps;
    uint8_t i;
    uint8_t sectionNumber, lastSectionNumber;
    S_AIT_APP_DESC *appPtr;
    uint32_t orgId;
    uint16_t appId;

    /* Skip the table ID and section len */
    dataPtr += 3;

    appType = (*dataPtr << 8u) + *(dataPtr + 1);
    dataPtr += 2;

    version = (*dataPtr & 0x3Eu) >> 1u;

    dataPtr++;
    sectionNumber = *dataPtr;
    dataPtr++;
    lastSectionNumber = *dataPtr;

    if (appType != 0x0010)
    {
        LOG(LOG_DEBUG,
            "Ait::ParseSection AIT sub-table with unsupported application_type %x IGNORED",
            appType);
    }
    else if (m_ait == nullptr || !SectionReceived(m_ait.get(), sectionNumber) || m_ait->version !=
             version)
    {
        changed = true;

        if ((m_ait != nullptr) && (m_ait->version != version))
        {
            /* This version is different, discard the old table and create a new one */
            m_ait = nullptr;
        }

        /* Skip to the common descriptors */
        dataPtr += 1;
        descLen = ((*dataPtr & 0x0Fu) << 8u) + *(dataPtr + 1);
        dataPtr += 2;
        /* Skip the common descriptors */
        dataPtr += descLen;

        descLen = ((*dataPtr & 0x0Fu) << 8u) + *(dataPtr + 1);
        dataPtr += 2;

        numNewApps = 0;
        loopEnd = dataPtr + descLen;
        appData = dataPtr;
        for (numApps = 0; appData < loopEnd; numApps++)
        {
            appData += 4;
            appId = (*appData << 8u);
            appData++;
            appId += *appData;
            appData += 2;
            appDescLen = ((*appData & 0x0Fu) << 8u) + *(appData + 1);
            appData += 2 + appDescLen;
            if (m_ait != nullptr)
            {
                for (i = 0; i < m_ait->numApps; i++)
                {
                    if (m_ait->appArray[i].appId == appId)
                    {
                        break;
                    }
                }
                if (i == m_ait->numApps)
                {
                    /* app_id not present, count it */
                    numNewApps++;
                }
            }
        }
        if (m_ait == nullptr)
        {
            numNewApps = numApps;
        }

        LOG(LOG_DEBUG, "appType=%x, version=%u numApps=%u, section=%d/%d", appType, version,
            numApps, sectionNumber, lastSectionNumber);

        if (m_ait != nullptr)
        {
            ait = std::make_shared<S_AIT_TABLE>(*m_ait);
        }
        else
        {
            ait = std::make_shared<S_AIT_TABLE>();
            ait->appType = appType;
            ait->version = version;
            ait->numApps = 0;
        }

        ait->complete = MarkSectionReceived(ait.get(), sectionNumber, lastSectionNumber);
        if (numNewApps > 0)
        {
            LOG(LOG_DEBUG, "Ait::ParseSection %d new apps in this section", numNewApps);
            {
                ait->appArray.resize(ait->numApps + numNewApps);
                for (numApps = 0, appData = dataPtr; appData < loopEnd; numApps++)
                {
                    orgId = (*appData << 24u) + (*(appData + 1) << 16u) +
                        (*(appData + 2) << 8u) + *(appData + 3);
                    appData += 4;
                    appId = (*appData << 8u) + *(appData + 1);
                    appData += 2;

                    if (m_ait != nullptr)
                    {
                        for (i = 0; i < ait->numApps; i++)
                        {
                            if (ait->appArray[i].appId == appId)
                            {
                                break;
                            }
                        }
                    }
                    else
                    {
                        /* if table wasn't provided (first section) just add the
                         * applications as they come */
                        i = numApps;
                    }

                    appPtr = &(ait->appArray[i]);

                    /* Check if the app in this loop is already present in the incoming
                     * table structure */
                    if ((m_ait == nullptr) || (i == ait->numApps))
                    {
                        /* This is a new app */
                        appPtr->orgId = orgId;
                        appPtr->appId = appId;

                        appPtr->controlCode = *appData;
                        appData++;

                        appDescLen = ((*appData & 0x0Fu) << 8u) + *(appData + 1);
                        appData += 2;

                        /* Initialise the app_desc with an invalid visibility so that we
                           know what it has been parsed for this application */
                        appPtr->appDesc.visibility = 2;

                        ait->numApps++;

                        // f(stderr,"[%u] org_id=0x%08x, app_id=0x%04x,
                        // control_code=0x%02x, app_desc_len=%u \n\n",num_apps,
                        // app_ptr->org_id, app_ptr->app_id, app_ptr->control_code,
                        // app_desc_len);
                    }
                    else
                    {
                        /* The app is already present */
                        appData += 7;
                        appDescLen = ((*appData & 0x0Fu) << 8u) + *(appData + 1);
                        appData += 2;
                    }

                    ParseApplication(appData, appDescLen, appPtr);

                    appData += appDescLen;
                }
            }
        }
        else
        {
            LOG(LOG_DEBUG, "Ait::ParseSection Skip this section, no new apps (version=%u)",
                version);
        }

        m_ait.swap(ait);

#ifdef ANDROID_DEBUG
        if (m_ait != nullptr)
        {
            PrintInfo(m_ait.get());
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
 * @param parentalRatings List of parental ratings included in the AIT.
 * @param parentalControlAge PC age set in the device.
 * @param parentalControlRegion 2 letter ISO 3166 region code.
 * @param parentalControlRegion3 3 letter ISO 3166 region code.
 */
bool Ait::IsAgeRestricted(const std::vector<Ait::S_APP_PARENTAL_RATING> parentalRatings, int
    parentalControlAge,
    std::string &parentalControlRegion, std::string &parentalControlRegion3)
{
    bool restricted;
    if (!parentalRatings.empty())
    {
        restricted = true;
        for (const Ait::S_APP_PARENTAL_RATING& pr : parentalRatings)
        {
            LOG(LOG_ERROR, "APP_PARENTAL_RATING %s %s/%s/%s %d/%d", pr.scheme.c_str(),
                pr.region.c_str(), parentalControlRegion.c_str(),
                parentalControlRegion3.c_str(),
                pr.value, parentalControlAge);
            LOG(LOG_ERROR, "%d %d %d %d", (pr.scheme == "dvb-si"),
                ((pr.region.size() == 2) && (strcasecmp(pr.region.c_str(),
                    parentalControlRegion.c_str()) == 0)),
                ((pr.region.size() == 3) && (strcasecmp(pr.region.c_str(),
                    parentalControlRegion3.c_str()) == 0)),
                (pr.value <= parentalControlAge));
            LOG(LOG_ERROR, "%d %d", strcasecmp(pr.region.c_str(), parentalControlRegion.c_str()),
                (strcasecmp(pr.region.c_str(), parentalControlRegion3.c_str()) == 0));
            if ((pr.scheme == "dvb-si") &&
                (((pr.region.size() == 2) && (strcasecmp(pr.region.c_str(),
                    parentalControlRegion.c_str()) == 0)) ||
                 ((pr.region.size() == 3) && (strcasecmp(pr.region.c_str(),
                     parentalControlRegion3.c_str()) == 0))) &&
                (pr.value <= parentalControlAge))
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
