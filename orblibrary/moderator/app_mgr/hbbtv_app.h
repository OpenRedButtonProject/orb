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
#include "application_session_callback.h"
#include "base_app.h"


namespace orb
{

class HbbTVApp : public BaseApp
{
public:
    /**
     * Create app from url.
     */
    HbbTVApp(const std::string &url, ApplicationSessionCallback *sessionCallback);

    /**
     * Create app from Ait description.
     *
     */
    HbbTVApp(const Utils::S_DVB_TRIPLET currentService,
        bool isBroadcast,
        bool isTrusted,
        ApplicationSessionCallback *sessionCallback);

    HbbTVApp(const HbbTVApp&) = delete;
    HbbTVApp &operator=(const HbbTVApp&) = delete;

    virtual ~HbbTVApp() = default;

    /**
     * Set URL of app from Ait description and URL params
     *
     */
    void SetUrl(const Ait::S_AIT_APP_DESC &desc,
        const std::string &urlParams, bool isNetworkAvailable);

    bool SetState(const E_APP_STATE &state) override;

    int Load() override;

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
     * @return true if success
     */
    bool Update(const Ait::S_AIT_APP_DESC &desc, bool isNetworkAvailable);

    bool TransitionToBroadcastRelated();
    bool TransitionToBroadcastIndependent();

    Utils::S_DVB_TRIPLET GetService() const { return m_service; }
    std::string GetScheme() const override;

    std::string GetEntryUrl() const { return m_entryUrl; }
    std::string GetBaseUrl() const { return m_baseUrl; }
    std::map<uint32_t, std::string> GetNames() const { return m_names; }

    uint16_t GetProtocolId() const { return m_protocolId; }

    bool IsTrusted() const { return m_isTrusted; }
    bool IsBroadcast() const { return m_isBroadcast; }

    uint8_t GetVersionMinor() const { return m_versionMinor; }

    Ait::S_AIT_APP_DESC GetAitDescription() const { return m_aitDesc; }

    uint16_t SetKeySetMask(const uint16_t keySetMask, const std::vector<uint16_t> &otherKeys) override;

    bool InKeySet(const uint16_t keyCode) override;

    /**
     * Return true if the key code is an allowed other key, i.e. VK_RECORD.
     *
     * @param keyCode The key code.
     * @return True if the key code is an allowed other key, false otherwise.
     */
    static bool IsAllowedOtherKey(const uint16_t keyCode);

 private:
    bool IsAllowedByParentalControl(const Ait::S_AIT_APP_DESC &desc) const;

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

    uint8_t m_versionMinor = 0;
};

} // namespace orb

#endif // HBBTV_APP_H
