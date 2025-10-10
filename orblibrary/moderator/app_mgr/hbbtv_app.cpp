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

namespace orb
{

static std::string getAppSchemeFromUrlParams(const std::string &urlParams);
static std::string getUrlParamsFromAppScheme(const std::string &scheme);

/**
 * Return the KeySet a key code belongs to.
 *
 * @param keyCode The key code.
 * @return The key set.
 */
static uint16_t GetKeySetMaskForKeyCode(const uint16_t &keyCode);

static bool IsKeyNavigation(const uint16_t &code);
static bool IsKeyNumeric(const uint16_t &code);
static bool IsKeyAlpha(const uint16_t &code);
static bool IsKeyVcr(const uint16_t &code);
static bool IsKeyScroll(const uint16_t &code);


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
    // Broadcast-related applications need to call show.
    m_state = isBroadcast ? BaseApp::BACKGROUND_STATE : BaseApp::FOREGROUND_STATE;
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

    if (m_protocolId == AIT_PROTOCOL_HTTP)
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


uint16_t HbbTVApp::SetKeySetMask(uint16_t keySetMask, const std::vector<uint16_t> &otherKeys) {
    std::string currentScheme = GetScheme();

    // Compatibility check for older versions
    bool isOldVersion = m_versionMinor > 1;
    bool isLinkedAppScheme12 = currentScheme == LINKED_APP_SCHEME_1_2;

    // Key events VK_STOP, VK_PLAY, VK_PAUSE, VK_PLAY_PAUSE, VK_FAST_FWD,
    // VK_REWIND and VK_RECORD shall always be available to linked applications
    // that are controlling media presentation without requiring the application
    // to be activated first (2.0.4, App. O.7)
    bool isException = isLinkedAppScheme12 && m_versionMinor == 7;

    if (!m_isActivated && currentScheme != LINKED_APP_SCHEME_2) {
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

    m_keySetMask = keySetMask;
    if ((keySetMask & KEY_SET_OTHER) != 0) {
        m_otherKeys = otherKeys; // Survived all checks
    }

    return keySetMask;
}

bool HbbTVApp::InKeySet(uint16_t keyCode)
{
    if ((m_keySetMask & GetKeySetMaskForKeyCode(keyCode)) != 0)
    {
        if ((m_keySetMask & KEY_SET_OTHER) != 0) {
            auto it = std::find(m_otherKeys.begin(), m_otherKeys.end(), keyCode);
            if (it == m_otherKeys.end()) {
                return false;
            }
        }
        if (!m_isActivated)
        {
            m_isActivated = true;
        }
        return true;
    }
    return false;
}

bool HbbTVApp::SetState(const E_APP_STATE &state)
{
    // HbbTV apps can go only to background or foreground state
    if (state == BACKGROUND_STATE || state == FOREGROUND_STATE)
    {
        if (state != m_state)
        {
            LOG(INFO) << "AppId [" << GetId() << "]; state transition: " << GetState() << " -> " << state;
            m_state = state;
            if (state == BACKGROUND_STATE)
            {
                m_sessionCallback->HideApplication(GetId());
            }
            else
            {
                m_sessionCallback->ShowApplication(GetId());
            }
        }
        return true;
    }
    LOG(INFO) << "Invalid state transition: " << GetState() << " -> " << state;
    return false;
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

/**
 * Return the KeySet a key code belongs to.
 *
 * @param keyCode The key code.
 * @return The key set.
 */
static uint16_t GetKeySetMaskForKeyCode(const uint16_t &keyCode)
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
        return KEY_SET_BLUE;
    }
    else if (keyCode == VK_INFO)
    {
        return KEY_SET_INFO;
    }
    else if (keyCode == VK_RECORD)
    {
        return KEY_SET_OTHER;
    }

    return 0;
}

static bool IsKeyNavigation(const uint16_t &code)
{
    return code == VK_UP ||
           code == VK_DOWN ||
           code == VK_LEFT ||
           code == VK_RIGHT ||
           code == VK_ENTER ||
           code == VK_BACK;
}

static bool IsKeyNumeric(const uint16_t &code)
{
    return code >= VK_NUMERIC_START && code <= VK_NUMERIC_END;
}

static bool IsKeyAlpha(const uint16_t &code)
{
    return code >= VK_ALPHA_START && code <= VK_ALPHA_END;
}

static bool IsKeyVcr(const uint16_t &code)
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

static bool IsKeyScroll(const uint16_t &code)
{
    return code == VK_PAGE_UP ||
           code == VK_PAGE_DOWN;
}

} // namespace orb
