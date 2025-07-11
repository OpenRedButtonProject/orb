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
 */
#pragma once

#include <string>

#include <json/json.h>

#include "Moderator.h"
#include "ComponentBase.hpp"
#include "app_mgr/application_session_callback.h"

namespace orb
{
class ApplicationManager;

class AppMgrInterface : ComponentBase, public ApplicationSessionCallback
{
public:
    // constructor for explicit Application Type
    explicit AppMgrInterface(ApplicationType apptype);

    /**
     * AppMgrInterface request
     *
     * @param method Application Manager method
     * @param token TODO to be replaced by application ID
     * @param params JSON params. TODO add details
     *
     * @return JSON encoded response string
     */
    std::string executeRequest(std::string method, Json::Value token, Json::Value params) override;

    void processAitSection(int32_t aitPid, int32_t serviceId, const std::vector<uint8_t>& section);
    void processXmlAit(const std::vector<uint8_t>& xmlait);

    // ApplicationSessionCallback interface implementation
    void LoadApplication(const int appId, const char *entryUrl) override;
    void LoadApplication(const int appId, const char *entryUrl, int size, const std::vector<uint16_t> graphics) override;
    void ShowApplication(const int appId) override;
    void HideApplication(const int appId) override;
    void StopBroadcast() override;
    void ResetBroadcastPresentation() override;
    void DispatchApplicationLoadErrorEvent() override;
    void DispatchTransitionedToBroadcastRelatedEvent(const int appId) override;
    std::string GetXmlAitContents(const std::string &url) override;
    int GetParentalControlAge() override;
    std::string GetParentalControlRegion() override;
    std::string GetParentalControlRegion3() override;
    void DispatchApplicationSchemeUpdatedEvent(const int appId, const std::string &scheme) override;
    void DispatchOperatorApplicationStateChange(const int appId, const std::string &oldState, const std::string &newState) override;
    void DispatchOperatorApplicationStateChangeCompleted(const int appId, const std::string &oldState, const std::string &newState) override;
    void DispatchOperatorApplicationContextChange(const int appId, const std::string &startupLocation, const std::string &launchLocation = "") override;
    void DispatchOpAppUpdate(const int appId, const std::string &updateEvent) override;
    bool isInstanceInCurrentService(const Utils::S_DVB_TRIPLET &triplet) override;

private:
    ApplicationType mAppType;
    mutable std::mutex mMutex;

    bool IsRequestAllowed(std::string token);

}; // class AppMgrInterface

} // namespace orb
