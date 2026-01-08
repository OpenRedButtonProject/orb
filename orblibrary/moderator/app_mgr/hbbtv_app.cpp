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

#include "hbbtv_app.h"
#include "third_party/orb/logging/include/log.h"
#include "application_manager.h"
#include "OrbConstants.h"

#include <stdexcept>

namespace orb
{

static std::string getAppSchemeFromUrlParams(const std::string &urlParams);
static std::string getUrlParamsFromAppScheme(const std::string &scheme);


// static
bool HbbTVApp::IsAllowedOtherKey(const uint16_t keyCode)
{
    // FREE-308: TS 102 796 v1.71 Annex A Table A.1.
    const uint16_t VK_RECORD = 416;
    return keyCode == VK_RECORD;
}


HbbTVApp::HbbTVApp(const std::string &url, ApplicationSessionCallback *sessionCallback)
    : BaseApp(APP_TYPE_HBBTV, url, sessionCallback),
    m_entryUrl(url),
    m_baseUrl(url)
{
    m_scheme = getAppSchemeFromUrlParams(url);
}

HbbTVApp::HbbTVApp(const Utils::S_DVB_TRIPLET currentService,
    bool isBroadcast,
    bool isTrusted,
    ApplicationSessionCallback *sessionCallback)
        : BaseApp(APP_TYPE_HBBTV, sessionCallback),
        m_service(currentService),
        m_isTrusted(isTrusted),
        m_isBroadcast(isBroadcast),
        m_versionMinor(INT8_MAX)
{
    m_state = m_isBroadcast ? BaseApp::BACKGROUND_STATE : BaseApp::FOREGROUND_STATE;
}

int HbbTVApp::Load() {
    // Load the HbbTV application with graphics constraints
    m_sessionCallback->LoadApplication(
        GetId(),
        GetEntryUrl().c_str(),
        GetAitDescription().graphicsConstraints.size(),
        GetAitDescription().graphicsConstraints,
        [this]() { SetState(BaseApp::FOREGROUND_STATE); });

    return GetId();
}

void HbbTVApp::SetUrl(const Ait::S_AIT_APP_DESC &desc,
    const std::string &urlParams, bool isNetworkAvailable)
{
    m_baseUrl = Ait::ExtractBaseURL(desc, m_service, isNetworkAvailable);
    m_entryUrl = Utils::MergeUrlParams(m_baseUrl, desc.location, urlParams);
    SetLoadedUrl(m_entryUrl);
}

bool HbbTVApp::Update(const Ait::S_AIT_APP_DESC &desc, bool isNetworkAvailable)
{
    if (!IsAllowedByParentalControl(desc))
    {
        LOG(ERROR) << "App with loaded url '" << GetLoadedUrl() << "' is not allowed by Parental Control.";
        return false;
    }
    m_protocolId = Ait::ExtractProtocolId(desc, isNetworkAvailable);
    if (m_protocolId == 0)
    {
        LOG(ERROR) << "No valid protocol ID";
        return false;
    }

    m_aitDesc = desc;

    for (uint8_t i = 0; i < desc.appDesc.appProfiles.size(); i++)
    {
        if (m_versionMinor >= desc.appDesc.appProfiles[i].versionMinor)
        {
            m_versionMinor = desc.appDesc.appProfiles[i].versionMinor;
        }
    }
    m_names.clear();
    for (uint8_t i = 0; i < desc.appName.numLangs; i++)
    {
        m_names[desc.appName.names[i].langCode] = desc.appName.names[i].name;
    }

    /* AUTOSTARTED apps are activated when they receive a key event */
    m_isActivated = desc.controlCode != Ait::APP_CTL_AUTOSTART;
    m_scheme = desc.scheme;
    if (!m_scheme.empty())
    {
        size_t index = desc.scheme.find('?');
        m_scheme = m_scheme.substr(0, index);
        SetLoadedUrl(Utils::MergeUrlParams("", m_entryUrl,
                                             getUrlParamsFromAppScheme(GetScheme())));
        if (index != std::string::npos) {
            std::string llocParams = desc.scheme.substr(index);
            SetLoadedUrl(Utils::MergeUrlParams("", m_entryUrl, llocParams));
        }
    }
    LOG(DEBUG) << "App[" << m_aitDesc.appId
        << "] properties: orgId=" << m_aitDesc.orgId
        << ", controlCode=" << m_aitDesc.controlCode
        << ", protocolId=" << m_protocolId
        << ", baseUrl=" << m_baseUrl
        << ", entryUrl=" << m_entryUrl
        << ", loadedUrl=" << GetLoadedUrl();

    m_sessionCallback->DispatchApplicationSchemeUpdatedEvent(GetId(), m_scheme);
    return true;
}

bool HbbTVApp::TransitionToBroadcastRelated()
{
    if (m_aitDesc.controlCode != Ait::APP_CTL_AUTOSTART && m_aitDesc.controlCode != Ait::APP_CTL_PRESENT)
    {
        LOG(INFO) << "Cannot transition to broadcast (app is not signalled in the new AIT as AUTOSTART or PRESENT)";
        return false;
    }

    if (m_protocolId == Ait::PROTOCOL_HTTP)
    {
        if (!Utils::CheckBoundaries(m_entryUrl, m_baseUrl, m_aitDesc.boundaries))
        {
            LOG(INFO) << "Cannot transition to broadcast (entry URL is not in boundaries)";
            return false;
        }
        if (!Utils::CheckBoundaries(GetLoadedUrl(), m_baseUrl, m_aitDesc.boundaries))
        {
            LOG(INFO) << "Cannot transition to broadcast (loaded URL is not in boundaries)";
            return false;
        }
    }
    else
    {
        LOG(INFO) << "Cannot transition to broadcast (invalid protocol id)";
        return false;
    }

    m_isBroadcast = true;
    m_sessionCallback->DispatchTransitionedToBroadcastRelatedEvent(GetId());
    return true;
}

bool HbbTVApp::TransitionToBroadcastIndependent()
{
    m_isBroadcast = false;
    return true;
}

std::string HbbTVApp::GetScheme() const {
    if (!m_scheme.empty()) {
        return m_scheme;
    }
    return LINKED_APP_SCHEME_1_1;
}


uint16_t HbbTVApp::SetKeySetMask(const uint16_t keySetMask, const std::vector<uint16_t> &otherKeys) {
    std::string currentScheme = GetScheme();

    // Compatibility check for older versions
    bool isOldVersion = m_versionMinor > 1;
    bool isLinkedAppScheme12 = currentScheme == LINKED_APP_SCHEME_1_2;

    uint16_t newKeySetMask = keySetMask;

    // Key events VK_STOP, VK_PLAY, VK_PAUSE, VK_PLAY_PAUSE, VK_FAST_FWD,
    // VK_REWIND and VK_RECORD shall always be available to linked applications
    // that are controlling media presentation without requiring the application
    // to be activated first (2.0.4, App. O.7)
    bool isException = isLinkedAppScheme12 && m_versionMinor == 7;

    if (!m_isActivated && (currentScheme != LINKED_APP_SCHEME_2)) {
        if (((newKeySetMask & KEY_SET_VCR) == KEY_SET_VCR)
                && (isOldVersion && !isException)) {
            newKeySetMask &= ~KEY_SET_VCR;
        }
        if (((newKeySetMask & KEY_SET_NUMERIC) == KEY_SET_NUMERIC)
                && (!isLinkedAppScheme12 && isOldVersion)) {
            newKeySetMask &= ~KEY_SET_NUMERIC;
        }
        if (((newKeySetMask & KEY_SET_OTHER) == KEY_SET_OTHER)
                && (!isLinkedAppScheme12 && isOldVersion)) {
            newKeySetMask &= ~KEY_SET_OTHER;
        }
    }

    m_keySetMask = newKeySetMask;
    if ((newKeySetMask & KEY_SET_OTHER) == KEY_SET_OTHER) {
        m_otherKeys = otherKeys; // Survived all checks
    }

    return newKeySetMask;
}

bool HbbTVApp::InKeySet(const uint16_t keyCode)
{
    bool result = BaseApp::InKeySet(keyCode);
    if (result) {
        // We don't care what state m_isActivated is, just set it.
        m_isActivated = true;
    }

    return result;
}

bool HbbTVApp::SetState(const E_APP_STATE &state)
{
    // HbbTV apps can go only to background or foreground state
    if (state != BACKGROUND_STATE && state != FOREGROUND_STATE) {
        LOG(INFO) << "Invalid state transition: " << GetState() << " -> " << state;
        return false;
    }

    if (state == m_state) {
        return true;
    }

    LOG(INFO) << "AppId [" << GetId() << "]; state transition: " << GetState() << " -> " << state;
    m_state = state;
    if (state == BACKGROUND_STATE) {
        m_sessionCallback->HideApplication(GetId());
    }
    else {
        m_sessionCallback->ShowApplication(GetId());
    }
    return true;
}

bool HbbTVApp::IsAllowedByParentalControl(const Ait::S_AIT_APP_DESC &desc) const
{
    /* Note: XML AIt uses the alpha-2 region codes as defined in ISO 3166-1.
     * DVB's parental_rating_descriptor uses the 3-character code as specified in ISO 3166. */
    std::string parental_control_region = m_sessionCallback->GetParentalControlRegion();
    std::string parental_control_region3 = m_sessionCallback->GetParentalControlRegion3();
    int parental_control_age = m_sessionCallback->GetParentalControlAge();
    //if none of the parental ratings provided in the broadcast AIT or XML AIT are supported
    //by the terminal), the request to launch the application shall fail.
    if (Ait::IsAgeRestricted(desc.parentalRatings, parental_control_age,
        parental_control_region, parental_control_region3))
    {
        LOG(INFO) << GetLoadedUrl() << ", Parental Control Age RESTRICTED for "
            << parental_control_region << ": only " << parental_control_age << " content accepted";
        return false;
    }
    return true;
}

static std::string getAppSchemeFromUrlParams(const std::string &urlParams)
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

static std::string getUrlParamsFromAppScheme(const std::string &scheme)
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

} // namespace orb
