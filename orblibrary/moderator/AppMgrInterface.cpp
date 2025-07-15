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
#include <json/json.h>

#include "AppMgrInterface.hpp"
#include "app_mgr/application_manager.h"

#include "log.h"

#define LINKED_APP_SCHEME_1_1 "urn:dvb:metadata:cs:LinkedApplicationCS:2019:1.1"
#define EMPTY_STRING ""

using namespace std;

namespace orb
{

AppMgrInterface::AppMgrInterface(IOrbBrowser* browser, ApplicationType apptype)
    : mOrbBrowser(browser)
    , mAppType(apptype)
{
    // Set this AppMgrInterface instance as the callback for ApplicationManager
    ApplicationManager::instance().RegisterCallback(apptype, this);
}

string AppMgrInterface::executeRequest(string method, Json::Value token, Json::Value params)
{
    // TODO Set up proper responses
    string response = R"({"Response": "AppMgrInterface request [)" + method + R"(] not implemented"})";

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

void AppMgrInterface::processAitSection(int32_t aitPid, int32_t serviceId, const std::vector<uint8_t>& section) {
    std::lock_guard<std::mutex> lock(mMutex);
    ApplicationManager::instance().SetCurrentInterface(mAppType);
    ApplicationManager::instance().ProcessAitSection(aitPid, serviceId, section.data(), section.size());
}

void AppMgrInterface::processXmlAit(const std::vector<uint8_t>& xmlait) {
    std::string xmlString(xmlait.begin(), xmlait.end());
    std::lock_guard<std::mutex> lock(mMutex);
    ApplicationManager::instance().SetCurrentInterface(mAppType);
    ApplicationManager::instance().ProcessXmlAit(xmlString);
}

// ApplicationSessionCallback implementation
void AppMgrInterface::LoadApplication(const int appId, const char *entryUrl) {
    LOGI("appID: " << appId << ", url: " << entryUrl);
    mOrbBrowser->loadApplication("AppId-todo", entryUrl);
}

void AppMgrInterface::LoadApplication(const int appId, const char *entryUrl, int size, const std::vector<uint16_t> graphics) {
    LOGI("appID: " << appId << ", url: " << entryUrl);
    // TODO: need a different API to add extra params
    mOrbBrowser->loadApplication("AppId-todo", entryUrl);
}

void AppMgrInterface::ShowApplication(const int appId) {
    LOGI("appID: " << appId);
    mOrbBrowser->showApplication();
}

void AppMgrInterface::HideApplication(const int appId) {
    LOGI("appID: " << appId);
    mOrbBrowser->hideApplication();
}

void AppMgrInterface::StopBroadcast() {
    LOGI(" TODO ");
}

void AppMgrInterface::ResetBroadcastPresentation() {
    LOGI(" TODO ");
}

void AppMgrInterface::DispatchApplicationLoadErrorEvent() {
    mOrbBrowser->dispatchEvent("ApplicationLoadError", "{}");
}

void AppMgrInterface::DispatchTransitionedToBroadcastRelatedEvent(const int appId) {
    LOGI("appID: " << appId);
}

std::string AppMgrInterface::GetXmlAitContents(const std::string &url) {
    std::string result;
    return result;
}

int AppMgrInterface::GetParentalControlAge() {
    LOGI("");
    return 0;
}

std::string AppMgrInterface::GetParentalControlRegion() {
    std::string result;
    LOGI("");
    return result;
}

std::string AppMgrInterface::GetParentalControlRegion3() {
    std::string result;
    LOGI("");
    return result;
}

void AppMgrInterface::DispatchApplicationSchemeUpdatedEvent(const int appId, const std::string &scheme) {
    LOGI("appID: " << appId << ", Scheme: " << scheme);
    Json::StreamWriterBuilder writerBuilder;
    Json::Value prop;
    writerBuilder["indentation"] = EMPTY_STRING; // optional?
    prop["scheme"] = scheme.c_str();
    std::string properties = Json::writeString(writerBuilder, prop);
    mOrbBrowser->dispatchEvent("ApplicationSchemeUpdated", properties);
}

void AppMgrInterface::DispatchOperatorApplicationStateChange(const int appId, const std::string &oldState, const std::string &newState) {
    LOGI("appID: " << appId);
}

void AppMgrInterface::DispatchOperatorApplicationStateChangeCompleted(const int appId, const std::string &oldState, const std::string &newState) {
    LOGI("appID: " << appId);
}

void AppMgrInterface::DispatchOperatorApplicationContextChange(const int appId, const std::string &startupLocation, const std::string &launchLocation) {
    LOGI("appID: " << appId);
}

void AppMgrInterface::DispatchOpAppUpdate(const int appId, const std::string &updateEvent) {
    LOGI("appID: " << appId);
}

bool AppMgrInterface::isInstanceInCurrentService(const Utils::S_DVB_TRIPLET &triplet) {
    LOGI("");
    return false;
}

bool AppMgrInterface::IsRequestAllowed(string token)
{
    // TODO implement
    return true;
}

} // namespace orb
