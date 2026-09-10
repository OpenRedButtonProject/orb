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
#include <algorithm>

#include "ait.h"
#include "log.h"
#include "utils.h"
#include "xml_parser.h"


#define KEY_SET_RED 0x1
#define KEY_SET_GREEN 0x2
#define KEY_SET_YELLOW 0x4
#define KET_SET_BLUE 0x8
#define KEY_SET_NAVIGATION 0x10
#define KEY_SET_VCR 0x20
#define KEY_SET_SCROLL 0x40
#define KEY_SET_INFO 0x80
#define KEY_SET_NUMERIC 0x100
#define KEY_SET_ALPHA 0x200
#define KEY_SET_OTHER 0x400

/* HbbTV Annex O.3: the number of times a linked application is re-started
 * after being terminated may be limited but shall be greater than one.
 * kMaxLinkedAppRestarts counts restarts of an app that had already presented.
 * Pre-init deaths (restart into an exhausted heap) use the attempt cap. */
static const int kMaxLinkedAppRestarts = 2;
static const int kMaxLinkedAppRestartAttempts = 5;

#define VK_RED 403
#define VK_GREEN 404
#define VK_YELLOW 405
#define VK_BLUE 406
#define VK_UP 38
#define VK_DOWN 40
#define VK_LEFT 37
#define VK_RIGHT 39
#define VK_ENTER 13
#define VK_BACK 461
#define VK_PLAY 415
#define VK_STOP 413
#define VK_PAUSE 19
#define VK_FAST_FWD 417
#define VK_REWIND 412
#define VK_NEXT 425
#define VK_PREV 424
#define VK_PLAY_PAUSE 402
#define VK_RECORD 416
#define VK_PAGE_UP 33
#define VK_PAGE_DOWN 34
#define VK_INFO 457
#define VK_NUMERIC_START 48
#define VK_NUMERIC_END 57
#define VK_ALPHA_START 65
#define VK_ALPHA_END 90

static bool IsKeyNavigation(uint16_t code);
static bool IsKeyNumeric(uint16_t code);
static bool IsKeyAlpha(uint16_t code);
static bool IsKeyVcr(uint16_t code);
static bool IsKeyScroll(uint16_t code);

/**
 * Application manager
 *
 * @param sessionCallback Implementation of ApplicationManager::SessionCallback interface.
 */
ApplicationManager::ApplicationManager(std::unique_ptr<SessionCallback> sessionCallback) :
    m_sessionCallback(std::move(sessionCallback)),
    m_nextAppId(0),
    m_aitTimeout([&] {
    OnSelectedServiceAitTimeout();
}, std::chrono::milliseconds(Utils::AIT_TIMEOUT))
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
    if (callingAppId != INVALID_APP_ID)
    {
        if (!m_app.isRunning || m_app.id != callingAppId)
        {
            LOG(LOG_INFO, "Called by non-running app, early out");
            return false;
        }
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
            if (appDescription && Ait::HasViableTransport(appDescription, m_isNetworkAvailable))
            {
                auto new_app = App::CreateAppFromAitDesc(appDescription, m_currentService,
                    info.parameters, true, false);
                result = RunApp(new_app);
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
                result = RunApp(App::CreateAppFromUrl(url));
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
 * @param callingAppId The calling app ID. 0 is EXIT (restart on this instance);
 *        a matching running id is Application.destroyApplication().
 */
void ApplicationManager::DestroyApplication(uint16_t callingAppId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    LOG(LOG_ERROR, "DestroyApplication");
    if (callingAppId == INVALID_APP_ID)
    {
        // EXIT / comparable key: keep the current service instance and allow
        // autostart to restart the application (HbbTV O.3 / errata #13697).
        KillRunningApp();
        OnRunningAppExited();
        return;
    }
    if (!m_app.isRunning || m_app.id != callingAppId)
    {
        LOG(LOG_INFO, "Called by non-running app, early out");
        return;
    }

    const std::string scheme = m_app.getScheme();
    KillRunningApp();
    // Application.destroyApplication() of a type 1.2 linked app: do not
    // restart this XML AIT. The DVB-I client discards the instance and
    // selects another (TS 103 770 §5.2.13 / errata #13697).
    if (scheme == LINKED_APP_SCHEME_1_2)
    {
        LOG(LOG_INFO, "LA 1.2 destroyApplication(); skip AIT autostart (5.2.13 instance discard)");
        return;
    }
    OnRunningAppExited();
}

/**
 * Kill the running application without starting the broadcast autostart app.
 * HbbTV O.3: a parental-blocked LA 1.2 is killed or frozen; autostart of the
 * previous service AIT would restart the blocked-over app and then skip the
 * PRESENT AIT after PIN (ERRATA0120).
 */
void ApplicationManager::KillForParentalControl()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    LOG(LOG_INFO, "KillForParentalControl: kill without AIT autostart (HbbTV O.3)");
    KillRunningApp();
}

/**
 * Show the calling application.
 *
 * @param callingAppId The calling app ID.
 */
void ApplicationManager::ShowApplication(uint16_t callingAppId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (m_app.id == callingAppId)
    {
        m_app.isHidden = false;
        if (m_app.isRunning)
        {
            m_sessionCallback->ShowApplication();
        }
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
    if (m_app.id == callingAppId)
    {
        m_app.isHidden = true;
        if (m_app.isRunning)
        {
            m_sessionCallback->HideApplication();
        }
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
uint16_t ApplicationManager::SetKeySetMask(uint16_t appId, uint16_t keySetMask, std::vector<uint16_t> otherKeys) {
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    if (m_app.id != appId) {
        return 0;
    }

    std::string currentScheme = m_app.getScheme();

    // Compatibility check for older versions
    bool isOldVersion = m_app.versionMinor > 1;
    bool isLinkedAppScheme12 = currentScheme == LINKED_APP_SCHEME_1_2;

    // Key events VK_STOP, VK_PLAY, VK_PAUSE, VK_PLAY_PAUSE, VK_FAST_FWD,
    // VK_REWIND and VK_RECORD shall always be available to linked applications
    // that are controlling media presentation without requiring the application
    // to be activated first (2.0.4, App. O.7)
    bool isException = isLinkedAppScheme12 && m_app.versionMinor == 7;

    if (!m_app.isActivated && currentScheme != LINKED_APP_SCHEME_2) {
        if ((keySetMask & KEY_SET_VCR) != 0 && isOldVersion && !isException) {
            keySetMask &= ~KEY_SET_VCR;
        }
        if ((keySetMask & KEY_SET_NUMERIC) != 0 && !isLinkedAppScheme12 && isOldVersion) {
            keySetMask &= ~KEY_SET_NUMERIC;
        }
        if ((keySetMask & KEY_SET_OTHER) != 0 && !isLinkedAppScheme12 && isOldVersion) {
            keySetMask &= ~KEY_SET_OTHER;
        }
    }

    m_app.keySetMask = keySetMask;
    if ((keySetMask & KEY_SET_OTHER) != 0) {
        m_app.otherKeys = otherKeys; // Survived all checks
    }

    return keySetMask;
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
    if (m_app.id == appId)
    {
        return m_app.keySetMask;
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
    if (m_app.id == appId)
    {
        return m_app.otherKeys;
    }
    return std::vector<uint16_t>();
}

std::string ApplicationManager::GetApplicationScheme(uint16_t appId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (m_app.id == appId)
    {
        return m_app.getScheme();
    }
    return LINKED_APP_SCHEME_1_1;
}

std::string ApplicationManager::GetApplicationHowRelatedHref(uint16_t appId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (m_app.isRunning && m_app.id == appId)
    {
        return m_app.getHowRelatedHref();
    }
    return "";
}

void ApplicationManager::SetApplicationHowRelatedHref(const std::string &href)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    DBGLOG("SetApplicationHowRelatedHref href=%s (app running=%d id=%u)",
        href.c_str(), m_app.isRunning, m_app.id);
    m_app.setHowRelatedHref(href);
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

    if (m_app.id != appId)
    {
        return false;
    }

    bool result = false;
    if ((m_app.keySetMask & GetKeySet(keyCode)) != 0)
    {
        result = true;
    }
    else if ((m_app.keySetMask & KEY_SET_OTHER) == KEY_SET_OTHER) {
        auto it = std::find(m_app.otherKeys.begin(), m_app.otherKeys.end(), keyCode);
        if (it != m_app.otherKeys.end()) {
            result = true;
        }
    }

    if (result) {
        m_app.isActivated = true;
    }

    return result;
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
        LOG(LOG_INFO, "The AIT is not for the current service (ait=%u current=%u), early out",
            serviceId, m_currentService.serviceId);
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

    if (xmlAit.empty())
    {
        ERRLOG("XML AIT is empty, returning false");
        return false;
    }

    std::unique_ptr<Ait::S_AIT_TABLE> aitTable = XmlParser::ParseAit(xmlAit.c_str(),
        xmlAit.length());
    if (nullptr == aitTable || aitTable->numApps == 0)
    {
        ERRLOG("No AIT or apps parsed, early out");
        return false;
    }
    DBGLOG("isDvbi=%d, scheme=%s, xmlAit length=%zu, app isRunning?=%d, orgId=%u, appId=%u, isBroadcast=%d, AIT-apps=%d",
        isDvbi, scheme.c_str(), xmlAit.length(), m_app.isRunning, m_app.orgId, m_app.appId, m_app.isBroadcast, aitTable->numApps);

    for (int index = 0; index != aitTable->numApps; index++)
    {
        aitTable->appArray[index].scheme = scheme;
    }
    Ait::PrintInfo(aitTable.get());

    if (isDvbi)
    {
        DBGLOG("DVB-I service RcvdFirstAit?=%d", m_currentServiceReceivedFirstAit);
        if (m_app.getScheme() == DASH_APP_SIGNALLING_SCHEME && scheme == LINKED_APP_SCHEME_1_1)
        {
            // app from MDP AIT supercedes a service-linked AIT app - see TS-103-770 sec 5.2.3.3
            DBGLOG("Ignore service linked AIT, when running in-band MPD application");
            return true;
        }

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
        /* Availability type 2 is applied in place without XML AIT (A.2.20.6).
         * Do not let a delayed 1.1/1.2 fetch overwrite that current reason. */
        if (m_app.isRunning && m_app.getHowRelatedHref() != LINKED_APP_SCHEME_2)
        {
            m_app.setHowRelatedHref(scheme);
        }
        result = true;
    }
    else
    {
        app_description = GetAutoStartApp(aitTable.get());

        if (app_description)
        {
            auto new_app = App::CreateAppFromAitDesc(app_description, m_currentService,
                "", isDvbi, false);
            result = RunApp(new_app);
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

    auto newApp = App::CreateAppFromAitDesc(appDescription, m_currentService,
        "", true, false);
    return RunApp(newApp);
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

    if (!m_app.isRunning || m_app.id != callingAppId)
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
            return m_app.isBroadcast;
        }
        case MethodRequirement::FOR_BROADCAST_OR_TRANSITIONING_APP_ONLY:
        {
            return !Utils::IsInvalidDvbTriplet(m_currentService);
        }
        case MethodRequirement::FOR_TRUSTED_APP_ONLY:
        {
            // Check document URL is inside app boundaries
            if (!Utils::CheckBoundaries(callingPageUrl, m_app.entryUrl, m_app.boundaries))
            {
                return false;
            }
            return m_app.isTrusted;
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
    std::map<uint32_t, std::string>::iterator it = m_app.names.begin();
    while (it != m_app.names.end())
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
    m_previousService = m_currentService = Utils::MakeInvalidDvbTriplet();
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
 *
 * DVB-I DASH linked XML AIT is one-shot HTTP. CONNECTING must not start the broadcast
 * AIT watchdog. A DVB-I RF instance uses the RF delivery AIT like a classic broadcast service.
 */
void ApplicationManager::OnChannelChanged(uint16_t originalNetworkId,
    uint16_t transportStreamId, uint16_t serviceId, bool isDvbi, bool useBroadcastAit)
{
    DBGLOG("(current serviceId: %u, new serviceId %u, isDvbi=%d, useBroadcastAit=%d)",
        m_currentService.serviceId, serviceId, isDvbi, useBroadcastAit);
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (m_currentService.originalNetworkId == originalNetworkId &&
        m_currentService.transportStreamId == transportStreamId &&
        m_currentService.serviceId == serviceId)
    {
        DBGLOG("Ignoring duplicate CONNECTING for the same service");
        return;
    }
    m_previousService = m_currentService;
    m_currentService = {
        .originalNetworkId = originalNetworkId,
        .transportStreamId = transportStreamId,
        .serviceId = serviceId,
    };
    m_currentServiceReceivedFirstAit = false;
    m_currentServiceAitPid = 0;
    m_linkedAppRestartCount = 0;
    m_linkedAppRestartAttempts = 0;
    m_linkedAppDidStart = false;
    if (isDvbi && !useBroadcastAit)
    {
        // Linked XML AIT is delivered once via Related Material, not on an RF AIT PID.
        // Keep the current AIT so a running 1.1 app survives DASH CONNECTING and setChannel
        // onto a native DASH service that does not re-signal the app (ERRATA0300–0320).
        m_aitTimeout.stop();
        if (m_app.isRunning && m_app.isBroadcast && m_app.isServiceBound)
        {
            LOG(LOG_INFO, "Kill running app (DVB-I service bound, left the service)");
            KillRunningApp();
        }
        else
        {
            LOG(LOG_INFO, "DVB-I channel change: skip AIT timeout (linked XML AIT is one-shot)");
        }
        return;
    }
    if (isDvbi)
    {
        LOG(LOG_INFO, "DVB-I RF instance: start AIT timeout for broadcast AIT (serviceId=%u)",
            serviceId);
    }
    m_ait.Clear();
    m_aitTimeout.start();
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

    if (!m_app.isRunning || m_app.id != appId)
    {
        return;
    }
    std::string scheme = m_app.getScheme();
    auto ait = m_ait.Get();
    if (ait != nullptr && m_app.appId != 0 && m_app.orgId != 0)
    {
        Ait::S_AIT_APP_DESC *app = Ait::FindApp(ait, m_app.orgId, m_app.appId);
        if (app != nullptr)
        {
            Ait::AppSetTransportFailedToLoad(app, m_app.protocolId);
        }
    }
    KillRunningApp();
    // HbbTV O.3 / TS 103 770 §5.2.13: LA 1.2 that cannot start is discarded at the
    // service-instance layer. Do not autostart the same XML AIT (would retry forever).
    if (scheme == LINKED_APP_SCHEME_1_2)
    {
        LOG(LOG_INFO, "LA 1.2 failed to start; skip AIT autostart (O.3 instance discard)");
        return;
    }
    if (Utils::IsInvalidDvbTriplet(m_currentService))
    {
        LOG(LOG_ERROR,
            "Unhandled condition (failed to load application while broadcast-independent)");
        return;
    }
    OnPerformBroadcastAutostart();
}

/**
 * Notify the application manager of an irrecoverable failure in the running
 * application (renderer OOM, process crash, or equivalent).
 *
 * HbbTV Annex O.3: a DVB-I linked application terminated for this reason
 * shall be re-started. Restarts may be limited but shall be greater than one.
 */
bool ApplicationManager::OnApplicationIrrecoverableError(uint16_t appId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    if (!m_app.isRunning || (appId != INVALID_APP_ID && m_app.id != appId))
    {
        return false;
    }

    LOG(LOG_INFO, "ERRATA0800: irrecoverable error appId=%u orgId=%u scheme=%s restarts=%d",
        m_app.id, m_app.orgId, m_app.getScheme().c_str(), m_linkedAppRestartCount);

    if (IsDvbiLinkedApp())
    {
        bool restarted = RestartDvbiLinkedApp();
        return !restarted;
    }

    KillRunningApp();
    return false;
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
    if (m_app.isRunning && m_app.id == appId)
    {
        m_app.loadedUrl = url;
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

void ApplicationManager::OnApplicationPresented(uint16_t appId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (m_app.isRunning && (appId == INVALID_APP_ID || m_app.id == appId))
    {
        m_linkedAppDidStart = true;
        LOG(LOG_INFO, "ERRATA0800: linked app presented id=%u", m_app.id);
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

        if (m_app.isRunning)
        {
            if (m_app.isBroadcast)
            {
                LOG(LOG_INFO,
                    "OnSelectedServiceAitReceived: Pre-existing broadcast-related app already running");
                if (m_app.isServiceBound && !m_sessionCallback->isInstanceInCurrentService(m_previousService))
                {
                    LOG(LOG_INFO, "Kill running app (is service bound)");
                    KillRunningApp();
                }
                else
                {
                    auto signalled = Ait::FindApp(ait, m_app.orgId, m_app.appId);
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
                    else if (!Ait::AppHasTransport(signalled, m_app.protocolId))
                    {
                        LOG(LOG_INFO,
                            "Kill running app (is not signalled in the new AIT with the same transport protocol)");
                        KillRunningApp();
                    }
                    else
                    {
                        m_app.setScheme(signalled->scheme);
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
        if (!m_app.isRunning)
        {
            // DVB-I XML AIT may signal PRESENT (same org/app as the previous
            // AUTOSTART instance). After a parental kill there is no running
            // app; start PRESENT so PIN override can restart LA 1.2 (O.3).
            OnPerformBroadcastAutostart(m_currentServiceAitPid == UINT16_MAX);
        }
        else
        {
            auto signalled = Ait::FindApp(ait, m_app.orgId, m_app.appId);
            if (signalled != nullptr) {
                m_app.setScheme(signalled->scheme);
            }
            m_sessionCallback->DispatchApplicationSchemeUpdatedEvent(m_app.getScheme());
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

    if (m_app.isRunning)
    {
        if (!m_app.isBroadcast)
        {
            // If the running app is not broadcast-related, we should not be tuned to broadcast
            LOG(LOG_ERROR, "Unexpected condition (AIT updated but app is not broadcast-related)");
            return;
        }

        LOG(LOG_INFO,
            "OnSelectedServiceAitUpdated: Pre-existing broadcast-related app already running");
        auto signalled = Ait::FindApp(ait, m_app.orgId, m_app.appId);
        if (signalled == nullptr)
        {
            LOG(LOG_INFO, "OnSelectedServiceAitUpdated: App (orgId=%u, appId=%u) not found in updated AIT - "
                "KILLING running app", m_app.orgId, m_app.appId);
            KillRunningApp();
        }
        else if (!Ait::AppHasTransport(signalled, m_app.protocolId))
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
        else
        {
            DBGLOG("Applic still signalled in updated AIT (ctrl code=%d, old scheme=%s, new scheme=%s) - CONTINUING to run without restart",
                    signalled->controlCode, m_app.getScheme().c_str(), signalled->scheme.c_str());
            m_app.setScheme(signalled->scheme);
        }
    }

    if (!m_app.isRunning)
    {
        DBGLOG(" App not running - calling OnPerformBroadcastAutostart");
        OnPerformBroadcastAutostart(m_currentServiceAitPid == UINT16_MAX);
    }
    else
    {
        m_sessionCallback->DispatchApplicationSchemeUpdatedEvent(m_app.getScheme());
    }
}

/**
 * Called when the running app has exited.
 */
void ApplicationManager::OnRunningAppExited()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    DBGLOG(" App exited - orgId=%u, appId=%u, id=%u, isBroadcast=%d, entryUrl=%s, isRunning=%d",
        m_app.orgId, m_app.appId, m_app.id, m_app.isBroadcast, m_app.entryUrl.c_str(), m_app.isRunning);
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
void ApplicationManager::OnPerformBroadcastAutostart(bool allowPresent)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    LOG(LOG_ERROR, "OnPerformAutostart");

    // Find autostart (or, for DVB-I XML AIT, PRESENT) app_desc
    auto ait = m_ait.Get();
    if (!m_currentServiceReceivedFirstAit || ait == nullptr)
    {
        LOG(LOG_INFO, "OnPerformAutostart No service selected/AIT, early out");
        return;
    }
    auto app_desc = GetAutoStartApp(ait, allowPresent);

    if (app_desc != nullptr)
    {
        LOG(LOG_ERROR, "OnPerformAutostart Start autostart app.");

        auto newApp = App::CreateAppFromAitDesc(app_desc, m_currentService,
            "", true, false);
        if (!RunApp(newApp))
        {
            LOG(LOG_ERROR, "OnPerformAutostart Failed to create autostart app.");
        }
    }
    else
    {
        LOG(LOG_INFO, "OnPerformAutostart No autostart app found.");
    }
}

/**
 * Run the app.
 *
 * @param app The app to run.
 * @return True on success, false on failure.
 */
bool ApplicationManager::RunApp(const App &app)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    DBGLOG(" orgId=%u, appId=%u, isBroadcast=%d, entryUrl=%s, scheme=%s, isHidden=%d",
        app.orgId, app.appId, app.isBroadcast, app.entryUrl.c_str(), app.getScheme().c_str(), app.isHidden);

    if (!app.entryUrl.empty())
    {
        /* Note: XML AIt uses the alpha-2 region codes as defined in ISO 3166-1.
         * DVB's parental_rating_descriptor uses the 3-character code as specified in ISO 3166. */
        std::string parental_control_region = m_sessionCallback->GetParentalControlRegion();
        std::string parental_control_region3 = m_sessionCallback->GetParentalControlRegion3();
        int parental_control_age = m_sessionCallback->GetParentalControlAge();
        //if none of the parental ratings provided in the broadcast AIT or XML AIT are supported
        //by the terminal), the request to launch the application shall fail.
        if (Ait::IsAgeRestricted(app.parentalRatings, parental_control_age,
            parental_control_region, parental_control_region3))
        {
            ERRLOG("%s, Parental Control Age RESTRICTED for %s: only %d content accepted",
                app.loadedUrl.c_str(), parental_control_region.c_str(), parental_control_age);
            return false;
        }

        if (++m_nextAppId == 0)
        {
            ++m_nextAppId;
        }

        // HowRelated is published before AIT/XML AIT (A.2.20.6). RunApp replaces
        // m_app; keep the href so an RF AUTOSTART stays undefined (ERRATA0900).
        std::string howRelated = m_app.getHowRelatedHref();
        m_app = app;
        m_app.id = m_nextAppId;
        m_app.isRunning = true;
        m_app.setHowRelatedHref(howRelated);

        if (m_app.isHidden)
        {
            LOG(LOG_INFO, "RunApp: App is hidden, hiding application");
            m_sessionCallback->HideApplication();
        }

        if (!app.isBroadcast)
        {
            // The app is broadcast-independent (e.g. created from a URL), stop the broadcast if there
            // is a current service.
            if (!Utils::IsInvalidDvbTriplet(m_currentService))
            {
                LOG(LOG_INFO, "RunApp: Broadcast-independent app - stopping broadcast");
                m_sessionCallback->StopBroadcast();
                m_previousService = m_currentService = Utils::MakeInvalidDvbTriplet();
            }
        }

        LOG(LOG_INFO, "RunApp: Loading application - id=%u, entryUrl=%s, graphicsConstraints=%zu",
            m_app.id, m_app.entryUrl.c_str(), m_app.graphicsConstraints.size());
        m_sessionCallback->LoadApplication(m_app.id, m_app.entryUrl.c_str(),
            m_app.graphicsConstraints.size(), m_app.graphicsConstraints);

        if (!m_app.isHidden)
        {
            LOG(LOG_INFO, "RunApp: Showing application");
            m_sessionCallback->ShowApplication();
        }

        LOG(LOG_INFO, "RunApp: App started successfully - id=%u, orgId=%u, appId=%u, isRunning=%d",
            m_app.id, m_app.orgId, m_app.appId, m_app.isRunning);
        return true;
    }
    LOG(LOG_INFO, "RunApp: App entryUrl is empty - FAILING to start");
    return false;
}

/**
 * Kill the running app.
 */
void ApplicationManager::KillRunningApp()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    LOG(LOG_INFO, "KillRunningApp: Killing app - orgId=%u, appId=%u, id=%u, isBroadcast=%d, "
        "entryUrl=%s, loadedUrl=%s",
        m_app.orgId, m_app.appId, m_app.id, m_app.isBroadcast,
        m_app.entryUrl.c_str(), m_app.loadedUrl.c_str());
    m_sessionCallback->HideApplication();
    if (++m_nextAppId == 0)
    {
        ++m_nextAppId;
    }
    m_sessionCallback->LoadApplication(m_nextAppId, "about:blank");
    m_app.isRunning = false;
}

bool ApplicationManager::IsDvbiLinkedApp() const
{
    const std::string scheme = m_app.getScheme();
    if (scheme == LINKED_APP_SCHEME_1_2 || scheme == LINKED_APP_SCHEME_2)
    {
        return true;
    }
    /* Broadcast AIT apps default getScheme() to 1.1; DVB-I XML AIT sets PID to UINT16_MAX. */
    return scheme == LINKED_APP_SCHEME_1_1 && m_currentServiceAitPid == UINT16_MAX;
}

bool ApplicationManager::RestartDvbiLinkedApp()
{
    if (m_linkedAppRestartCount >= kMaxLinkedAppRestarts
        || m_linkedAppRestartAttempts >= kMaxLinkedAppRestartAttempts)
    {
        LOG(LOG_INFO, "ERRATA0800: restart limit reached (started=%d/%d attempts=%d/%d); not re-starting",
            m_linkedAppRestartCount, kMaxLinkedAppRestarts,
            m_linkedAppRestartAttempts, kMaxLinkedAppRestartAttempts);
        KillRunningApp();
        return false;
    }

    ++m_linkedAppRestartAttempts;
    if (m_linkedAppDidStart)
    {
        ++m_linkedAppRestartCount;
    }
    m_linkedAppDidStart = false;
    App snapshot = m_app;
    KillRunningApp();

    auto ait = m_ait.Get();
    const Ait::S_AIT_APP_DESC *app_desc = nullptr;
    if (ait != nullptr)
    {
        app_desc = GetAutoStartApp(ait);
    }
    if (app_desc == nullptr)
    {
        LOG(LOG_INFO, "ERRATA0800: restart linked app from snapshot (started %d/%d attempt %d/%d)",
            m_linkedAppRestartCount, kMaxLinkedAppRestarts,
            m_linkedAppRestartAttempts, kMaxLinkedAppRestartAttempts);
        return RunApp(snapshot);
    }

    auto newApp = App::CreateAppFromAitDesc(app_desc, m_currentService, "", true, false);
    LOG(LOG_INFO, "ERRATA0800: restart linked app from XML AIT (started %d/%d attempt %d/%d)",
        m_linkedAppRestartCount, kMaxLinkedAppRestarts,
        m_linkedAppRestartAttempts, kMaxLinkedAppRestartAttempts);
    return RunApp(newApp);
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
    if (!m_app.isRunning || (m_app.appId == 0) || (m_app.orgId == 0))
    {
        LOG(LOG_INFO, "Cannot transition to broadcast (no running app or app/org id is 0)");
        return false;
    }
    const Ait::S_AIT_APP_DESC *app = Ait::FindApp(ait, m_app.orgId, m_app.appId);
    if (app == nullptr)
    {
        LOG(LOG_INFO, "Cannot transition to broadcast (app is not signalled in the new AIT)");
        return false;
    }
    if (app->controlCode != Ait::APP_CTL_AUTOSTART && app->controlCode != Ait::APP_CTL_PRESENT)
    {
        LOG(LOG_INFO,
            "Cannot transition to broadcast (app is not signalled in the new AIT as AUTOSTART or PRESENT)");
        return false;
    }

    // Try and find entry URL in boundaries
    uint8_t i;
    bool entry_url_in_boundaries = false;
    for (i = 0; i < app->numTransports; i++)
    {
        if (app->transportArray[i].protocolId == AIT_PROTOCOL_HTTP)
        {
            if (Utils::CheckBoundaries(m_app.entryUrl, app->transportArray[i].url.baseUrl,
                app->boundaries))
            {
                entry_url_in_boundaries = true;
                break;
            }
        }
    }
    if (!entry_url_in_boundaries)
    {
        LOG(LOG_INFO, "Cannot transition to broadcast (entry URL is not in boundaries)");
        return false;
    }

    // Try and find loaded URL in boundaries
    bool loadedUrlInBoundaries = false;
    for (i = 0; i < app->numTransports; i++)
    {
        if (app->transportArray[i].protocolId == AIT_PROTOCOL_HTTP)
        {
            if (Utils::CheckBoundaries(m_app.loadedUrl, app->transportArray[i].url.baseUrl,
                app->boundaries))
            {
                loadedUrlInBoundaries = true;
                break;
            }
        }
    }
    if (!loadedUrlInBoundaries)
    {
        LOG(LOG_INFO, "Cannot transition to broadcast (loaded URL is not in boundaries)");
        return false;
    }

    m_app.isBroadcast = true;
    m_app.isServiceBound = app->appDesc.serviceBound;
    /* Note: what about app.is_trusted, app.parental_ratings, ... */
    m_sessionCallback->DispatchTransitionedToBroadcastRelatedEvent();

    return true;
}

/**
 * Transition the running app to broadcast-independent, if conditions permit.
 *
 * @return true on success, false on failure.
 */
bool ApplicationManager::TransitionRunningAppToBroadcastIndependent()
{
    m_app.isBroadcast = false;
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
const Ait::S_AIT_APP_DESC * ApplicationManager::GetAutoStartApp(const Ait::S_AIT_TABLE *aitTable,
    bool allowPresent)
{
    LOG(LOG_ERROR, "GetAutoStartApp");

    /* Note: XML AIt uses the alpha-2 region codes as defined in ISO 3166-1.
     * DVB's parental_rating_descriptor uses the 3-character code as specified in ISO 3166. */
    std::string parentalControlRegion = m_sessionCallback->GetParentalControlRegion();
    std::string parentalControlRegion3 = m_sessionCallback->GetParentalControlRegion3();
    int parentalControlAge = m_sessionCallback->GetParentalControlAge();
    return Ait::AutoStartApp(aitTable, parentalControlAge, parentalControlRegion,
        parentalControlRegion3, m_isNetworkAvailable, allowPresent);
}

/**
 * Return the KeySet a key code belongs to.
 *
 * @param keyCode The key code.
 * @return The key set.
 */
uint16_t ApplicationManager::GetKeySet(const uint16_t keyCode)
{
    if (IsKeyNavigation(keyCode))
    {
        return KEY_SET_NAVIGATION;
    }
    else if (IsKeyNumeric(keyCode))
    {
        return KEY_SET_NUMERIC;
    }
    else if (IsKeyAlpha(keyCode))
    {
        return KEY_SET_ALPHA;
    }
    else if (IsKeyVcr(keyCode))
    {
        return KEY_SET_VCR;
    }
    else if (IsKeyScroll(keyCode))
    {
        return KEY_SET_SCROLL;
    }
    else if (keyCode == VK_RED)
    {
        return KEY_SET_RED;
    }
    else if (keyCode == VK_GREEN)
    {
        return KEY_SET_GREEN;
    }
    else if (keyCode == VK_YELLOW)
    {
        return KEY_SET_YELLOW;
    }
    else if (keyCode == VK_BLUE)
    {
        return KET_SET_BLUE;
    }
    else if (keyCode == VK_INFO)
    {
        return KEY_SET_INFO;
    }

    return 0;
}

/**
 * Provide access to the AIT organization id
 *
 * @return uint32_t the organization id
 */
uint32_t ApplicationManager::GetOrganizationId()
{
    LOG(LOG_INFO, "The organization id is %d\n", m_app.orgId);
    return m_app.orgId;
}

static bool IsKeyNavigation(uint16_t code)
{
    return code == VK_UP ||
           code == VK_DOWN ||
           code == VK_LEFT ||
           code == VK_RIGHT ||
           code == VK_ENTER ||
           code == VK_BACK;
}

static bool IsKeyNumeric(uint16_t code)
{
    return code >= VK_NUMERIC_START && code <= VK_NUMERIC_END;
}

static bool IsKeyAlpha(uint16_t code)
{
    return code >= VK_ALPHA_START && code <= VK_ALPHA_END;
}

static bool IsKeyVcr(uint16_t code)
{
    return code == VK_PLAY ||
           code == VK_STOP ||
           code == VK_PAUSE ||
           code == VK_FAST_FWD ||
           code == VK_REWIND ||
           code == VK_NEXT ||
           code == VK_PREV ||
           code == VK_PLAY_PAUSE;
}

static bool IsKeyScroll(uint16_t code)
{
    return code == VK_PAGE_UP ||
           code == VK_PAGE_DOWN;
}
