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

#ifndef HBBTV_APP_H
#define HBBTV_APP_H

#include <vector>
#include <map>
#include <cstdint>

#include "utils.h"
#include "ait.h"

#define INVALID_APP_ID 0

class HbbTVApp
{
public:
    typedef enum
    {
        HBBTV_APP_TYPE,
        OPAPP_TYPE
    } E_APP_TYPE;

    typedef enum
    {
        FOREGROUND_STATE,
        BACKGROUND_STATE,
        TRANSIENT_STATE,
        OVERLAID_FOREGROUND_STATE,
        OVERLAID_TRANSIENT_STATE
    } E_APP_STATE;

    class SessionCallback
    {
public:
        virtual void ShowApplication(uint16_t appId) = 0;
        virtual void HideApplication(uint16_t appId) = 0;
        virtual void DispatchTransitionedToBroadcastRelatedEvent(uint16_t appId) = 0;
        virtual void DispatchApplicationSchemeUpdatedEvent(uint16_t appId, const std::string &scheme) = 0;
        virtual int GetParentalControlAge() = 0;
        virtual std::string GetParentalControlRegion() = 0;
        virtual std::string GetParentalControlRegion3() = 0;
        virtual ~SessionCallback() = default;
    };

    /**
     * Create app from url.
     * 
     * @throws std::runtime_error
     */
    HbbTVApp(const std::string &url, std::shared_ptr<HbbTVApp::SessionCallback> sessionCallback);

    /**
     * Create app from Ait description.
     * 
     * @throws std::runtime_error
     */
    HbbTVApp(const Ait::S_AIT_APP_DESC &desc,
        const Utils::S_DVB_TRIPLET currentService,
        bool isNetworkAvailable,
        const std::string &urlParams,
        bool isBroadcast,
        bool isTrusted,
        std::shared_ptr<HbbTVApp::SessionCallback> sessionCallback);
    
    HbbTVApp(const HbbTVApp&) = delete;
    HbbTVApp &operator=(const HbbTVApp&) = delete;

    virtual ~HbbTVApp() = default;

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
    void Update(const Ait::S_AIT_APP_DESC &desc, bool isNetworkAvailable);

    virtual bool TransitionToBroadcastRelated();
    virtual bool TransitionToBroadcastIndependent();

    Utils::S_DVB_TRIPLET GetService() const { return m_service; }
    std::string GetScheme() const;

    std::string GetEntryUrl() const { return m_entryUrl; }
    std::string GetBaseUrl() const { return m_baseUrl; }
    std::map<uint32_t, std::string> GetNames() const { return m_names; }

    uint16_t GetProtocolId() const { return m_protocolId; }
    
    bool IsTrusted() const { return m_isTrusted; }
    bool IsBroadcast() const { return m_isBroadcast; }

    uint8_t GetVersionMinor() const { return m_versionMinor; }

    Ait::S_AIT_APP_DESC GetAitDescription() const { return m_aitDesc; }

    /**
     * Get the key set mask for an application.
     *
     * @return The key set mask for the application.
     */
    uint16_t GetKeySetMask() const { return m_keySetMask; }

    /**
     * Set the key set mask for an application.
     *
     * @param keySetMask The key set mask.
     * @param otherKeys optional other keys
     * @return The key set mask for the application.
     */
    uint16_t SetKeySetMask(uint16_t keySetMask, const std::vector<uint16_t> &otherKeys);

    /**
     * Check the key code is accepted by the current key mask. Activate the app as a result if the
     * key is accepted.
     *
     * @param appId The application.
     * @param keyCode The key code to check.
     * @return The supplied key_code is accepted by the current app's key set.
     */
    virtual bool InKeySet(uint16_t keyCode);

    virtual E_APP_TYPE GetType() const { return HBBTV_APP_TYPE; }

    E_APP_STATE GetState() const { return m_state; }
    
    /**
     * Set the application state.
     * 
     * @param state The desired state to transition to.
     * @returns true if transitioned successfully to the desired state, false otherwise.
     */
    virtual bool SetState(const E_APP_STATE &state);

    /**
     * Get the other keys for an application.
     *
     * @param appId The application.
     * @return The other keys for the application.
     */
    std::vector<uint16_t> GetOtherKeyValues() const { return m_otherKeys; }

    uint16_t GetId() const { return m_id; }

    std::string loadedUrl;

protected:
    bool IsAllowedByParentalControl(const Ait::S_AIT_APP_DESC &desc) const;

    uint16_t m_keySetMask = 0;
    std::vector<uint16_t> m_otherKeys;

    std::string m_entryUrl;
    std::string m_baseUrl;
    Utils::S_DVB_TRIPLET m_service = Utils::MakeInvalidDvbTriplet();
    uint16_t m_protocolId = 0;

    /* Activated by default. Deactivate if they are AUTOSTARTED */
    bool m_isActivated = true;
    bool m_isTrusted = false;
    bool m_isBroadcast = false;

    Ait::S_AIT_APP_DESC m_aitDesc;
    std::map<uint32_t, std::string> m_names;

    std::string m_scheme;
    uint8_t m_versionMinor = 0;
    E_APP_STATE m_state = FOREGROUND_STATE;

    std::shared_ptr<HbbTVApp::SessionCallback> m_sessionCallback;

private:
    uint16_t m_id;
};

#endif // HBBTV_APP_H
