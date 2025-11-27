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
 * Mock AppMgrInterface for testing using Google Mock
 */

#ifndef MOCK_APP_MGR_INTERFACE_H
#define MOCK_APP_MGR_INTERFACE_H

#include "AppMgrInterface.hpp"
#include "app_mgr/utils.h"
#include <gmock/gmock.h>

namespace orb
{

class MockAppMgrInterface : public IAppMgrInterface
{
public:
    // Constructor - we need to provide a constructor that matches the base class
    MockAppMgrInterface(IOrbBrowser* browser, ApplicationType apptype) : IAppMgrInterface() {}

    virtual ~MockAppMgrInterface() = default;

    // ComponentBase interface using Google Mock
    MOCK_METHOD(std::string, executeRequest, (const std::string& method, const std::string& token, const IJson& params), (override));

    MOCK_METHOD(ApplicationType, GetApplicationType, (), (const, override));

    // AppMgrInterface specific methods using Google Mock
    MOCK_METHOD(void, onNetworkStatusChange, (bool available), (override));
    MOCK_METHOD(void, onChannelChange, (uint16_t onetId, uint16_t transId, uint16_t serviceId), (override));
    MOCK_METHOD(void, processAitSection, (int32_t aitPid, int32_t serviceId, const std::vector<uint8_t>& section), (override));
    MOCK_METHOD(void, processXmlAit, (const std::vector<uint8_t>& xmlait), (override));

    // ApplicationSessionCallback interface using Google Mock
    MOCK_METHOD(void, LoadApplication, (const int appId, const char *entryUrl, onPageLoadedSuccess callback), (override));
    MOCK_METHOD(void, LoadApplication, (const int appId, const char *entryUrl, int size, const std::vector<uint16_t> graphics, onPageLoadedSuccess callback), (override));
    MOCK_METHOD(void, ShowApplication, (const int appId), (override));
    MOCK_METHOD(void, HideApplication, (const int appId), (override));
    MOCK_METHOD(void, StopBroadcast, (), (override));
    MOCK_METHOD(void, ResetBroadcastPresentation, (), (override));
    MOCK_METHOD(void, DispatchApplicationLoadErrorEvent, (), (override));
    MOCK_METHOD(void, DispatchApplicationLoadedEvent, (const int appId), (override));
    MOCK_METHOD(void, DispatchApplicationUnloadedEvent, (const int appId), (override));
    MOCK_METHOD(void, DispatchTransitionedToBroadcastRelatedEvent, (const int appId), (override));
    MOCK_METHOD(std::string, GetXmlAitContents, (const std::string &url), (override));
    MOCK_METHOD(int, GetParentalControlAge, (), (override));
    MOCK_METHOD(std::string, GetParentalControlRegion, (), (override));
    MOCK_METHOD(std::string, GetParentalControlRegion3, (), (override));
    MOCK_METHOD(void, DispatchApplicationSchemeUpdatedEvent, (const int appId, const std::string &scheme), (override));
    MOCK_METHOD(void, DispatchOperatorApplicationStateChange, (const int appId, const std::string &oldState, const std::string &newState), (override));
    MOCK_METHOD(void, DispatchOperatorApplicationStateChangeCompleted, (const int appId, const std::string &oldState, const std::string &newState), (override));
    MOCK_METHOD(void, DispatchOperatorApplicationContextChange, (const int appId, const std::string &startupLocation, const std::string &launchLocation), (override));
    MOCK_METHOD(void, DispatchOpAppUpdate, (const int appId, const std::string &updateEvent), (override));
    MOCK_METHOD(bool, isInstanceInCurrentService, (const Utils::S_DVB_TRIPLET &triplet), (override));
    MOCK_METHOD(bool, InKeySet, (const uint16_t keyCode), (override));
};

} // namespace orb

#endif // MOCK_APP_MGR_INTERFACE_H
