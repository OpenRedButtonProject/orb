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
#include "log.h"

#include <stdexcept>

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

static uint16_t g_id = INVALID_APP_ID;

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

/**
 * Create app from url.
 * 
 * @throws std::runtime_error
 */
HbbTVApp::HbbTVApp(const std::string &url, std::shared_ptr<SessionCallback> sessionCallback)
    : loadedUrl(url), m_entryUrl(url), m_baseUrl(url), m_sessionCallback(sessionCallback), m_id(++g_id)
{
    if (url.empty())
    {
        throw std::runtime_error("[App]: Provided url should not be empty.");
    }
    m_scheme = getAppSchemeFromUrlParams(url);
}

/**
 * Create app from Ait description.
 * 
 * @throws std::runtime_error
 */
HbbTVApp::HbbTVApp(const Ait::S_AIT_APP_DESC &desc,
    const Utils::S_DVB_TRIPLET currentService,
    bool isNetworkAvailable,
    const std::string &urlParams,
    bool isBroadcast,
    bool isTrusted,
    std::shared_ptr<HbbTVApp::SessionCallback> sessionCallback)
        : m_service(currentService),
        m_isTrusted(isTrusted),
        m_isBroadcast(isBroadcast),
        m_versionMinor(INT8_MAX),
        m_sessionCallback(sessionCallback),
        m_id(++g_id)
{
    m_baseUrl = Ait::ExtractBaseURL(desc, m_service, isNetworkAvailable);
    m_entryUrl = Utils::MergeUrlParams(m_baseUrl, desc.location, urlParams);
    loadedUrl = m_entryUrl;
    
    // Broadcast-related applications need to call show.
    m_state = isBroadcast ? BACKGROUND_STATE : FOREGROUND_STATE;

    Update(desc, isNetworkAvailable);
}

/**
 * Updates the app's state. Meant to be called by the ApplicationManager
 * when it receives a new AIT table or when the network availability is 
 * changed.
 * 
 * @param desc The model that the App will use to extract info about its
 *      state.
 * @param isNetworkAvailable The network availability. Will be used to
 *      determine the protocolId.
 * 
 * @throws std::runtime_error
 */
void HbbTVApp::Update(const Ait::S_AIT_APP_DESC &desc, bool isNetworkAvailable)
{   
    if (!IsAllowedByParentalControl(desc))
    {
        throw std::runtime_error("App with loaded url '" + loadedUrl + "' is not allowed by Parental Control.");
    }
    m_protocolId = Ait::ExtractProtocolId(desc, isNetworkAvailable);
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
        loadedUrl = Utils::MergeUrlParams("", loadedUrl,
                                             getUrlParamsFromAppScheme(GetScheme()));
        if (index != std::string::npos) {
            std::string llocParams = desc.scheme.substr(index);
            loadedUrl = Utils::MergeUrlParams("", m_entryUrl, llocParams);
        }
    }
    LOG(LOG_DEBUG, "App[%d] properties: orgId=%d, controlCode=%d, protocolId=%d, baseUrl=%s, entryUrl=%s, loadedUrl=%s",
            m_aitDesc.appId, m_aitDesc.orgId, m_aitDesc.controlCode, m_protocolId, m_baseUrl.c_str(), m_entryUrl.c_str(), loadedUrl.c_str());
    
    m_sessionCallback->DispatchApplicationSchemeUpdatedEvent(GetId(), m_scheme);
}

bool HbbTVApp::TransitionToBroadcastRelated()
{
    if (m_aitDesc.controlCode != Ait::APP_CTL_AUTOSTART && m_aitDesc.controlCode != Ait::APP_CTL_PRESENT)
    {
        LOG(LOG_INFO,
            "Cannot transition to broadcast (app is not signalled in the new AIT as AUTOSTART or PRESENT)");
        return false;
    }

    if (m_protocolId == AIT_PROTOCOL_HTTP)
    {
        if (!Utils::CheckBoundaries(m_entryUrl, m_baseUrl, m_aitDesc.boundaries))
        {
            LOG(LOG_INFO, "Cannot transition to broadcast (entry URL is not in boundaries)");
            return false;
        }
        if (!Utils::CheckBoundaries(loadedUrl, m_baseUrl, m_aitDesc.boundaries))
        {
            LOG(LOG_INFO, "Cannot transition to broadcast (loaded URL is not in boundaries)");
            return false;
        }
    }
    else
    {
        LOG(LOG_INFO, "Cannot transition to broadcast (invalid protocol id)");
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

/**
 * Set the key set mask for an application.
 *
 * @param keySetMask The key set mask.
 * @param otherKeys optional other keys
 * @return The key set mask for the application.
 */
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

/**
 * Check the key code is accepted by the current key mask. Activate the app as a result if the
 * key is accepted.
 *
 * @param appId The application.
 * @param keyCode The key code to check.
 * @return The supplied key_code is accepted by the current app's key set.
 */
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

/**
 * Set the application state.
 * 
 * @param state The desired state to transition to.
 * @returns true if transitioned successfully to the desired state, false otherwise.
 */
bool HbbTVApp::SetState(const E_APP_STATE &state)
{
    // HbbTV apps can go only to background or foreground state
    if (state == BACKGROUND_STATE || state == FOREGROUND_STATE)
    {
        if (state != m_state)
        {
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
    LOG(LOG_INFO, "Invalid state transition: %d -> %d", m_state, state);
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
        LOG(LOG_INFO, "%s, Parental Control Age RESTRICTED for %s: only %d content accepted",
            loadedUrl.c_str(), parental_control_region.c_str(), parental_control_age);
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
        return KET_SET_BLUE;
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
