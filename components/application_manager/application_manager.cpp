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
#include <cstring>
#include <iostream>
#include <string>
#include <stdexcept>

#include "ait.h"
#include "log.h"
#include "utils.h"
#include "xml_parser.h"

class AppSessionCallback : public HbbTVApp::SessionCallback
{
public:
    AppSessionCallback(ApplicationManager::SessionCallback *sessionCallback, uint16_t *currentAppId)
        : m_sessionCallback(sessionCallback), m_currentAppId(currentAppId)
    { }
    
    void ShowApplication(uint16_t appId) {
        if (appId == *m_currentAppId) m_sessionCallback->ShowApplication();
    }

    void HideApplication(uint16_t appId) {
        if (appId == *m_currentAppId) m_sessionCallback->HideApplication();
    }
    
    void DispatchTransitionedToBroadcastRelatedEvent(uint16_t appId) { 
        if (appId == *m_currentAppId) m_sessionCallback->DispatchTransitionedToBroadcastRelatedEvent();
    }
    
    void DispatchApplicationSchemeUpdatedEvent(uint16_t appId, const std::string &scheme) {
        if (appId == *m_currentAppId) m_sessionCallback->DispatchApplicationSchemeUpdatedEvent(scheme);
    }
    
    int GetParentalControlAge() {
        return m_sessionCallback->GetParentalControlAge();
    }
    
    std::string GetParentalControlRegion() {
        return m_sessionCallback->GetParentalControlRegion();
    }
    
    std::string GetParentalControlRegion3() {
        return m_sessionCallback->GetParentalControlRegion3();
    }

private:
    ApplicationManager::SessionCallback *m_sessionCallback;
    uint16_t *m_currentAppId;
};

/**
 * Application manager
 *
 * @param sessionCallback Implementation of ApplicationManager::SessionCallback interface.
 */
ApplicationManager::ApplicationManager(std::unique_ptr<ApplicationManager::SessionCallback> sessionCallback) :
    m_sessionCallback(std::move(sessionCallback)),
    m_aitTimeout([&] {
        OnSelectedServiceAitTimeout();
    }),
    m_appSessionCallback(std::make_shared<AppSessionCallback>(m_sessionCallback.get(), &m_appId))
{
    m_sessionCallback->HideApplication();
}

/**
 *
 */
ApplicationManager::~ApplicationManager() = default;

/**
 * Create and run a new application. If called by an application, check it is allowed.
 *
 * @param callingAppId The calling app ID or INVALID_APP_ID if not called by an app.
 * @param url A HTTP/HTTPS or DVB URL.
 *
 * A HTTP/HTTPS URL may refer to the entry page or XML AIT of a broadcast-independent app.
 *
 * A DVB URL may refer to a broadcast-related app signalled in the current service AIT. This
 * will result in the signalled URL being loaded, which may be HTTP/HTTPS for broadband or DVB
 * for carousel.
 *
 * @return true if the application can be created, otherwise false
 */
bool ApplicationManager::CreateApplication(uint16_t callingAppId, const std::string &url)
{
    bool result = false;
    const Ait::S_AIT_APP_DESC *appDescription;
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    LOG(LOG_INFO, "CreateApplication");
    if (m_apps.count(callingAppId) <= 0)
    {
        LOG(LOG_INFO, "Called by non-running app, early out");
        return false;
    }
    if (url.empty())
    {
        LOG(LOG_INFO, "Called with empty URL, early out");
        m_sessionCallback->DispatchApplicationLoadErrorEvent();
        return false;
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
            if (appDescription)
            {
                result = CreateAndRunApp(*appDescription, info.parameters, true, false);
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
            std::string contents = m_sessionCallback->GetXmlAitContents(url);
            if (!contents.empty())
            {
                LOG(LOG_INFO, "Locator resource is XML AIT");
                result = ProcessXmlAit(contents, false);
            }
            else
            {
                LOG(LOG_INFO, "Locator resource is ENTRY PAGE");
                result = CreateAndRunApp(url);
            }
            break;
        }

        case Utils::CreateLocatorType::UNKNOWN_LOCATOR:
        {
            LOG(LOG_INFO, "Do not create for UNKNOWN_LOCATOR (url=%s)", url.c_str());
            result = false;
            break;
        }
    }

    if (!result)
    {
        m_sessionCallback->DispatchApplicationLoadErrorEvent();
    }

    return result;
}

/**
 * Destroy the calling application.
 *
 * @param callingAppId The calling app ID.
 */
void ApplicationManager::DestroyApplication(uint16_t callingAppId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    LOG(LOG_ERROR, "DestroyApplication");
    if (callingAppId == INVALID_APP_ID)
    {
        KillRunningApp();
        OnRunningAppExited();
    }
    if (m_apps.count(callingAppId) <= 0)
    {
        LOG(LOG_INFO, "Called by non-running app, early out");
        return;
    }

    KillRunningApp();
    OnRunningAppExited();
}

/**
 * Show the calling application.
 *
 * @param callingAppId The calling app ID.
 */
void ApplicationManager::ShowApplication(uint16_t callingAppId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (m_apps.count(callingAppId) > 0)
    {
        m_apps[callingAppId]->SetState(HbbTVApp::FOREGROUND_STATE);
    }
}

/**
 * Hide the calling application.
 *
 * @param callingAppId The calling app ID.
 */
void ApplicationManager::HideApplication(uint16_t callingAppId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (m_apps.count(callingAppId) > 0)
    {
        m_apps[callingAppId]->SetState(HbbTVApp::BACKGROUND_STATE);
    }
}

/**
 * Set the key set mask for an application.
 *
 * @param appId The application.
 * @param keySetMask The key set mask.
 * @param otherKeys optional other keys
 * @return The key set mask for the application.
 */
uint16_t ApplicationManager::SetKeySetMask(uint16_t appId, uint16_t keySetMask, std::vector<uint16_t> otherKeys)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    if (m_apps.count(appId) <= 0)
    {
        return 0;
    }

    return m_apps[appId]->SetKeySetMask(keySetMask, otherKeys);
}

/**
 * Get the key set mask for an application.
 *
 * @param appId The application.
 * @return The key set mask for the application.
 */
uint16_t ApplicationManager::GetKeySetMask(uint16_t appId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (m_apps.count(appId) > 0)
    {
        return m_apps[appId]->GetKeySetMask();
    }
    return 0;
}

/**
 * Get the other keys for an application.
 *
 * @param appId The application.
 * @return The other keys for the application.
 */
std::vector<uint16_t> ApplicationManager::GetOtherKeyValues(uint16_t appId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (m_apps.count(appId) > 0)
    {
        return m_apps[appId]->GetOtherKeyValues();
    }
    return std::vector<uint16_t>();
}

std::string ApplicationManager::GetApplicationScheme(uint16_t appId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (m_apps.count(appId) > 0)
    {
        return m_apps[appId]->GetScheme();
    }
    return LINKED_APP_SCHEME_1_1;
}

/**
 * Check the key code is accepted by the current key mask. Activate the app as a result if the
 * key is accepted.
 *
 * @param appId The application.
 * @param keyCode The key code to check.
 * @return The supplied key_code is accepted by the current app's key set.
 */
bool ApplicationManager::InKeySet(uint16_t appId, uint16_t keyCode)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (m_apps.count(appId) > 0)
    {
        return m_apps[appId]->InKeySet(keyCode);
    }
    LOG(LOG_INFO, "InKeySet(): No app with id %d found. Returning false.", appId);
    return false;
}

/**
 * Process an AIT section. The table will be processed when it is completed or updated.
 *
 * @param aitPid The section PID.
 * @param serviceId The service this section was received for.
 * @param sectionData The section section_data.
 * @param sectionDataBytes The size of section_data in bytes.
 */
void ApplicationManager::ProcessAitSection(uint16_t aitPid, uint16_t serviceId,
    uint8_t *sectionData, uint32_t sectionDataBytes)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    LOG(LOG_INFO, "ProcessAitSection");

    if (serviceId != m_currentService.serviceId)
    {
        LOG(LOG_INFO, "The AIT is not for the current service, early out");
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
        OnSelectedServiceAitReceived();
    }
    else
    {
        OnSelectedServiceAitUpdated();
    }
}

/**
 * Process an XML AIT and create and run a new broadcast-independent application.
 *
 * @param xmlAit The XML AIT contents.
 * @return true if the application can be created, otherwise false
 */
bool ApplicationManager::ProcessXmlAit(const std::string &xmlAit, const bool &isDvbi, const
    std::string &scheme)
{
    const Ait::S_AIT_APP_DESC *app_description;
    bool result = false;

    std::lock_guard<std::recursive_mutex> lock(m_lock);

    LOG(LOG_INFO, "ProcessXmlAit");

    if (xmlAit.empty())
    {
        return false;
    }

    std::unique_ptr<Ait::S_AIT_TABLE> aitTable = XmlParser::ParseAit(xmlAit.c_str(),
        xmlAit.length());
    if (nullptr == aitTable || aitTable->numApps == 0)
    {
        // No AIT or apps parsed, early out
        return false;
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
            OnSelectedServiceAitReceived();
        }
        else
        {
            OnSelectedServiceAitUpdated();
        }
        result = true;
    }
    else
    {
        app_description = GetAutoStartApp(aitTable.get());

        if (app_description)
        {
            result = CreateAndRunApp(*app_description, "", isDvbi, false);
            if (!result)
            {
                LOG(LOG_ERROR, "Could not find app (org_id=%d, app_id=%d)",
                    app_description->orgId,
                    app_description->appId);
            }
        }
    }

    return result;
}

/**
 * Check whether a Teletext application is signalled.
 *
 * @return true if a Teletext application is signalled, otherwise false
 */
bool ApplicationManager::IsTeletextApplicationSignalled()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (m_ait.Get() == nullptr)
    {
        return false;
    }
    return Ait::TeletextApp(m_ait.Get()) != nullptr;
}

/**
 * Run the signalled Teletext application.
 *
 * @return true if the Teletext application can be created, otherwise false
 */
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

/**
 * Check whether a request from the polyfill is allowed.
 *
 * @param callingAppId The app ID making the request.
 * @param callingPageUrl The page URL making the request.
 * @param methodRequirement Any additional requirement of the method.
 * @return true if the request is allowed, otherwise false
 */
bool ApplicationManager::IsRequestAllowed(uint16_t callingAppId, const
    std::string &callingPageUrl,
    MethodRequirement methodRequirement)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    if (m_apps.count(m_appId) <= 0 || m_appId != callingAppId)
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
            return m_apps[m_appId]->IsBroadcast();
        }
        case MethodRequirement::FOR_BROADCAST_OR_TRANSITIONING_APP_ONLY:
        {
            return !Utils::IsInvalidDvbTriplet(m_currentService);
        }
        case MethodRequirement::FOR_TRUSTED_APP_ONLY:
        {
            // Check document URL is inside app boundaries
            if (!Utils::CheckBoundaries(callingPageUrl, m_apps[m_appId]->GetEntryUrl(), m_apps[m_appId]->GetAitDescription().boundaries))
            {
                return false;
            }
            return m_apps[m_appId]->IsTrusted();
        }
        default:
        {
            return false;
        }
    }
}

/**
 * Get the names of the current app.
 *
 * @return The current app names as a map of <lang,name> pairs
 */
std::map<std::string, std::string> ApplicationManager::GetCurrentAppNames()
{
    std::map<std::string, std::string> result;
    LOG(LOG_DEBUG, "GetCurrentAppNames");
    if (m_apps.count(m_appId) > 0)
    {
        std::map<uint32_t, std::string> names = m_apps[m_appId]->GetNames();
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

/**
 * Called when broadcast is stopped (for example when v/b object setChannel is called with null).
 *
 * If a broadcast-related application is running, it will transition to broadcast-independent or
 * be killed depending on the signalling.
 */
void ApplicationManager::OnBroadcastStopped()
{
    LOG(LOG_DEBUG, "OnBroadcastStopped");
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    m_currentServiceReceivedFirstAit = false;
    m_currentServiceAitPid = 0;
    m_ait.Clear();
    m_currentService = Utils::MakeInvalidDvbTriplet();
    if (!TransitionRunningAppToBroadcastIndependent())
    {
        LOG(LOG_INFO, "Kill running app (could not transition to broadcast-independent)");
        KillRunningApp();
    }
}

/**
 * Called when the selected broadcast channel is changed (e.g. by the user or by v/b object).
 *
 * Once the first complete AIT is received or times out:
 *
 * If a broadcast-related application is running, it will continue to run or be killed depending
 * on the signalling.
 *
 * If a broadcast-independent application is running, it will transition to broadcast-related or
 * be killed depending on the signalling.
 */
void ApplicationManager::OnChannelChanged(uint16_t originalNetworkId,
    uint16_t transportStreamId, uint16_t serviceId)
{
    LOG(LOG_DEBUG, "OnChannelChanged (current service: %d)", m_currentService.serviceId);
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    m_currentServiceReceivedFirstAit = false;
    m_currentServiceAitPid = 0;
    m_ait.Clear();
    m_aitTimeout.start(std::chrono::milliseconds(Utils::AIT_TIMEOUT));
    m_currentService = {
        .originalNetworkId = originalNetworkId,
        .transportStreamId = transportStreamId,
        .serviceId = serviceId,
    };
}

/**
 * Called when the network availability has changed.
 *
 * @param available true if the network is available, otherwise false
 */
void ApplicationManager::OnNetworkAvailabilityChanged(bool available)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    LOG(LOG_DEBUG, "OnNetworkAvailabilityChanged available=%d", available);
    m_isNetworkAvailable = available;
}

/**
 * Notify the application manager that a call to loadApplication failed.
 *
 * @param appId The application ID of the application that failed to load.
 */
void ApplicationManager::OnLoadApplicationFailed(uint16_t appId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    // TODO If a call to createApplication has failed, set app_ back to old_app_ and send event?

    if (Utils::IsInvalidDvbTriplet(m_currentService))
    {
        LOG(LOG_ERROR,
            "Unhandled condition (failed to load application while broadcast-independent)");
        return;
    }

    if (m_apps.count(appId) <= 0)
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
    KillRunningApp();
    OnPerformBroadcastAutostart();
}

/**
 * Notify the application manager of application page changed, before the new page is
 * loaded. For example, when the user follows a link.
 *
 * @param appId The application ID.
 * @param url The URL of the new page.
 */
void ApplicationManager::OnApplicationPageChanged(uint16_t appId, const std::string &url)
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
            m_sessionCallback->ResetBroadcastPresentation();
        }
    }
}

// Private methods...

/**
 * Called when the AIT for the selected service is received.
 */
void ApplicationManager::OnSelectedServiceAitReceived()
{
    LOG(LOG_INFO, "OnSelectedServiceAitReceived");
    auto ait = m_ait.Get();
    if (ait != nullptr)
    {
        LOG(LOG_INFO, "New service selected and first AIT received");

        if (m_apps.count(m_appId) > 0)
        {
            if (m_apps[m_appId]->IsBroadcast())
            {
                Ait::S_AIT_APP_DESC aitDesc = m_apps[m_appId]->GetAitDescription();
                LOG(LOG_INFO,
                    "OnSelectedServiceAitReceived: Pre-existing broadcast-related app already running");
                if (aitDesc.appDesc.serviceBound)
                {
                    LOG(LOG_INFO, "Kill running app (is service bound)");
                    KillRunningApp();
                }
                else
                {
                    auto signalled = Ait::FindApp(ait, aitDesc.orgId, aitDesc.appId);
                    if (signalled == nullptr)
                    {
                        LOG(LOG_INFO, "Kill running app (is not signalled in the new AIT)");
                        KillRunningApp();
                    }
                    else if (signalled->controlCode == Ait::APP_CTL_KILL)
                    {
                        LOG(LOG_INFO, "Kill running app (signalled with control code KILL)");
                        KillRunningApp();
                    }
                    else if (!Ait::AppHasTransport(signalled, m_apps[m_appId]->GetProtocolId()))
                    {
                        LOG(LOG_INFO,
                            "Kill running app (is not signalled in the new AIT with the same transport protocol)");
                        KillRunningApp();
                    }
                    else if (!UpdateRunningApp(*signalled))
                    {
                        KillRunningApp();
                    }
                }
            }
            else
            {
                LOG(LOG_INFO, "Pre-existing broadcast-independent app already running");
                if (!TransitionRunningAppToBroadcastRelated())
                {
                    LOG(LOG_INFO, "Kill running app (could not transition to broadcast-related)");
                    KillRunningApp();
                }
            }
        }
        if (m_apps.count(m_appId) <= 0)
        {
            OnPerformBroadcastAutostart();
        }
        else
        {
            Ait::S_AIT_APP_DESC aitDesc = m_apps[m_appId]->GetAitDescription();
            auto signalled = Ait::FindApp(ait, aitDesc.orgId, aitDesc.appId);
            if (signalled != nullptr && !UpdateRunningApp(*signalled))
            {
                KillRunningApp();
            }
        }
    }
}

/**
 * Called when the AIT for the selected service is not received after some timeout.
 */
void ApplicationManager::OnSelectedServiceAitTimeout()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    LOG(LOG_INFO, "OnSelectedServiceAitTimeout");
    KillRunningApp();
}

/**
 * Called when the AIT for the selected service is updated.
 */
void ApplicationManager::OnSelectedServiceAitUpdated()
{
    auto ait = m_ait.Get();
    LOG(LOG_INFO, "OnSelectedServiceAitUpdated");
    if (ait == nullptr)
    {
        LOG(LOG_ERROR, "Unexpected condition (AIT updated but is missing)");
        return;
    }

    if (m_apps.count(m_appId) > 0)
    {
        if (!m_apps[m_appId]->IsBroadcast())
        {
            // If the running app is not broadcast-related, we should not be tuned to broadcast
            LOG(LOG_ERROR, "Unexpected condition (AIT updated but app is not broadcast-related)");
            return;
        }

        Ait::S_AIT_APP_DESC aitDesc = m_apps[m_appId]->GetAitDescription();
        LOG(LOG_INFO,
            "OnSelectedServiceAitUpdated: Pre-existing broadcast-related app already running");
        auto signalled = Ait::FindApp(ait, aitDesc.orgId, aitDesc.appId);
        if (signalled == nullptr)
        {
            LOG(LOG_INFO, "Kill running app (is not signalled in the updated AIT)");
            KillRunningApp();
        }
        else if (!Ait::AppHasTransport(signalled, m_apps[m_appId]->GetProtocolId()))
        {
            LOG(LOG_INFO,
                "Kill running app (is not signalled in the updated AIT with the same transport protocol)");
            KillRunningApp();
        }
        else if (signalled->controlCode == Ait::APP_CTL_KILL)
        {
            LOG(LOG_INFO, "Kill running app (signalled has control code KILL)");
            KillRunningApp();
        }
        else if (!UpdateRunningApp(*signalled))
        {
            KillRunningApp();
        }
    }

    if (m_apps.count(m_appId) <= 0)
    {
        OnPerformBroadcastAutostart();
    }
}

/**
 * Called when the running app has exited.
 */
void ApplicationManager::OnRunningAppExited()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    LOG(LOG_ERROR, "OnRunningAppExited");
    if (!Utils::IsInvalidDvbTriplet(m_currentService))
    {
        OnPerformBroadcastAutostart();
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
void ApplicationManager::OnPerformBroadcastAutostart()
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
    auto app_desc = GetAutoStartApp(ait);

    if (app_desc != nullptr)
    {
        LOG(LOG_ERROR, "OnPerformAutostart Start autostart app.");
        CreateAndRunApp(*app_desc, "", true, false);
    }
    else
    {
        LOG(LOG_INFO, "OnPerformAutostart No autostart app found.");
    }
}

/**
 * Create and run an App by url.
 * 
 * @param url The url the of the App. 
 * 
 * @return True on success, false on failure.
 */
bool ApplicationManager::CreateAndRunApp(std::string url)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    try
    {
        RunApp(std::make_unique<HbbTVApp>(url, m_appSessionCallback));
    }
    catch(const std::exception& e)
    {
        LOG(LOG_ERROR, "%s", e.what());
        return false;
    }
    return true;
}

/**
 * Create and run an App by AIT description.
 * 
 * @param desc The AIT description the new App will use to set its initial state.
 * @param urlParams Additional url parameters that will be concatenated with the
 *      loaded url of the new App.
 * @param isBroadcast Is the new App broadcast related?
 * @param isTrusted Is the new App trusted?
 * 
 * @return True on success, false on failure.
 */
bool ApplicationManager::CreateAndRunApp(const Ait::S_AIT_APP_DESC &desc, const std::string &urlParams, bool isBroadcast, bool isTrusted)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    try
    {
        RunApp(std::make_unique<HbbTVApp>(desc,
            m_currentService,
            m_isNetworkAvailable,
            urlParams,
            isBroadcast,
            isTrusted,
            m_appSessionCallback));
    }
    catch(const std::exception& e)
    {
        LOG(LOG_ERROR, "%s", e.what());
        return false;
    }
    return true;
}

/**
 * Run the app.
 *
 * @param app The app to run.
 */
void ApplicationManager::RunApp(std::unique_ptr<HbbTVApp> app)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    m_apps.erase(m_appId);
    if (!app->IsBroadcast() && !Utils::IsInvalidDvbTriplet(m_currentService))
    {
        m_sessionCallback->StopBroadcast();
        m_currentService = Utils::MakeInvalidDvbTriplet();
    }
    
    m_sessionCallback->LoadApplication(app->GetId(), app->GetEntryUrl().c_str(),
        app->GetAitDescription().graphicsConstraints.size(), app->GetAitDescription().graphicsConstraints);

    m_appId = app->GetId();
    m_apps[m_appId] = std::move(app);

    // Call explicitly Show/Hide 
    if (m_apps[m_appId]->GetState() != HbbTVApp::BACKGROUND_STATE)
    {
        m_sessionCallback->ShowApplication();
    }
    else
    {
        m_sessionCallback->HideApplication();
    }
}

/**
 * Update the running app.
 * 
 * @param desc The AIT description the running App will use to update its state.
 * 
 * @return True on success, false on failure.
 */
bool ApplicationManager::UpdateRunningApp(const Ait::S_AIT_APP_DESC &desc)
{
    try
    {
        m_apps[m_appId]->Update(desc, m_isNetworkAvailable);
    }
    catch(const std::exception& e)
    {
        return false;
    }
    return true;
}

/**
 * Kill the running app.
 */   
void ApplicationManager::KillRunningApp()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    m_sessionCallback->HideApplication();
    m_sessionCallback->LoadApplication(INVALID_APP_ID, "about:blank");
    m_appId = INVALID_APP_ID;
}

/**
 * Transition the running app to broadcast related, if conditions permit.
 *
 * @return True on success, false on failure.
 */
bool ApplicationManager::TransitionRunningAppToBroadcastRelated()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    LOG(LOG_INFO, "TransitionRunningAppToBroadcastRelated");
    auto ait = m_ait.Get();
    if (ait == nullptr)
    {
        LOG(LOG_INFO, "Cannot transition to broadcast (no broadcast AIT)");
        return false;
    }
    if (m_apps.count(m_appId) <= 0)
    {
        LOG(LOG_INFO, "Cannot transition to broadcast (no running app)");
        return false;
    }
    Ait::S_AIT_APP_DESC aitDesc = m_apps[m_appId]->GetAitDescription();
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

    if (!UpdateRunningApp(*app))
    {
        return false;
    }
    
    /* Note: what about app.is_trusted, app.parental_ratings, ... */
    return m_apps[m_appId]->TransitionToBroadcastRelated();
}

/**
 * Transition the running app to broadcast-independent, if conditions permit.
 *
 * @return true on success, false on failure.
 */
bool ApplicationManager::TransitionRunningAppToBroadcastIndependent()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (m_apps.count(m_appId) > 0)
    {
        return m_apps[m_appId]->TransitionToBroadcastIndependent();
    }
    return true;
}

/**
 * Whether the app should be trusted or not TODO
 *
 * @param is_broadcast Whether the app is broadcast-related
 * @return True if the app is trusted, false otherwise
 */
bool ApplicationManager::IsAppTrusted(bool)
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
const Ait::S_AIT_APP_DESC * ApplicationManager::GetAutoStartApp(const Ait::S_AIT_TABLE *aitTable)
{
    int index;
    LOG(LOG_ERROR, "GetAutoStartApp");

    /* Note: XML AIt uses the alpha-2 region codes as defined in ISO 3166-1.
     * DVB's parental_rating_descriptor uses the 3-character code as specified in ISO 3166. */
    std::string parentalControlRegion = m_sessionCallback->GetParentalControlRegion();
    std::string parentalControlRegion3 = m_sessionCallback->GetParentalControlRegion3();
    int parentalControlAge = m_sessionCallback->GetParentalControlAge();
    return Ait::AutoStartApp(aitTable, parentalControlAge, parentalControlRegion,
        parentalControlRegion3);
}

/**
 * Provide access to the AIT organization id
 *
 * @return uint32_t the organization id
 */
uint32_t ApplicationManager::GetOrganizationId()
{
    if (m_apps.count(m_appId) > 0)
    {
        LOG(LOG_INFO, "The organization id is %d\n", m_apps[m_appId]->GetAitDescription().orgId);
        return m_apps[m_appId]->GetAitDescription().orgId;
    }
    LOG(LOG_INFO, "Cannot retrieve organization id (no running app). Returning -1.\n");
    return -1;
}
