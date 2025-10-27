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
#include "third_party/orb/logging/include/log.h"
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
    if (apptype >= APP_TYPE_MAX) {
        LOG(ERROR) << "Invalid param: atype=" << apptype << " cb=" << callback;
        return;
    }

    // Store the raw pointer without taking ownership (allow nullptr for cleanup)
    m_sessionCallback[apptype] = callback;
}


int ApplicationManager::CreateApplication(int callingAppId, const std::string &url, bool runAsOpApp)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    int result = BaseApp::INVALID_APP_ID;

    LOG(INFO) << "CreateApplication";

    ApplicationSessionCallback* callback = nullptr;

    auto callingApp = getAppById(callingAppId);

    if (runAsOpApp) {
        /* No support for multiple opapp installations*/
        /* Check if calling app is an opapp or if an opapp is already running */
        if(callingApp || m_opApp)
        {
            LOG(ERROR) << "Called with runAsOpApp=true from other app or an opapp is already running, early out";
            return BaseApp::INVALID_APP_ID;
        }

        callback = m_sessionCallback[APP_TYPE_OPAPP];
        if (callback == nullptr) {
            LOG(ERROR) << "OpApp session callback is NULL";
            return BaseApp::INVALID_APP_ID;
        }
    }
    else /** HbbTV app */ {
        if (callingApp == nullptr)
        {
            LOG(INFO) << "Called by non-running app, early out";
            return BaseApp::INVALID_APP_ID;
        }

        callback = m_sessionCallback[callingApp->GetType()];
        if (callback == nullptr) {
            LOG(ERROR) << "Session callback is NULL";
            return BaseApp::INVALID_APP_ID;
        }
    }

    if (url.empty())
    {
        LOG(INFO) << "Called with empty URL, early out";
        callback->DispatchApplicationLoadErrorEvent();
        return BaseApp::INVALID_APP_ID;
    }

    Utils::CreateLocatorInfo info = Utils::ParseCreateLocatorInfo(url, m_currentService);
    switch (info.type)
    {
        case Utils::CreateLocatorType::AIT_APPLICATION_LOCATOR:
        {
            LOG(INFO) << "Create for AIT_APPLICATION_LOCATOR (url=" << url << ")";
            if (m_ait.Get() == nullptr)
            {
                LOG(INFO) << "No AIT, early out";
                break;
            }
            const Ait::S_AIT_APP_DESC *appDescription = Ait::FindApp(m_ait.Get(), info.orgId, info.appId);
            if (appDescription && Ait::HasViableTransport(appDescription, m_isNetworkAvailable))
            {
                result = CreateAndRunApp(*appDescription, info.parameters, true, false, runAsOpApp);
            }
            else
            {
                LOG(ERROR) << "Could not find app (org_id=" << info.orgId << ", app_id=" << info.appId << ")";
            }

            break;
        }

        case Utils::CreateLocatorType::ENTRY_PAGE_OR_XML_AIT_LOCATOR:
        {
            LOG(INFO) << "Create for ENTRY_PAGE_OR_XML_AIT_LOCATOR (url=" << url << ")";
            std::string contents;

            contents = callback->GetXmlAitContents(url);

            if (!contents.empty())
            {
                LOG(INFO) << "Locator resource is XML AIT";
                result = ProcessXmlAit(contents, false);
            }
            else
            {
                LOG(INFO) << "Locator resource is ENTRY PAGE";
                result = CreateAndRunApp(url, runAsOpApp);
            }
            break;
        }

        case Utils::CreateLocatorType::UNKNOWN_LOCATOR:
        {
            LOG(INFO) << "Do not create for UNKNOWN_LOCATOR (url=" << url << ")";
            result = BaseApp::INVALID_APP_ID;
            break;
        }
    }

    if (result == BaseApp::INVALID_APP_ID)
    {
        callback->DispatchApplicationLoadErrorEvent();
    }
    else
    {
        if (m_sessionCallback[APP_TYPE_OPAPP] != nullptr) {
            LOG(INFO) << "Dispatching ApplicationLoaded event to OpApp";
            m_sessionCallback[APP_TYPE_OPAPP]->DispatchApplicationLoadedEvent(result);
        } else {
            LOG(ERROR) << "OpApp callback is NULL, cannot dispatch ApplicationLoaded event";
        }
    }

    return result;
}

int ApplicationManager::CreateAndRunApp(std::string url, bool runAsOpApp)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    LOG(INFO) << "CreateAndRunApp: URL " << url << " Is OpApp? " << runAsOpApp;

    if (url.empty())
    {
        LOG(ERROR) << "URL is empty";
        return BaseApp::INVALID_APP_ID;
    }

    if (runAsOpApp)
    {
        if (m_sessionCallback[APP_TYPE_OPAPP] == nullptr) {
            LOG(ERROR) << "OpApp session callback is NULL";
            return BaseApp::INVALID_APP_ID;
        }

        return runOpApp(std::make_unique<OpApp>(url, m_sessionCallback[APP_TYPE_OPAPP]));
    }
    else /* HbbTV app */
    {
        if (m_sessionCallback[APP_TYPE_HBBTV] == nullptr) {
            LOG(ERROR) << "HbbTV session callback is NULL";
            return BaseApp::INVALID_APP_ID;
        }

        return runHbbTVApp(std::make_unique<HbbTVApp>(url, m_sessionCallback[APP_TYPE_HBBTV]));
    }
}

int ApplicationManager::CreateAndRunApp(
    const Ait::S_AIT_APP_DESC &desc,
    const std::string &urlParams,
    bool isBroadcast,
    bool isTrusted,
    bool runAsOpApp
){
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    LOG(INFO) << "CreateAndRunApp: Type " << desc.appUsage
        << " AppId [" << desc.appId << "] Is OpApp? " << runAsOpApp;

    // ETSI TS 103 606 V1.2.1 (2024-03) Table 7: XML AIT Profile
    if (runAsOpApp ||
        desc.appUsage == "urn:hbbtv:opapp:privileged:2017" ||
        desc.appUsage == "urn:hbbtv:opapp:opspecific:2017")
    {
        if (m_sessionCallback[APP_TYPE_OPAPP] == nullptr) {
            LOG(ERROR) << "OpApp session callback is NULL";
            return BaseApp::INVALID_APP_ID;
        }
        auto opApp = std::make_unique<OpApp>(m_sessionCallback[APP_TYPE_OPAPP]);
        // opApp->SetUrl(desc, urlParams, m_isNetworkAvailable);
        return runOpApp(std::move(opApp));
    }
    else /* HbbTV app */
    {
        if (m_sessionCallback[APP_TYPE_HBBTV] == nullptr) {
            LOG(ERROR) << "HbbTV session callback is NULL";
            return BaseApp::INVALID_APP_ID;
        }

        auto hbbtvApp = std::make_unique<HbbTVApp>(
            m_currentService, isBroadcast, isTrusted, m_sessionCallback[APP_TYPE_HBBTV]);
        hbbtvApp->SetUrl(desc, urlParams, m_isNetworkAvailable);
        if (!hbbtvApp->Update(desc, m_isNetworkAvailable)) {
            LOG(ERROR) << "Update failed";
            return BaseApp::INVALID_APP_ID;
        }
        return runHbbTVApp(std::move(hbbtvApp));
    }
}

void ApplicationManager::DestroyApplication(int callingAppId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    LOG(ERROR) << "DestroyApplication callingAppId [" << callingAppId << "]";
    if (getAppById(callingAppId) == nullptr)
    {
        LOG(INFO) << "Called by non-running app, early out";
        return;
    }

    killRunningApp(callingAppId);
    onRunningAppExited();
}

void ApplicationManager::ShowApplication(int callingAppId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    BaseApp* app = getAppById(callingAppId);
    if (app)
    {
        LOG(INFO) << "ShowApplication: AppId " << app->GetId() << ", Type " << app->GetType();
        app->SetState(BaseApp::FOREGROUND_STATE);
    }
}

void ApplicationManager::HideApplication(int callingAppId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    BaseApp* app = getAppById(callingAppId);
    if (app)
    {
        LOG(INFO) << "HideApplication: AppId " << app->GetId() << ", Type " << app->GetType();
        app->SetState(BaseApp::BACKGROUND_STATE);
    }
}

uint16_t ApplicationManager::SetKeySetMask(int appId, uint16_t keySetMask, std::vector<uint16_t> otherKeys)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    // Key masks only apply to HbbTV apps
    if (m_hbbtvApp && m_hbbtvApp->GetId() == appId) {
        return m_hbbtvApp->SetKeySetMask(keySetMask, otherKeys);
    }

    LOG(INFO) << "SetKeySetMask(): App with id " << appId << " is not an HbbTV app or not found. Returning 0.";
    return 0;
}

uint16_t ApplicationManager::GetKeySetMask(int appId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    // Key masks only apply to HbbTV apps
    if (m_hbbtvApp && m_hbbtvApp->GetId() == appId) {
        return m_hbbtvApp->GetKeySetMask();
    }

    LOG(INFO) << "GetKeySetMask(): App with id " << appId << " is not an HbbTV app or not found. Returning 0.";
    return 0;
}

std::vector<uint16_t> ApplicationManager::GetOtherKeyValues(int appId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    // Key masks only apply to HbbTV apps
    if (m_hbbtvApp && m_hbbtvApp->GetId() == appId) {
        return m_hbbtvApp->GetOtherKeyValues();
    }

    LOG(INFO) << "GetOtherKeyValues(): App with id " << appId << " is not an HbbTV app or not found. Returning empty vector.";
    return std::vector<uint16_t>();
}

std::string ApplicationManager::GetApplicationScheme(int appId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    BaseApp* app = getAppById(appId);
    if (app)
    {
        return app->GetScheme();
    }
    return "Error: App not found";
}

std::vector<int> ApplicationManager::GetRunningAppIds()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    std::vector<int> ids;
    if (m_hbbtvApp)
    {
        LOG(INFO) << "GetRunningAppIds(): " << m_hbbtvApp->GetId();
        ids.push_back(m_hbbtvApp->GetId());
    }
    if (m_opApp)
    {
        LOG(INFO) << "GetRunningAppIds(): " << m_opApp->GetId();
        ids.push_back(m_opApp->GetId());
    }
    return ids;
}

std::string ApplicationManager::GetApplicationUrl(int appId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    BaseApp* app = getAppById(appId);
    if (app)
    {
        LOG(INFO) << "GetApplicationUrl(" << appId << "): " << app->GetLoadedUrl();
        return app->GetLoadedUrl();
    }
    return std::string();
}

bool ApplicationManager::InKeySet(int appId, uint16_t keyCode)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    // Key masks only apply to HbbTV apps
    if (m_hbbtvApp && m_hbbtvApp->GetId() == appId) {
        return m_hbbtvApp->InKeySet(keyCode);
    }

    LOG(INFO) << "InKeySet(): App with id " << appId << " is not an HbbTV app or not found. Returning false.";
    return false;
}

void ApplicationManager::ProcessAitSection(uint16_t aitPid, uint16_t serviceId,
    const uint8_t *sectionData, uint32_t sectionDataBytes)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    LOG(INFO) << "ProcessAitSection";

    if (serviceId != m_currentService.serviceId)
    {
        LOG(INFO) << "The AIT is for service " << serviceId << ", not current service " << m_currentService.serviceId << ", early out";
        return;
    }

    if (aitPid != m_currentServiceAitPid)
    {
        if (m_currentServiceAitPid != 0)
        {
            LOG(INFO) << "The AIT comes in a different PID, now=" << aitPid << " before= " << m_currentServiceAitPid;
            m_ait.Clear();
        }
        m_currentServiceAitPid = aitPid;
    }

    if (!m_ait.ProcessSection(sectionData, sectionDataBytes))
    {
        LOG(INFO) << "The AIT was not completed and/or updated, early out";
        return;
    }

    const Ait::S_AIT_TABLE *updated_ait = m_ait.Get();
    if (updated_ait == nullptr)
    {
        LOG(ERROR) << "No AIT, early out";
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
    int result = BaseApp::INVALID_APP_ID;

    std::lock_guard<std::recursive_mutex> lock(m_lock);

    LOG(INFO) << "ProcessXmlAit";

    if (xmlAit.empty())
    {
        return BaseApp::INVALID_APP_ID;
    }

    if (m_xmlParser == nullptr)
    {
        LOG(ERROR) << "No XML parser provided";
        return BaseApp::INVALID_APP_ID;
    }

    std::unique_ptr<Ait::S_AIT_TABLE> aitTable = m_xmlParser->ParseAit(xmlAit.c_str(),
        xmlAit.length());
    if (nullptr == aitTable || aitTable->numApps == 0)
    {
        // No AIT or apps parsed, early out
        return BaseApp::INVALID_APP_ID;
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
        result = getCurrentHbbTVAppId();
    }
    else
    {
        app_description = getAutoStartApp(aitTable.get());

        if (app_description)
        {
            result = CreateAndRunApp(*app_description, "", isDvbi, false);
            if (result == BaseApp::INVALID_APP_ID)
            {
                LOG(ERROR) << "Could not find app (org_id="
                    << app_description->orgId << ", app_id=" << app_description->appId << ")";
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

    LOG(INFO) << "RunTeletextApplication";

    if (m_ait.Get() == nullptr)
    {
        return false;
    }
    appDescription = Ait::TeletextApp(m_ait.Get());
    if (appDescription == nullptr)
    {
        LOG(ERROR) << "Could not find Teletext app";

        return false;
    }
    return CreateAndRunApp(*appDescription, "", true, false);
}

bool ApplicationManager::IsRequestAllowed(int callingAppId, const
    std::string &callingPageUrl,
    MethodRequirement methodRequirement)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    if ((m_hbbtvApp == nullptr) || (m_hbbtvApp->GetId() != callingAppId))
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
            return m_hbbtvApp->IsBroadcast();
        }
        case MethodRequirement::FOR_BROADCAST_OR_TRANSITIONING_APP_ONLY:
        {
            return !Utils::IsInvalidDvbTriplet(m_currentService);
        }
        case MethodRequirement::FOR_TRUSTED_APP_ONLY:
        {
            // Check document URL is inside app boundaries
            if (!Utils::CheckBoundaries(callingPageUrl, m_hbbtvApp->GetEntryUrl(), m_hbbtvApp->GetAitDescription().boundaries))
            {
                return false;
            }
            return m_hbbtvApp->IsTrusted();
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
    LOG(DEBUG) << "GetCurrentAppNames";
    if (m_hbbtvApp)
    {
        std::map<uint32_t, std::string> names = m_hbbtvApp->GetNames();
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
            LOG(DEBUG) << "lang=" << langCodeString << " name=" << name;
            it++;
        }
    }
    return result;
}

void ApplicationManager::OnBroadcastStopped()
{
    LOG(DEBUG) << "OnBroadcastStopped";
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    m_currentServiceReceivedFirstAit = false;
    m_currentServiceAitPid = 0;
    m_ait.Clear();
    m_previousService = m_currentService = Utils::MakeInvalidDvbTriplet();
    if (!transitionRunningAppToBroadcastIndependent())
    {
        LOG(INFO) << "Kill running app (could not transition to broadcast-independent)";
        killRunningApp(getCurrentHbbTVAppId());
    }
}

void ApplicationManager::OnChannelChanged(uint16_t originalNetworkId,
    uint16_t transportStreamId, uint16_t serviceId)
{
    LOG(DEBUG) << "OnChannelChanged (current service: " << m_currentService.serviceId << ")";
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
    LOG(DEBUG) << "OnNetworkAvailabilityChanged available=" << available;
    m_isNetworkAvailable = available;
}

void ApplicationManager::OnLoadApplicationFailed(int appId)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    // TODO If a call to createApplication has failed, set app_ back to old_app_ and send event?

    if (Utils::IsInvalidDvbTriplet(m_currentService))
    {
        LOG(ERROR) << "Unhandled condition (failed to load application while broadcast-independent)";
        return;
    }

    BaseApp* app = getAppById(appId);
    if (app == nullptr)
    {
        LOG(ERROR) << "App not found";
        return;
    }

    if (app->GetType() == APP_TYPE_HBBTV)
    {
        HbbTVApp* hbbtvApp = static_cast<HbbTVApp*>(app);
        uint16_t protocolId = hbbtvApp->GetProtocolId();

        auto ait = m_ait.Get();
        if (ait != nullptr)
        {
            Ait::S_AIT_APP_DESC aitDesc = hbbtvApp->GetAitDescription();
            if (aitDesc.appId != 0 && aitDesc.orgId != 0)
            {
                Ait::S_AIT_APP_DESC *aitApp = Ait::FindApp(ait, aitDesc.orgId, aitDesc.appId);
                Ait::AppSetTransportFailedToLoad(aitApp, protocolId);
            }
        }
    }

    killRunningApp(appId);

    if(app->GetType() == APP_TYPE_HBBTV)
    {
        onPerformBroadcastAutostart();
    }
}

void ApplicationManager::OnApplicationPageChanged(int appId, const std::string &url)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    BaseApp* app = getAppById(appId);
    if (app)
    {
        app->SetLoadedUrl(url);
        if (!Utils::IsInvalidDvbTriplet(m_currentService) &&
            url.find("https://www.live.bbctvapps.co.uk/tap/iplayer") == std::string::npos)
        {
            // For broadcast-related applications we reset the broadcast presentation on page change,
            // as dead JS objects may have suspended presentation, set the video rectangle or set
            // the presented components.
            if (app->GetType() == APP_TYPE_HBBTV
                 && m_sessionCallback[APP_TYPE_HBBTV])
            {
                m_sessionCallback[APP_TYPE_HBBTV]->ResetBroadcastPresentation();
            }
        }
    }
}

// Private methods...

void ApplicationManager::onSelectedServiceAitReceived()
{
    LOG(INFO) << "OnSelectedServiceAitReceived";
    auto ait = m_ait.Get();
    if (ait != nullptr)
    {
        LOG(INFO) << "New service selected and first AIT received";

        if (m_hbbtvApp)
        {
            if (m_hbbtvApp->IsBroadcast())
            {
                Ait::S_AIT_APP_DESC aitDesc = m_hbbtvApp->GetAitDescription();
                LOG(INFO) << "OnSelectedServiceAitReceived: Pre-existing broadcast-related app already running";
                if (aitDesc.appDesc.serviceBound
                    && m_sessionCallback[APP_TYPE_HBBTV]
                    && !m_sessionCallback[APP_TYPE_HBBTV]->isInstanceInCurrentService(m_previousService))
                {
                    LOG(INFO) << "Kill running app (is service bound)";
                    killRunningApp(getCurrentHbbTVAppId());
                }
                else
                {
                    auto signalled = Ait::FindApp(ait, aitDesc.orgId, aitDesc.appId);
                    if (signalled == nullptr)
                    {
                        LOG(INFO) << "Kill running app (is not signalled in the new AIT)";
                        killRunningApp(getCurrentHbbTVAppId());
                    }
                    else if (signalled->controlCode == Ait::APP_CTL_KILL)
                    {
                        LOG(INFO) << "Kill running app (signalled with control code KILL)";
                        killRunningApp(getCurrentHbbTVAppId());
                    }
                    else if (!Ait::AppHasTransport(signalled, m_hbbtvApp->GetProtocolId()))
                    {
                        LOG(INFO) <<
                            "Kill running app (is not signalled in the new AIT with the same transport protocol)";
                        killRunningApp(getCurrentHbbTVAppId());
                    }
                    else if (!updateRunningApp(*signalled))
                    {
                        killRunningApp(getCurrentHbbTVAppId());
                    }
                }
            }
            else
            {
                LOG(INFO) << "Pre-existing broadcast-independent app already running";
                if (!transitionRunningAppToBroadcastRelated())
                {
                    LOG(INFO) << "Kill running app (could not transition to broadcast-related)";
                    killRunningApp(getCurrentHbbTVAppId());
                }
            }
        }
        if (!isHbbTVAppRunning())
        {
            onPerformBroadcastAutostart();
        }
        else
        {
            Ait::S_AIT_APP_DESC aitDesc = m_hbbtvApp->GetAitDescription();
            auto signalled = Ait::FindApp(ait, aitDesc.orgId, aitDesc.appId);
            if (signalled != nullptr && !updateRunningApp(*signalled))
            {
                killRunningApp(getCurrentHbbTVAppId());
            }
        }
    }
}

void ApplicationManager::onSelectedServiceAitTimeout()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    LOG(INFO) << "OnSelectedServiceAitTimeout";
    killRunningApp(getCurrentHbbTVAppId());
}

void ApplicationManager::onSelectedServiceAitUpdated()
{
    auto ait = m_ait.Get();
    LOG(INFO) << "OnSelectedServiceAitUpdated";
    if (ait == nullptr)
    {
        LOG(ERROR) << "Unexpected condition (AIT updated but is missing)";
        return;
    }

    if (m_hbbtvApp)
    {
        if (!m_hbbtvApp->IsBroadcast())
        {
            // If the running app is not broadcast-related, we should not be tuned to broadcast
            LOG(ERROR) << "Unexpected condition (AIT updated but app is not broadcast-related)";
            return;
        }

        Ait::S_AIT_APP_DESC aitDesc = m_hbbtvApp->GetAitDescription();
        LOG(INFO) << "OnSelectedServiceAitUpdated: Pre-existing broadcast-related app already running";
        auto signalled = Ait::FindApp(ait, aitDesc.orgId, aitDesc.appId);
        if (signalled == nullptr)
        {
            LOG(INFO) << "Kill running app (is not signalled in the updated AIT)";
            killRunningApp(getCurrentHbbTVAppId());
        }
        else if (!Ait::AppHasTransport(signalled, m_hbbtvApp->GetProtocolId()))
        {
            LOG(INFO) << "Kill running app (is not signalled in the updated AIT with the same transport protocol)";
            killRunningApp(getCurrentHbbTVAppId());
        }
        else if (signalled->controlCode == Ait::APP_CTL_KILL)
        {
            LOG(INFO) << "Kill running app (signalled has control code KILL)";
            killRunningApp(getCurrentHbbTVAppId());
        }
        else if (!updateRunningApp(*signalled))
        {
            killRunningApp(getCurrentHbbTVAppId());
        }
    }

    if (!isHbbTVAppRunning())
    {
        onPerformBroadcastAutostart();
    }
}

void ApplicationManager::onRunningAppExited()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    LOG(ERROR) << "OnRunningAppExited";
    if (!Utils::IsInvalidDvbTriplet(m_currentService))
    {
        onPerformBroadcastAutostart();
    }
    else
    {
        // TODO This behaviour is implementation specific
        LOG(ERROR) << "Unhandled condition (broadcast-independent app exited)";
    }
}

/**
 * Called at a time when the broadcast autostart app should be started.
 */
void ApplicationManager::onPerformBroadcastAutostart()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    LOG(ERROR) << "OnPerformAutostart";

    // Find autostart app_desc
    auto ait = m_ait.Get();
    if (!m_currentServiceReceivedFirstAit || ait == nullptr)
    {
        LOG(INFO) << "OnPerformAutostart No service selected/AIT, early out";
        return;
    }
    auto app_desc = getAutoStartApp(ait);

    if (app_desc != nullptr)
    {
        LOG(ERROR) << "OnPerformAutostart Start autostart app.";
        CreateAndRunApp(*app_desc, "", true, false);
    }
    else
    {
        LOG(INFO) << "OnPerformAutostart No viable autostart app found. isNetworkAvailable? " << m_isNetworkAvailable;
    }
}

int ApplicationManager::runOpApp(std::unique_ptr<OpApp> app)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    // Clear existing OpApp
    m_opApp.reset();

    // Extract callback to avoid repeated checks
    auto callback = m_sessionCallback[APP_TYPE_OPAPP];
    if (callback == nullptr) {
        LOG(ERROR) << "OpApp session callback is NULL";
        return BaseApp::INVALID_APP_ID;
    }

    m_opApp = std::move(app);

    int id = m_opApp->GetId();
    callback->LoadApplication(id, m_opApp->GetLoadedUrl().c_str());

    if (m_opApp->GetState() == BaseApp::BACKGROUND_STATE)
    {
        LOG(INFO) << "HideApplication: Type " << APP_TYPE_OPAPP << " AppId [" << id << "]";
        callback->HideApplication(id);
    }
    else
    {
        LOG(INFO) << "ShowApplication: Type " << APP_TYPE_OPAPP << " AppId [" << id << "]";
        callback->ShowApplication(id);
    }

    return m_opApp->GetId();
}

int ApplicationManager::runHbbTVApp(std::unique_ptr<HbbTVApp> app)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    // Clear existing HbbTV app
    m_hbbtvApp.reset();

    // Extract callback to avoid repeated checks
    auto callback = m_sessionCallback[APP_TYPE_HBBTV];
    if (callback == nullptr) {
        LOG(ERROR) << "HbbTV session callback is NULL";
        return BaseApp::INVALID_APP_ID;
    }

    // Handle broadcast service cleanup if needed
    if (!app->IsBroadcast() && !Utils::IsInvalidDvbTriplet(m_currentService))
    {
        callback->StopBroadcast();
        m_previousService = m_currentService = Utils::MakeInvalidDvbTriplet();
    }

    // Store the HbbTV app
    m_hbbtvApp = std::move(app);

    int id = m_hbbtvApp->GetId();
    // Load the HbbTV application with graphics constraints
    callback->LoadApplication(
        id,
        m_hbbtvApp->GetEntryUrl().c_str(),
        m_hbbtvApp->GetAitDescription().graphicsConstraints.size(),
        m_hbbtvApp->GetAitDescription().graphicsConstraints);

    if (m_hbbtvApp->GetState() == BaseApp::BACKGROUND_STATE)
    {
        LOG(INFO) << "HideApplication: Type " << APP_TYPE_HBBTV << " AppId [" << id << "]";
        callback->HideApplication(id);
    }
    else
    {
        LOG(INFO) << "ShowApplication: Type " << APP_TYPE_HBBTV << " AppId [" << id << "]";
        callback->ShowApplication(id);
    }

    return m_hbbtvApp->GetId();
}

bool ApplicationManager::updateRunningApp(const Ait::S_AIT_APP_DESC &desc)
{
    if (m_hbbtvApp) {
        return m_hbbtvApp->Update(desc, m_isNetworkAvailable);
    }
    return false;
}

void ApplicationManager::killRunningApp(int appid)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    BaseApp* app = getAppById(appid);
    if (app == nullptr) {
        LOG(ERROR) << "App is NULL";
        return;
    }

    auto type = app->GetType();

    // Extract callback to avoid repeated checks
    auto callback = m_sessionCallback[type];
    if (callback == nullptr) {
        LOG(ERROR) << "Session callback is NULL";
        return;
    }

    LOG(INFO) << "killRunningApp: Type " << type << " AppId [" << appid << "]";
    if (m_sessionCallback[APP_TYPE_OPAPP] != nullptr) {
        LOG(INFO) << "Dispatching ApplicationUnloaded event to OpApp";
        m_sessionCallback[APP_TYPE_OPAPP]->DispatchApplicationUnloadedEvent(appid);
    } else {
        LOG(ERROR) << "OpApp callback is NULL, cannot dispatch ApplicationUnloaded event";
    }

    callback->HideApplication(appid);
    callback->LoadApplication(BaseApp::INVALID_APP_ID, "about:blank");

    if (type == APP_TYPE_HBBTV)
    {   
        m_hbbtvApp.reset();
    }
    else /* if (type == APP_TYPE_OPAPP) */
    {
        m_opApp.reset();
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
    LOG(INFO) << "TransitionRunningAppToBroadcastRelated";
    auto ait = m_ait.Get();
    if (ait == nullptr)
    {
        LOG(INFO) << "Cannot transition to broadcast (no broadcast AIT)";
        return false;
    }
    if (!isHbbTVAppRunning())
    {
        LOG(INFO) << "Cannot transition to broadcast (no running app)";
        return false;
    }
    Ait::S_AIT_APP_DESC aitDesc = m_hbbtvApp->GetAitDescription();
    if (aitDesc.appId == 0 || aitDesc.orgId == 0)
    {
        LOG(INFO) << "Cannot transition to broadcast (app/org id is 0)";
        return false;
    }
    // get the latest ait description from the ait table
    const Ait::S_AIT_APP_DESC *app = Ait::FindApp(ait, aitDesc.orgId, aitDesc.appId);
    if (app == nullptr)
    {
        LOG(INFO) << "Cannot transition to broadcast (app is not signalled in the new AIT)";
        return false;
    }

    if (!updateRunningApp(*app))
    {
        return false;
    }

    /* Note: what about app.is_trusted, app.parental_ratings, ... */
    return m_hbbtvApp->TransitionToBroadcastRelated();
}

/**
 * Transition the running app to broadcast-independent, if conditions permit.
 *
 * @return true on success, false on failure.
 */
bool ApplicationManager::transitionRunningAppToBroadcastIndependent()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (m_hbbtvApp)
    {
        return m_hbbtvApp->TransitionToBroadcastIndependent();
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
    LOG(ERROR) << "GetAutoStartApp";

    /* Note: XML AIt uses the alpha-2 region codes as defined in ISO 3166-1.
     * DVB's parental_rating_descriptor uses the 3-character code as specified in ISO 3166. */
    std::string parentalControlRegion;
    std::string parentalControlRegion3;
    int parentalControlAge = 0;
    auto callback = m_sessionCallback[APP_TYPE_HBBTV];
    if (callback) {
        parentalControlRegion = callback->GetParentalControlRegion();
        parentalControlRegion3 = callback->GetParentalControlRegion3();
        parentalControlAge = callback->GetParentalControlAge();
    }
    return Ait::AutoStartApp(aitTable, parentalControlAge, parentalControlRegion,
        parentalControlRegion3, m_isNetworkAvailable);
}

uint32_t ApplicationManager::GetOrganizationId()
{
    if (m_hbbtvApp)
    {
        LOG(INFO) << "The organization id is " << m_hbbtvApp->GetAitDescription().orgId;
        return m_hbbtvApp->GetAitDescription().orgId;
    }
    LOG(INFO) << "Cannot retrieve organization id (no running app). Returning -1.";
    return -1;
}

// Helper methods for accessing apps with the new pointer structure
BaseApp* ApplicationManager::getAppById(int appId)
{
    if (m_hbbtvApp && m_hbbtvApp->GetId() == appId) {
        return m_hbbtvApp.get();
    }
    if (m_opApp && m_opApp->GetId() == appId) {
        return m_opApp.get();
    }
    return nullptr;
}

bool ApplicationManager::isHbbTVAppRunning() const
{
    return m_hbbtvApp != nullptr;
}

bool ApplicationManager::isOpAppRunning() const
{
    return m_opApp != nullptr;
}

int ApplicationManager::getCurrentHbbTVAppId() const
{
    return m_hbbtvApp ? m_hbbtvApp->GetId() : BaseApp::INVALID_APP_ID;
}

int ApplicationManager::getCurrentOpAppId() const
{
    return m_opApp ? m_opApp->GetId() : BaseApp::INVALID_APP_ID;
}

} // namespace orb
