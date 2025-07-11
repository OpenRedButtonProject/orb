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
 * ORB Application Manager
 *
 */

#include <sys/sysinfo.h>

#include "AppManager.hpp"
#include "app_mgr/application_manager.h"

#include "log.h"

#define LINKED_APP_SCHEME_1_1 "urn:dvb:metadata:cs:LinkedApplicationCS:2019:1.1"

using namespace std;

namespace orb
{

AppManager::AppManager(ApplicationType apptype)
    : mAppType(apptype)
{
    // Set this AppManager instance as the callback for ApplicationManager
    ApplicationManager::instance().RegisterCallback(apptype, this);
}

string AppManager::executeRequest(string method, Json::Value token, Json::Value params, ApplicationType apptype)
{
    // TODO Set up proper responses
    string response = R"({"Response": "AppManager request [)" + method + R"(] not implemented"})";

    std::lock_guard<std::mutex> lock(mMutex);
    ApplicationManager::instance().SetCurrentInterface(mAppType);

    LOGI("Request with method [" + method + "] received");
    if (method == "createApplication")
    {
        LOGI("app type: ") << mAppType;
    }
    else if (method == "destroyApplication")
    {
        // no response
        LOGI("");
    }
    else if (method == "showApplication")
    {
        // no response
        LOGI("");
    }
    else if (method == "hideApplication")
    {
        // no response
        LOGI("");
    }
    else if (method == "searchOwner")
    {
        // no response
        LOGI("");
    }
    else if (method == "getRunningAppIds")
    {
        // TODO implement
        LOGI("");
    }
    else
    {
        LOGI("Unknown method: " << method);
    }

    return response;
}

void AppManager::processAitSection(int32_t aitPid, int32_t serviceId, const std::vector<uint8_t>& section) {
    std::lock_guard<std::mutex> lock(mMutex);
    ApplicationManager::instance().SetCurrentInterface(mAppType);
    ApplicationManager::instance().ProcessAitSection(aitPid, serviceId, section.data(), section.size());
}

void AppManager::processXmlAit(const std::vector<uint8_t>& xmlait) {
    std::string xmlString(xmlait.begin(), xmlait.end());
    std::lock_guard<std::mutex> lock(mMutex);
    ApplicationManager::instance().SetCurrentInterface(mAppType);
    ApplicationManager::instance().ProcessXmlAit(xmlString);
}

// ApplicationSessionCallback implementation
void AppManager::LoadApplication(const int appId, const char *entryUrl) {
    LOGI("appID: " << appId << ", url: " << entryUrl);
}

void AppManager::LoadApplication(const int appId, const char *entryUrl, int size, const std::vector<uint16_t> graphics) {
    LOGI("appID: " << appId << ", url: " << entryUrl);
}

void AppManager::ShowApplication(const int appId) {
    LOGI("appID: " << appId);
}

void AppManager::HideApplication(const int appId) {
    LOGI("appID: " << appId);
}

void AppManager::StopBroadcast() {
    LOGI("");
}

void AppManager::ResetBroadcastPresentation() {
    LOGI("");
}

void AppManager::DispatchApplicationLoadErrorEvent() {
    LOGI("");
}

void AppManager::DispatchTransitionedToBroadcastRelatedEvent(const int appId) {
    LOGI("appID: " << appId);
}

std::string AppManager::GetXmlAitContents(const std::string &url) {
    std::string result;
    return result;
}

int AppManager::GetParentalControlAge() {
    LOGI("");
    return 0;
}

std::string AppManager::GetParentalControlRegion() {
    std::string result;
    LOGI("");
    return result;
}

std::string AppManager::GetParentalControlRegion3() {
    std::string result;
    LOGI("");
    return result;
}

void AppManager::DispatchApplicationSchemeUpdatedEvent(const int appId, const std::string &scheme) {
    LOGI("appID: " << appId);
}

void AppManager::DispatchOperatorApplicationStateChange(const int appId, const std::string &oldState, const std::string &newState) {
    LOGI("appID: " << appId);
}

void AppManager::DispatchOperatorApplicationStateChangeCompleted(const int appId, const std::string &oldState, const std::string &newState) {
    LOGI("appID: " << appId);
}

void AppManager::DispatchOperatorApplicationContextChange(const int appId, const std::string &startupLocation, const std::string &launchLocation) {
    LOGI("appID: " << appId);
}

void AppManager::DispatchOpAppUpdate(const int appId, const std::string &updateEvent) {
    LOGI("appID: " << appId);
}

bool AppManager::isInstanceInCurrentService(const Utils::S_DVB_TRIPLET &triplet) {
    LOGI("");
    return false;
}

bool AppManager::IsRequestAllowed(string token)
{
    // TODO implement
    return true;
}

} // namespace orb
