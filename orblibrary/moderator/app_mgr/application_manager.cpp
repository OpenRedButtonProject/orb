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
 * Application manager
 *
 * Note: This file is part of the platform-agnostic application manager library.
 */

#include "application_manager.h"

#include <memory>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <cstring>
#include <iostream>
#include <string>
#include <stdexcept>

#include "ait.h"
#include "log.h"
#include "utils.h"
#include "xml_parser.h"

namespace orb
{

ApplicationManager::ApplicationManager(std::unique_ptr<IXmlParser> xmlParser) :
    m_sessionCallback{nullptr},
    m_xmlParser(std::move(xmlParser)),
    m_aitTimeout([&] {
        onSelectedServiceAitTimeout();
    })
{
}

ApplicationManager::~ApplicationManager() = default;

ApplicationManager& ApplicationManager::instance()
{
    static ApplicationManager s_instance;
    return s_instance;
}

void ApplicationManager::SetXmlParser(std::unique_ptr<IXmlParser> xmlParser)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    m_xmlParser = std::move(xmlParser);
}


void ApplicationManager::RegisterCallback(ApplicationType apptype, ApplicationSessionCallback* callback)
{
    if (apptype <= APP_TYPE_OPAPP) {
        // Store the raw pointer without taking ownership (allow nullptr for cleanup)
        m_sessionCallback[apptype] = callback;
    }
    else {
        LOG(LOG_ERROR, "Invalid param: atype=%u cb=%p", apptype, callback);
    }
}


void ApplicationManager::SetCurrentInterface(ApplicationType apptype)
{
    if (apptype <= APP_TYPE_OPAPP) {
        m_cif = (int)apptype;
    }
    else {
        LOG(LOG_ERROR, "Invalid param: atype=%u", apptype);
    }
}

int ApplicationManager::CreateApplication(int callingAppId, const std::string &url, bool runAsOpApp)
{
    int result = INVALID_APP_ID;
    const Ait::S_AIT_APP_DESC *appDescription;
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    LOG(LOG_INFO, "CreateApplication");
    if (m_apps.count(callingAppId) == 0)
    {
        LOG(LOG_INFO, "Called by non-running app, early out");
        return INVALID_APP_ID;
    }
    if (url.empty())
    {
        LOG(LOG_INFO, "Called with empty URL, early out");
        if (m_sessionCallback[m_cif]) {
            m_sessionCallback[m_cif]->DispatchApplicationLoadErrorEvent();
        }
        return INVALID_APP_ID;
    }

    if (runAsOpApp && m_apps[callingAppId]->GetType() != HbbTVApp::OPAPP_TYPE)
    {
        LOG(LOG_INFO, "Called with runAsOpApp=true from a non-opapp, early out");
        return INVALID_APP_ID;
    }

    Utils::CreateLocatorInfo info = Utils::ParseCreateLocatorInfo(url, m_currentService);
    switch (info.type)
    {
        case Utils::CreateLocatorType::AIT_APPLICATION_LOCATOR:
        {
            LOG(LOG_INFO, "Create for AIT_APPLICATION_LOCATOR (url=%s)", url.c_str());
            if (m_ait.Get() == nullptr)
            {
                LOG(LOG_INFO, "No AIT, early out");
                break;
            }
            appDescription = Ait::FindApp(m_ait.Get(), info.orgId, info.appId);
            if (appDescription && Ait::HasViableTransport(appDescription, m_isNetworkAvailable))
            {
                result = CreateAndRunApp(*appDescription, info.parameters, true, false, runAsOpApp);
            }
            else
            {
                LOG(LOG_ERROR, "Could not find app (org_id=%d, app_id=%d)",
                    info.orgId,
                    info.appId);
            }

            break;
        }

        case Utils::CreateLocatorType::ENTRY_PAGE_OR_XML_AIT_LOCATOR:
        {
            LOG(LOG_INFO, "Create for ENTRY_PAGE_OR_XML_AIT_LOCATOR (url=%s)", url.c_str());
            std::string contents;
            if (m_sessionCallback[m_cif]) {
                contents = m_sessionCallback[m_cif]->GetXmlAitContents(url);
            }
            if (!contents.empty())
            {
                LOG(LOG_INFO, "Locator resource is XML AIT");
                result = ProcessXmlAit(contents, false);
            }
            else
            {
                LOG(LOG_INFO, "Locator resource is ENTRY PAGE");
                result = CreateAndRunApp(url, runAsOpApp);
            }
            break;
        }

        case Utils::CreateLocatorType::UNKNOWN_LOCATOR:
        {
            LOG(LOG_INFO, "Do not create for UNKNOWN_LOCATOR (url=%s)", url.c_str());
            result = INVALID_APP_ID;
            break;
        }
    }

    if (result == INVALID_APP_ID)
    {
        if (m_sessionCallback[m_cif]) {
            m_sessionCallback[m_cif]->DispatchApplicationLoadErrorEvent();
        }
    }

    return result;
}

int ApplicationManager::CreateAndRunApp(std::string url, bool runAsOpApp)
{
    int result;
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (url.empty())
    {
        LOG(LOG_ERROR, "URL is empty");
        result = INVALID_APP_ID;
    }
    else if (!m_sessionCallback[m_cif])
    {
        LOG(LOG_ERROR, "Callback is NULL");
        result = INVALID_APP_ID;
     }
    else
    {
        if (runAsOpApp)
        {
            result = runApp(std::make_unique<OpApp>(url, m_sessionCallback[m_cif]));
        }
        else
        {
            result = runApp(std::make_unique<HbbTVApp>(url, m_sessionCallback[m_cif]));
        }
    }
    return result;
}

int ApplicationManager::CreateAndRunApp(
    const Ait::S_AIT_APP_DESC &desc,
    const std::string &urlParams,
    bool isBroadcast,
    bool isTrusted,
    bool runAsOpApp
){
    int result;
    std::unique_ptr<HbbTVApp> app;
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    // ETSI TS 103 606 V1.2.1 (2024-03) Table 7: XML AIT Profile
    if (m_sessionCallback[m_cif]) {
        if (runAsOpApp || desc.appUsage == "urn:hbbtv:opapp:privileged:2017" ||
            desc.appUsage == "urn:hbbtv:opapp:opspecific:2017")
        {
            app = std::make_unique<OpApp>(m_sessionCallback[m_cif]);
        }
        else
        {
            app = std::make_unique<HbbTVApp>(m_currentService, isBroadcast, isTrusted, m_sessionCallback[m_cif]);
        }
    }
    app->SetUrl(desc, urlParams, m_isNetworkAvailable);
    if (!app->Update(desc, m_isNetworkAvailable))
    {
        LOG(LOG_ERROR, "Update failed");
        result = INVALID_APP_ID;
    }
    else
    {
        result = runApp(std::move(app));
    }
    return result;
}

void ApplicationManager::DestroyApplication(int callingAppId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    LOG(LOG_ERROR, "DestroyApplication");
    if (callingAppId == INVALID_APP_ID)
    {
        killRunningApp(callingAppId);
        onRunningAppExited();
    }
    if (m_apps.count(callingAppId) == 0)
    {
        LOG(LOG_INFO, "Called by non-running app, early out");
        return;
    }

    killRunningApp(callingAppId);
    onRunningAppExited();
}

void ApplicationManager::ShowApplication(int callingAppId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (m_apps.count(callingAppId) > 0)
    {
        m_apps[callingAppId]->SetState(HbbTVApp::FOREGROUND_STATE);
    }
}

void ApplicationManager::HideApplication(int callingAppId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (m_apps.count(callingAppId) > 0)
    {
        m_apps[callingAppId]->SetState(HbbTVApp::BACKGROUND_STATE);
    }
}

uint16_t ApplicationManager::SetKeySetMask(int appId, uint16_t keySetMask, std::vector<uint16_t> otherKeys)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    if (m_apps.count(appId) == 0)
    {
        return 0;
    }

    return m_apps[appId]->SetKeySetMask(keySetMask, otherKeys);
}

uint16_t ApplicationManager::GetKeySetMask(int appId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (m_apps.count(appId) > 0)
    {
        return m_apps[appId]->GetKeySetMask();
    }
    return 0;
}

std::vector<uint16_t> ApplicationManager::GetOtherKeyValues(int appId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (m_apps.count(appId) > 0)
    {
        return m_apps[appId]->GetOtherKeyValues();
    }
    return std::vector<uint16_t>();
}

std::string ApplicationManager::GetApplicationScheme(int appId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (m_apps.count(appId) > 0)
    {
        return m_apps[appId]->GetScheme();
    }
    return LINKED_APP_SCHEME_1_1;
}

std::vector<int> ApplicationManager::GetRunningAppIds()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    std::vector<int> ids;
    for (const auto& pair : m_apps)
    {
        LOG(LOG_INFO, "GetRunningAppIds(): %d", pair.first);
        ids.push_back(pair.first);
    }
    return ids;
}

std::string ApplicationManager::GetApplicationUrl(int appId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (m_apps.count(appId) > 0)
    {
        LOG(LOG_INFO, "GetApplicationUrl(%d): %s", appId, m_apps[appId]->loadedUrl.c_str());
        return m_apps[appId]->loadedUrl;
    }
    return std::string();
}

bool ApplicationManager::InKeySet(int appId, uint16_t keyCode)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (m_apps.count(appId) > 0)
    {
        return m_apps[appId]->InKeySet(keyCode);
    }
    LOG(LOG_INFO, "InKeySet(): No app with id %d found. Returning false.", appId);
    return false;
}

void ApplicationManager::ProcessAitSection(uint16_t aitPid, uint16_t serviceId,
    const uint8_t *sectionData, uint32_t sectionDataBytes)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    LOG(LOG_INFO, "ProcessAitSection");

    if (serviceId != m_currentService.serviceId)
    {
        LOG(LOG_INFO, "The AIT is for service %x, not current service %x, early out",serviceId, m_currentService.serviceId);
        return;
    }

    if (aitPid != m_currentServiceAitPid)
    {
        if (m_currentServiceAitPid != 0)
        {
            LOG(LOG_INFO, "The AIT comes in a different PID, now=%d before= %d", aitPid,
                m_currentServiceAitPid);
            m_ait.Clear();
        }
        m_currentServiceAitPid = aitPid;
    }

    if (!m_ait.ProcessSection(sectionData, sectionDataBytes))
    {
        LOG(LOG_INFO, "The AIT was not completed and/or updated, early out");
        return;
    }

    const Ait::S_AIT_TABLE *updated_ait = m_ait.Get();
    if (updated_ait == nullptr)
    {
        LOG(LOG_ERROR, "No AIT, early out");
        return;
    }

    if (!m_currentServiceReceivedFirstAit)
    {
        m_aitTimeout.stop();
        m_currentServiceReceivedFirstAit = true;
        onSelectedServiceAitReceived();
    }
    else
    {
        onSelectedServiceAitUpdated();
    }
}


int ApplicationManager::ProcessXmlAit(
    const std::string &xmlAit,
    const bool isDvbi,
    const std::string &scheme
){
    const Ait::S_AIT_APP_DESC *app_description;
    bool result = INVALID_APP_ID;

    std::lock_guard<std::recursive_mutex> lock(m_lock);

    LOG(LOG_INFO, "ProcessXmlAit");

    if (xmlAit.empty())
    {
        return INVALID_APP_ID;
    }

    if (m_xmlParser == nullptr)
    {
        LOG(LOG_ERROR, "No XML parser provided");
        return INVALID_APP_ID;
    }

    std::unique_ptr<Ait::S_AIT_TABLE> aitTable = m_xmlParser->ParseAit(xmlAit.c_str(),
        xmlAit.length());
    if (nullptr == aitTable || aitTable->numApps == 0)
    {
        // No AIT or apps parsed, early out
        return INVALID_APP_ID;
    }
    for (int index = 0; index != aitTable->numApps; index++)
    {
        aitTable->appArray[index].scheme = scheme;
    }
    Ait::PrintInfo(aitTable.get());
    if (isDvbi)
    {
        m_ait.Clear();
        m_currentServiceAitPid = UINT16_MAX;
        m_ait.ApplyAitTable(aitTable);

        if (!m_currentServiceReceivedFirstAit)
        {
            m_aitTimeout.stop();
            m_currentServiceReceivedFirstAit = true;
            onSelectedServiceAitReceived();
        }
        else
        {
            onSelectedServiceAitUpdated();
        }
        result = m_hbbtvAppId;
    }
    else
    {
        app_description = getAutoStartApp(aitTable.get());

        if (app_description)
        {
            result = CreateAndRunApp(*app_description, "", isDvbi, false);
            if (result == INVALID_APP_ID)
            {
                LOG(LOG_ERROR, "Could not find app (org_id=%d, app_id=%d)",
                    app_description->orgId,
                    app_description->appId);
            }
        }
    }

    return result;
}

bool ApplicationManager::IsTeletextApplicationSignalled()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (m_ait.Get() == nullptr)
    {
        return false;
    }
    return Ait::TeletextApp(m_ait.Get()) != nullptr;
}

bool ApplicationManager::RunTeletextApplication()
{
    const Ait::S_AIT_APP_DESC *appDescription;
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    LOG(LOG_INFO, "RunTeletextApplication");

    if (m_ait.Get() == nullptr)
    {
        return false;
    }
    appDescription = Ait::TeletextApp(m_ait.Get());
    if (appDescription == nullptr)
    {
        LOG(LOG_ERROR, "Could not find Teletext app");

        return false;
    }
    return CreateAndRunApp(*appDescription, "", true, false);
}

bool ApplicationManager::IsRequestAllowed(int callingAppId, const
    std::string &callingPageUrl,
    MethodRequirement methodRequirement)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    if (m_apps.count(m_hbbtvAppId) == 0 || m_hbbtvAppId != callingAppId)
    {
        return false;
    }

    if (callingPageUrl.empty() || Utils::CompareUrls(callingPageUrl, "about:blank"))
    {
        return false;
    }

    switch (methodRequirement)
    {
        case MethodRequirement::FOR_RUNNING_APP_ONLY:
        {
            return true;
        }
        case MethodRequirement::FOR_BROADCAST_APP_ONLY:
        {
            return m_apps[m_hbbtvAppId]->IsBroadcast();
        }
        case MethodRequirement::FOR_BROADCAST_OR_TRANSITIONING_APP_ONLY:
        {
            return !Utils::IsInvalidDvbTriplet(m_currentService);
        }
        case MethodRequirement::FOR_TRUSTED_APP_ONLY:
        {
            // Check document URL is inside app boundaries
            if (!Utils::CheckBoundaries(callingPageUrl, m_apps[m_hbbtvAppId]->GetEntryUrl(), m_apps[m_hbbtvAppId]->GetAitDescription().boundaries))
            {
                return false;
            }
            return m_apps[m_hbbtvAppId]->IsTrusted();
        }
        default:
        {
            return false;
        }
    }
}

std::map<std::string, std::string> ApplicationManager::GetCurrentAppNames()
{
    std::map<std::string, std::string> result;
    LOG(LOG_DEBUG, "GetCurrentAppNames");
    if (m_apps.count(m_hbbtvAppId) > 0)
    {
        std::map<uint32_t, std::string> names = m_apps[m_hbbtvAppId]->GetNames();
        std::map<uint32_t, std::string>::iterator it = names.begin();
        while (it != names.end())
        {
            uint32_t lang_code = it->first;
            std::string name = it->second;
            std::string langCodeString("");
            langCodeString += static_cast<char>((lang_code >> 16) & 0xff);
            langCodeString += static_cast<char>((lang_code >> 8) & 0xff);
            langCodeString += static_cast<char>((lang_code & 0xff));
            result[langCodeString] = name;
            LOG(LOG_DEBUG, "lang=%s name=%s", langCodeString.c_str(), name.c_str());
            it++;
        }
    }
    return result;
}

void ApplicationManager::OnBroadcastStopped()
{
    LOG(LOG_DEBUG, "OnBroadcastStopped");
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    m_currentServiceReceivedFirstAit = false;
    m_currentServiceAitPid = 0;
    m_ait.Clear();
    m_previousService = m_currentService = Utils::MakeInvalidDvbTriplet();
    if (!transitionRunningAppToBroadcastIndependent())
    {
        LOG(LOG_INFO, "Kill running app (could not transition to broadcast-independent)");
        killRunningApp(m_hbbtvAppId);
    }
}

void ApplicationManager::OnChannelChanged(uint16_t originalNetworkId,
    uint16_t transportStreamId, uint16_t serviceId)
{
    LOG(LOG_DEBUG, "OnChannelChanged (current service: %d)", m_currentService.serviceId);
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    m_currentServiceReceivedFirstAit = false;
    m_currentServiceAitPid = 0;
    m_ait.Clear();
    m_aitTimeout.start(std::chrono::milliseconds(Utils::AIT_TIMEOUT));
    m_previousService = m_currentService;
    m_currentService = {
        .originalNetworkId = originalNetworkId,
        .transportStreamId = transportStreamId,
        .serviceId = serviceId,
    };
}

void ApplicationManager::OnNetworkAvailabilityChanged(bool available)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    LOG(LOG_DEBUG, "OnNetworkAvailabilityChanged available=%d", available);
    m_isNetworkAvailable = available;
}

void ApplicationManager::OnLoadApplicationFailed(int appId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    // TODO If a call to createApplication has failed, set app_ back to old_app_ and send event?

    if (Utils::IsInvalidDvbTriplet(m_currentService))
    {
        LOG(LOG_ERROR,
            "Unhandled condition (failed to load application while broadcast-independent)");
        return;
    }

    if (m_apps.count(appId) == 0)
    {
        return;
    }
    auto ait = m_ait.Get();
    Ait::S_AIT_APP_DESC aitDesc = m_apps[appId]->GetAitDescription();
    if (ait != nullptr && aitDesc.appId != 0 && aitDesc.orgId != 0)
    {
        Ait::S_AIT_APP_DESC *app = Ait::FindApp(ait, aitDesc.orgId, aitDesc.appId);
        Ait::AppSetTransportFailedToLoad(app, m_apps[appId]->GetProtocolId());
    }
    killRunningApp(appId);
    onPerformBroadcastAutostart();
}

void ApplicationManager::OnApplicationPageChanged(int appId, const std::string &url)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (m_apps.count(appId) > 0)
    {
        m_apps[appId]->loadedUrl = url;
        if (!Utils::IsInvalidDvbTriplet(m_currentService) &&
            url.find("https://www.live.bbctvapps.co.uk/tap/iplayer") == std::string::npos)
        {
            // For broadcast-related applications we reset the broadcast presentation on page change,
            // as dead JS objects may have suspended presentation, set the video rectangle or set
            // the presented components.
            if (m_sessionCallback[m_cif]) {
                m_sessionCallback[m_cif]->ResetBroadcastPresentation();
            }
        }
    }
}

// Private methods...

void ApplicationManager::onSelectedServiceAitReceived()
{
    LOG(LOG_INFO, "OnSelectedServiceAitReceived");
    auto ait = m_ait.Get();
    if (ait != nullptr)
    {
        LOG(LOG_INFO, "New service selected and first AIT received");

        if (m_apps.count(m_hbbtvAppId) > 0)
        {
            if (m_apps[m_hbbtvAppId]->IsBroadcast())
            {
                Ait::S_AIT_APP_DESC aitDesc = m_apps[m_hbbtvAppId]->GetAitDescription();
                LOG(LOG_INFO,
                    "OnSelectedServiceAitReceived: Pre-existing broadcast-related app already running");
                if (aitDesc.appDesc.serviceBound && m_sessionCallback[m_cif] && !m_sessionCallback[m_cif]->isInstanceInCurrentService(m_previousService))
                {
                    LOG(LOG_INFO, "Kill running app (is service bound)");
                    killRunningApp(m_hbbtvAppId);
                }
                else
                {
                    auto signalled = Ait::FindApp(ait, aitDesc.orgId, aitDesc.appId);
                    if (signalled == nullptr)
                    {
                        LOG(LOG_INFO, "Kill running app (is not signalled in the new AIT)");
                        killRunningApp(m_hbbtvAppId);
                    }
                    else if (signalled->controlCode == Ait::APP_CTL_KILL)
                    {
                        LOG(LOG_INFO, "Kill running app (signalled with control code KILL)");
                        killRunningApp(m_hbbtvAppId);
                    }
                    else if (!Ait::AppHasTransport(signalled, m_apps[m_hbbtvAppId]->GetProtocolId()))
                    {
                        LOG(LOG_INFO,
                            "Kill running app (is not signalled in the new AIT with the same transport protocol)");
                        killRunningApp(m_hbbtvAppId);
                    }
                    else if (!updateRunningApp(*signalled))
                    {
                        killRunningApp(m_hbbtvAppId);
                    }
                }
            }
            else
            {
                LOG(LOG_INFO, "Pre-existing broadcast-independent app already running");
                if (!transitionRunningAppToBroadcastRelated())
                {
                    LOG(LOG_INFO, "Kill running app (could not transition to broadcast-related)");
                    killRunningApp(m_hbbtvAppId);
                }
            }
        }
        if (m_apps.count(m_hbbtvAppId) == 0)
        {
            onPerformBroadcastAutostart();
        }
        else
        {
            Ait::S_AIT_APP_DESC aitDesc = m_apps[m_hbbtvAppId]->GetAitDescription();
            auto signalled = Ait::FindApp(ait, aitDesc.orgId, aitDesc.appId);
            if (signalled != nullptr && !updateRunningApp(*signalled))
            {
                killRunningApp(m_hbbtvAppId);
            }
        }
    }
}

void ApplicationManager::onSelectedServiceAitTimeout()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    LOG(LOG_INFO, "OnSelectedServiceAitTimeout");
    killRunningApp(m_hbbtvAppId);
}

void ApplicationManager::onSelectedServiceAitUpdated()
{
    auto ait = m_ait.Get();
    LOG(LOG_INFO, "OnSelectedServiceAitUpdated");
    if (ait == nullptr)
    {
        LOG(LOG_ERROR, "Unexpected condition (AIT updated but is missing)");
        return;
    }

    if (m_apps.count(m_hbbtvAppId) > 0)
    {
        if (!m_apps[m_hbbtvAppId]->IsBroadcast())
        {
            // If the running app is not broadcast-related, we should not be tuned to broadcast
            LOG(LOG_ERROR, "Unexpected condition (AIT updated but app is not broadcast-related)");
            return;
        }

        Ait::S_AIT_APP_DESC aitDesc = m_apps[m_hbbtvAppId]->GetAitDescription();
        LOG(LOG_INFO,
            "OnSelectedServiceAitUpdated: Pre-existing broadcast-related app already running");
        auto signalled = Ait::FindApp(ait, aitDesc.orgId, aitDesc.appId);
        if (signalled == nullptr)
        {
            LOG(LOG_INFO, "Kill running app (is not signalled in the updated AIT)");
            killRunningApp(m_hbbtvAppId);
        }
        else if (!Ait::AppHasTransport(signalled, m_apps[m_hbbtvAppId]->GetProtocolId()))
        {
            LOG(LOG_INFO,
                "Kill running app (is not signalled in the updated AIT with the same transport protocol)");
            killRunningApp(m_hbbtvAppId);
        }
        else if (signalled->controlCode == Ait::APP_CTL_KILL)
        {
            LOG(LOG_INFO, "Kill running app (signalled has control code KILL)");
            killRunningApp(m_hbbtvAppId);
        }
        else if (!updateRunningApp(*signalled))
        {
            killRunningApp(m_hbbtvAppId);
        }
    }

    if (m_apps.count(m_hbbtvAppId) == 0)
    {
        onPerformBroadcastAutostart();
    }
}

void ApplicationManager::onRunningAppExited()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    LOG(LOG_ERROR, "OnRunningAppExited");
    if (!Utils::IsInvalidDvbTriplet(m_currentService))
    {
        onPerformBroadcastAutostart();
    }
    else
    {
        // TODO This behaviour is implementation specific
        LOG(LOG_ERROR, "Unhandled condition (broadcast-independent app exited)");
    }
}

/**
 * Called at a time when the broadcast autostart app should be started.
 */
void ApplicationManager::onPerformBroadcastAutostart()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    LOG(LOG_ERROR, "OnPerformAutostart");

    // Find autostart app_desc
    auto ait = m_ait.Get();
    if (!m_currentServiceReceivedFirstAit || ait == nullptr)
    {
        LOG(LOG_INFO, "OnPerformAutostart No service selected/AIT, early out");
        return;
    }
    auto app_desc = getAutoStartApp(ait);

    if (app_desc != nullptr)
    {
        LOG(LOG_ERROR, "OnPerformAutostart Start autostart app.");
        CreateAndRunApp(*app_desc, "", true, false);
    }
    else
    {
        LOG(LOG_INFO, "OnPerformAutostart No viable autostart app found. isNetworkAvailable? %u", m_isNetworkAvailable);
    }
}

int ApplicationManager::runApp(std::unique_ptr<HbbTVApp> app)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    int *appId = &m_hbbtvAppId;
    if (app->GetType() == HbbTVApp::OPAPP_TYPE)
    {
        appId = &m_opAppId;
    }

    m_apps.erase(*appId);
    if (!app->IsBroadcast() && !Utils::IsInvalidDvbTriplet(m_currentService))
    {
        if (m_sessionCallback[m_cif]) {
            m_sessionCallback[m_cif]->StopBroadcast();
        }
        m_previousService = m_currentService = Utils::MakeInvalidDvbTriplet();
    }

    if (m_sessionCallback[m_cif]) {
        m_sessionCallback[m_cif]->LoadApplication(app->GetId(), app->GetEntryUrl().c_str(),
            app->GetAitDescription().graphicsConstraints.size(), app->GetAitDescription().graphicsConstraints);
    }

    *appId = app->GetId();
    m_apps[*appId] = std::move(app);

    // Call explicitly Show/Hide
    if (m_sessionCallback[m_cif]) {
        if (m_apps[*appId]->GetState() != HbbTVApp::BACKGROUND_STATE)
        {
            m_sessionCallback[m_cif]->ShowApplication(*appId);
        }
        else
        {
            m_sessionCallback[m_cif]->HideApplication(*appId);
        }
    }
    return *appId;
}

bool ApplicationManager::updateRunningApp(const Ait::S_AIT_APP_DESC &desc)
{
    return m_apps[m_hbbtvAppId]->Update(desc, m_isNetworkAvailable);
}

void ApplicationManager::killRunningApp(int appid)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if ((m_hbbtvAppId == appid || m_opAppId == appid) && m_apps.erase(appid) > 0)
    {
        if (m_sessionCallback[m_cif]) {
            m_sessionCallback[m_cif]->HideApplication(appid);
            m_sessionCallback[m_cif]->LoadApplication(INVALID_APP_ID, "about:blank");
        }
        if (appid == m_hbbtvAppId)
        {
            m_hbbtvAppId = INVALID_APP_ID;
        }
        else
        {
            m_opAppId = INVALID_APP_ID;
        }
    }
}

/**
 * Transition the running app to broadcast related, if conditions permit.
 *
 * @return True on success, false on failure.
 */
bool ApplicationManager::transitionRunningAppToBroadcastRelated()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    LOG(LOG_INFO, "TransitionRunningAppToBroadcastRelated");
    auto ait = m_ait.Get();
    if (ait == nullptr)
    {
        LOG(LOG_INFO, "Cannot transition to broadcast (no broadcast AIT)");
        return false;
    }
    if (m_apps.count(m_hbbtvAppId) == 0)
    {
        LOG(LOG_INFO, "Cannot transition to broadcast (no running app)");
        return false;
    }
    Ait::S_AIT_APP_DESC aitDesc = m_apps[m_hbbtvAppId]->GetAitDescription();
    if (aitDesc.appId == 0 || aitDesc.orgId == 0)
    {
        LOG(LOG_INFO, "Cannot transition to broadcast (app/org id is 0)");
        return false;
    }
    // get the latest ait description from the ait table
    const Ait::S_AIT_APP_DESC *app = Ait::FindApp(ait, aitDesc.orgId, aitDesc.appId);
    if (app == nullptr)
    {
        LOG(LOG_INFO, "Cannot transition to broadcast (app is not signalled in the new AIT)");
        return false;
    }

    if (!updateRunningApp(*app))
    {
        return false;
    }

    /* Note: what about app.is_trusted, app.parental_ratings, ... */
    return m_apps[m_hbbtvAppId]->TransitionToBroadcastRelated();
}

/**
 * Transition the running app to broadcast-independent, if conditions permit.
 *
 * @return true on success, false on failure.
 */
bool ApplicationManager::transitionRunningAppToBroadcastIndependent()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (m_apps.count(m_hbbtvAppId) > 0)
    {
        return m_apps[m_hbbtvAppId]->TransitionToBroadcastIndependent();
    }
    return true;
}

bool ApplicationManager::isAppTrusted(bool)
{
    // TODO See specification. Probably need to add more parameters to this method
    return false;
}

/**
 * Call to Ait::AutoStartApp() passing the parental restrictions.
 *
 * @param aitTable AIT table.
 * @return The App to auto start.
 */
const Ait::S_AIT_APP_DESC * ApplicationManager::getAutoStartApp(const Ait::S_AIT_TABLE *aitTable)
{
    //int index;
    LOG(LOG_ERROR, "GetAutoStartApp");

    /* Note: XML AIt uses the alpha-2 region codes as defined in ISO 3166-1.
     * DVB's parental_rating_descriptor uses the 3-character code as specified in ISO 3166. */
    std::string parentalControlRegion;
    std::string parentalControlRegion3;
    int parentalControlAge = 0;
    if (m_sessionCallback[m_cif]) {
        parentalControlRegion = m_sessionCallback[m_cif]->GetParentalControlRegion();
        parentalControlRegion3 = m_sessionCallback[m_cif]->GetParentalControlRegion3();
        parentalControlAge = m_sessionCallback[m_cif]->GetParentalControlAge();
    }
    return Ait::AutoStartApp(aitTable, parentalControlAge, parentalControlRegion,
        parentalControlRegion3, m_isNetworkAvailable);
}

uint32_t ApplicationManager::GetOrganizationId()
{
    if (m_apps.count(m_hbbtvAppId) > 0)
    {
        LOG(LOG_INFO, "The organization id is %d\n", m_apps[m_hbbtvAppId]->GetAitDescription().orgId);
        return m_apps[m_hbbtvAppId]->GetAitDescription().orgId;
    }
    LOG(LOG_INFO, "Cannot retrieve organization id (no running app). Returning -1.\n");
    return -1;
}

} // namespace orb
